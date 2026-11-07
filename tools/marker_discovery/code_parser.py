"""C/C++ source code parser using tree-sitter for t2_event_* marker extraction."""

import logging
import os

import tree_sitter_c as tsc
from tree_sitter import Language, Parser

from . import MarkerRecord

logger = logging.getLogger(__name__)

C_LANGUAGE = Language(tsc.language())

T2_DIRECT_APIS = {"t2_event_s", "t2_event_d", "t2_event_f"}
C_EXTENSIONS = {".c", ".cpp", ".h"}

# Directory names to skip during scanning
SKIP_DIRS = {"test", "tests", ".git"}


def _create_parser():
    """Create a tree-sitter parser with C grammar."""
    parser = Parser(C_LANGUAGE)
    return parser


def _walk_tree(node):
    """Recursively yield all nodes in the AST."""
    yield node
    for child in node.children:
        yield from _walk_tree(child)


def _get_function_name(call_node):
    """Extract the function name from a call_expression node."""
    func = call_node.child_by_field_name("function")
    if func is None:
        return None
    if func.type == "identifier":
        return func.text.decode("utf-8")
    if func.type == "field_expression":
        field = func.child_by_field_name("field")
        if field:
            return field.text.decode("utf-8")
    return None


def _get_call_arguments(call_node):
    """Extract argument nodes from a call_expression."""
    args_node = call_node.child_by_field_name("arguments")
    if args_node is None:
        return []
    return [child for child in args_node.named_children]


def _extract_string_literal(node):
    """Extract the string content from a string_literal node, stripping quotes."""
    if node.type == "string_literal":
        text = node.text.decode("utf-8")
        # Strip surrounding quotes
        if text.startswith('"') and text.endswith('"'):
            return text[1:-1]
    return None


def _extract_string_from_arg(node):
    """Extract a string literal from an argument, handling casts like (char *) \"str\"."""
    # Direct string literal
    result = _extract_string_literal(node)
    if result is not None:
        return result

    # Cast expression: (char *) "str" — search descendants for a string_literal
    if node.type == "cast_expression":
        for child in _walk_tree(node):
            result = _extract_string_literal(child)
            if result is not None:
                return result

    return None


def _find_c_files(repo_path):
    """Recursively find all .c, .cpp, .h files in a directory."""
    c_files = []
    for root, dirs, files in os.walk(repo_path):
        # Skip test and .git directories
        dirs[:] = [d for d in dirs if d.lower() not in SKIP_DIRS]
        for fname in files:
            ext = os.path.splitext(fname)[1].lower()
            if ext in C_EXTENSIONS:
                c_files.append(os.path.join(root, fname))
    return c_files


def _parse_file(parser, file_path):
    """Parse a C/C++ file and return the tree, or None on error."""
    try:
        with open(file_path, "rb") as f:
            source = f.read()
        tree = parser.parse(source)
        return tree
    except Exception as e:
        logger.warning("Failed to parse %s: %s", file_path, e)
        return None


def scan_direct_calls(repo_path, repo_name):
    """Scan a repo for direct t2_event_s/d/f calls with string literal markers.

    Returns list of MarkerRecord.
    """
    parser = _create_parser()
    markers = []
    c_files = _find_c_files(repo_path)

    for file_path in c_files:
        tree = _parse_file(parser, file_path)
        if tree is None:
            continue

        rel_path = os.path.relpath(file_path, repo_path)

        for node in _walk_tree(tree.root_node):
            if node.type != "call_expression":
                continue

            func_name = _get_function_name(node)
            if func_name not in T2_DIRECT_APIS:
                continue

            args = _get_call_arguments(node)
            if not args:
                continue

            marker_name = _extract_string_from_arg(args[0])
            if marker_name is None:
                # Fall back to raw argument text so no occurrence is missed
                marker_name = args[0].text.decode("utf-8")

            line = node.start_point[0] + 1  # tree-sitter is 0-indexed
            markers.append(MarkerRecord(
                marker_name=marker_name,
                component=repo_name,
                file_path=rel_path,
                line=line,
                api=func_name,
                source_type="source",
            ))

    logger.info("Found %d direct markers in %s", len(markers), repo_name)
    return markers


def _get_function_params(func_def_node):
    """Extract parameter names from a function_definition node.

    Returns list of parameter name strings in order.
    """
    declarator = func_def_node.child_by_field_name("declarator")
    if declarator is None:
        return []

    # Navigate to the parameter list — could be nested in pointer_declarator etc.
    params_node = None
    for node in _walk_tree(declarator):
        if node.type == "parameter_list":
            params_node = node
            break

    if params_node is None:
        return []

    param_names = []
    for child in params_node.named_children:
        if child.type == "parameter_declaration":
            pdecl = child.child_by_field_name("declarator")
            if pdecl:
                # Could be a plain identifier or a pointer_declarator
                if pdecl.type == "identifier":
                    param_names.append(pdecl.text.decode("utf-8"))
                elif pdecl.type == "pointer_declarator":
                    for n in _walk_tree(pdecl):
                        if n.type == "identifier":
                            param_names.append(n.text.decode("utf-8"))
                            break
                else:
                    # Try to find any identifier descendant
                    for n in _walk_tree(pdecl):
                        if n.type == "identifier":
                            param_names.append(n.text.decode("utf-8"))
                            break
    return param_names


def detect_wrappers(repo_path):
    """Detect wrapper functions that call t2_event_* with a variable (not literal) first arg.

    Returns list of dicts:
        {'wrapper_name': str, 'marker_param_index': int, 'api': str, 'file': str, 'inner_line': int}
    """
    parser = _create_parser()
    wrappers = []
    c_files = _find_c_files(repo_path)

    for file_path in c_files:
        tree = _parse_file(parser, file_path)
        if tree is None:
            continue

        for node in _walk_tree(tree.root_node):
            if node.type != "function_definition":
                continue

            # Get function name
            declarator = node.child_by_field_name("declarator")
            if declarator is None:
                continue

            func_name = None
            for n in _walk_tree(declarator):
                if n.type == "identifier":
                    func_name = n.text.decode("utf-8")
                    break

            if func_name is None:
                continue

            # Skip direct API functions — they are not wrappers
            if func_name in T2_DIRECT_APIS:
                continue

            # Get parameter names
            param_names = _get_function_params(node)
            if not param_names:
                continue

            # Search body for t2_event_* calls with variable first arg
            body = node.child_by_field_name("body")
            if body is None:
                continue

            for body_node in _walk_tree(body):
                if body_node.type != "call_expression":
                    continue

                callee = _get_function_name(body_node)
                if callee not in T2_DIRECT_APIS:
                    continue

                args = _get_call_arguments(body_node)
                if not args:
                    continue

                first_arg = args[0]
                # If first arg is an identifier (variable), this is a wrapper
                if first_arg.type == "identifier":
                    var_name = first_arg.text.decode("utf-8")
                    # Find which parameter position this variable corresponds to
                    if var_name in param_names:
                        param_index = param_names.index(var_name)
                        inner_line = body_node.start_point[0] + 1
                        wrappers.append({
                            "wrapper_name": func_name,
                            "marker_param_index": param_index,
                            "api": callee,
                            "file": file_path,
                            "inner_line": inner_line,
                        })
                        logger.debug("Detected wrapper: %s (param %d) → %s in %s",
                                     func_name, param_index, callee, file_path)

    logger.info("Detected %d wrapper functions in repo", len(wrappers))
    return wrappers


def resolve_wrapper_calls(repo_path, repo_name, wrappers):
    """Find call sites of detected wrappers and extract marker names.

    Returns list of MarkerRecord.
    """
    if not wrappers:
        return []

    parser = _create_parser()
    markers = []
    c_files = _find_c_files(repo_path)

    # Build lookup: wrapper_name → (param_index, api)
    wrapper_lookup = {}
    for w in wrappers:
        wrapper_lookup[w["wrapper_name"]] = (w["marker_param_index"], w["api"])

    for file_path in c_files:
        tree = _parse_file(parser, file_path)
        if tree is None:
            continue

        rel_path = os.path.relpath(file_path, repo_path)

        for node in _walk_tree(tree.root_node):
            if node.type != "call_expression":
                continue

            func_name = _get_function_name(node)
            if func_name not in wrapper_lookup:
                continue

            param_index, underlying_api = wrapper_lookup[func_name]
            args = _get_call_arguments(node)
            if len(args) <= param_index:
                continue

            marker_name = _extract_string_from_arg(args[param_index])
            if marker_name is None:
                # Fall back to raw argument text so no occurrence is missed
                marker_name = args[param_index].text.decode("utf-8")

            line = node.start_point[0] + 1
            markers.append(MarkerRecord(
                marker_name=marker_name,
                component=repo_name,
                file_path=rel_path,
                line=line,
                api=f"{func_name}→{underlying_api}",
                source_type="source",
            ))

    logger.info("Resolved %d wrapper call markers in %s", len(markers), repo_name)
    return markers


def scan_t2_init(repo_path):
    """Scan a repo for t2_init() calls and extract the configured component name argument.

    Returns a list of strings (the first string-literal argument of each t2_init call).
    Returns an empty list if no t2_init calls are found.
    """
    parser = _create_parser()
    names = []
    c_files = _find_c_files(repo_path)

    for file_path in c_files:
        tree = _parse_file(parser, file_path)
        if tree is None:
            continue

        for node in _walk_tree(tree.root_node):
            if node.type != "call_expression":
                continue

            func_name = _get_function_name(node)
            if func_name != "t2_init":
                continue

            args = _get_call_arguments(node)
            if not args:
                continue

            name = _extract_string_from_arg(args[0])
            if name is None:
                # Fall back to raw argument text so no occurrence is missed
                name = args[0].text.decode("utf-8")
            names.append(name)

    logger.info("Found %d t2_init call(s) in %s", len(names), repo_path)
    return names


def scan_repo(repo_path, repo_name):
    """Full scan of a repo: direct calls + wrapper resolution.

    Returns list of MarkerRecord.
    """
    # Pass 1: Direct calls (includes raw-text fallback for non-literals)
    markers = scan_direct_calls(repo_path, repo_name)

    # Pass 2: Detect wrappers
    wrappers = detect_wrappers(repo_path)

    # Exclude wrapper-internal t2_event_* calls from Pass 1 results
    # (these have raw variable names like "marker" and are resolved properly in Pass 3)
    if wrappers:
        wrapper_internal = set()
        for w in wrappers:
            rel_path = os.path.relpath(w["file"], repo_path)
            wrapper_internal.add((rel_path, w["inner_line"]))
        markers = [m for m in markers if (m.file_path, m.line) not in wrapper_internal]

    # Pass 3: Resolve wrapper call sites
    wrapper_markers = resolve_wrapper_calls(repo_path, repo_name, wrappers)
    markers.extend(wrapper_markers)

    logger.info("Total markers in %s: %d (direct) + %d (wrapper) = %d",
                repo_name, len(markers) - len(wrapper_markers),
                len(wrapper_markers), len(markers))
    return markers
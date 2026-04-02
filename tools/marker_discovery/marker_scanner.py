"""Marker Discovery Tool — CLI entry point and orchestration."""

import argparse
import logging
import sys

from . import github_client, code_parser, script_parser, patch_parser, report_generator
from . import component_file_parser

logger = logging.getLogger(__name__)


def setup_logging(verbose=False):
    level = logging.DEBUG if verbose else logging.INFO
    logging.basicConfig(
        level=level,
        format="%(asctime)s [%(levelname)s] %(name)s: %(message)s",
        stream=sys.stderr,
    )


def parse_args(argv=None):
    parser = argparse.ArgumentParser(
        description="Scan GitHub organizations for T2 telemetry marker instrumentation.",
    )
    parser.add_argument(
        "--branch", default="main",
        help="Branch/tag to scan (default: main). Ignored when --input-file is used.",
    )
    parser.add_argument(
        "--org", action="append", dest="orgs", default=None,
        help="GitHub organization to scan (repeatable, default: all 4 RDK orgs)",
    )
    parser.add_argument(
        "--repo",
        help="Single repo name to scan (requires --org). Example: --org rdkcentral --repo telemetry",
    )
    parser.add_argument(
        "--input-file",
        help="Version manifest file (versions.txt format). Clones only listed GitHub repos at exact commits.",
    )
    parser.add_argument(
        "--output", default=None,
        help="Output file path for markdown report (default: stdout)",
    )
    parser.add_argument(
        "--verbose", "-v", action="store_true",
        help="Enable verbose/debug logging",
    )
    args = parser.parse_args(argv)
    if args.repo and not args.orgs:
        parser.error("--repo requires --org. Example: --org rdkcentral --repo telemetry")
    if args.orgs is None:
        args.orgs = list(github_client.DEFAULT_ORGS)
    return args


def run_fast_path(org, branch, temp_dir):
    """Fast path: use search API to find repos with markers, then clone only those."""
    logger.info("Fast path: searching for marker repos in %s via Search API...", org)

    matching_repos = github_client.search_all_markers_in_org(org)
    if not matching_repos:
        logger.warning("No repos with markers found in %s via search", org)
        return [], 0

    logger.info("Found %d repos with potential markers, cloning...", len(matching_repos))

    # Get clone URLs for matching repos
    all_repos = github_client.list_org_repos(org)
    repo_names_to_clone = {r for r in matching_repos}
    repos_to_clone = [r["name"] for r in all_repos if r["name"] in repo_names_to_clone]

    cloned = github_client.clone_matching_repos(org, repos_to_clone, branch, temp_dir)
    return cloned, len(all_repos)


def run_full_path(org, branch, temp_dir):
    """Full path: list all repos, clone all, scan locally."""
    logger.info("Full path: listing all repos in %s...", org)

    all_repos = github_client.list_org_repos(org)
    repo_names = [r["name"] for r in all_repos]

    logger.info("Cloning %d repos on branch %s...", len(repo_names), branch)
    cloned = github_client.clone_matching_repos(org, repo_names, branch, temp_dir)
    return cloned, len(all_repos)


def run_full_path_single(org, repo, branch, temp_dir):
    """Clone and scan a single repo."""
    logger.info("Single-repo mode: cloning %s/%s on branch %s...", org, repo, branch)
    clone_path, actual_branch = github_client.clone_repo(org, repo, branch, temp_dir)
    if clone_path:
        cloned = [{"name": repo, "path": clone_path, "branch": actual_branch}]
        return cloned, 1
    logger.warning("Failed to clone %s/%s", org, repo)
    return [], 0


def scan_cloned_repos(cloned_repos):
    """Run all scanners on cloned repos. Returns list of MarkerRecord."""
    all_markers = []

    for repo_info in cloned_repos:
        repo_path = repo_info["path"]
        repo_name = repo_info["name"]

        logger.info("Scanning %s...", repo_name)

        # C/C++ source scanning (tree-sitter)
        try:
            markers = code_parser.scan_repo(repo_path, repo_name)
            all_markers.extend(markers)
        except Exception as e:
            logger.warning("Code parser failed for %s: %s", repo_name, e)

        # Script scanning
        try:
            markers = script_parser.scan_repo_scripts(repo_path, repo_name)
            all_markers.extend(markers)
        except Exception as e:
            logger.warning("Script parser failed for %s: %s", repo_name, e)

        # Patch scanning
        try:
            markers = patch_parser.scan_repo_patches(repo_path, repo_name)
            all_markers.extend(markers)
        except Exception as e:
            logger.warning("Patch parser failed for %s: %s", repo_name, e)

    return all_markers


def run_input_file_path(input_file, temp_dir):
    """Input-file path: parse version manifest, clone at exact commits."""
    logger.info("Input-file path: reading components from %s", input_file)

    components = component_file_parser.parse_component_file(input_file)
    if not components:
        logger.warning("No components found in %s", input_file)
        return [], 0, []

    cloned, unresolved = github_client.clone_components_from_file(components, temp_dir)
    return cloned, len(components), unresolved


def main(argv=None):
    args = parse_args(argv)
    setup_logging(args.verbose)

    temp_dir = github_client.create_temp_dir()
    logger.info("Using temp directory: %s", temp_dir)

    try:
        all_markers = []
        total_repos_scanned = 0
        unresolved_components = []

        if args.input_file:
            # Input-file mode: clone from version manifest
            cloned, total_components, unresolved = run_input_file_path(
                args.input_file, temp_dir
            )
            total_repos_scanned = total_components
            unresolved_components = unresolved

            markers = scan_cloned_repos(cloned)
            all_markers.extend(markers)

            branch_display = f"per-component (from {args.input_file})"
        elif args.repo:
            # Single-repo mode: clone one repo from the specified org
            org = args.orgs[0]
            cloned, _ = run_full_path_single(org, args.repo, args.branch, temp_dir)
            total_repos_scanned = 1

            if cloned:
                markers = scan_cloned_repos(cloned)
                all_markers.extend(markers)

            branch_display = args.branch
        else:
            # Default mode: scan all repos in all orgs on main branch
            for org in args.orgs:
                cloned, total_in_org = run_full_path(org, args.branch, temp_dir)
                total_repos_scanned += total_in_org

                markers = scan_cloned_repos(cloned)
                all_markers.extend(markers)

            branch_display = args.branch

        # Generate report
        report = report_generator.generate_report(
            markers=all_markers,
            branch=branch_display,
            orgs=args.orgs,
            components_scanned=total_repos_scanned,
            unresolved_components=unresolved_components,
        )

        # Output
        if args.output:
            with open(args.output, "w", encoding="utf-8") as f:
                f.write(report)
            logger.info("Report written to %s", args.output)
        else:
            print(report)

        logger.info("Done. Found %d markers total.", len(all_markers))
        return 0

    except Exception as e:
        logger.error("Fatal error: %s", e, exc_info=True)
        return 1

    finally:
        github_client.cleanup_temp_dir(temp_dir)


if __name__ == "__main__":
    sys.exit(main())
"""GitHub API client for repo enumeration, branch checking, and cloning."""

import logging
import os
import shutil
import subprocess
import tempfile
import time

import requests
from requests.auth import AuthBase

logger = logging.getLogger(__name__)

GITHUB_API = "https://api.github.com"


class _NetrcAuth(AuthBase):
    """requests-compatible auth that reads ~/.netrc on each request.

    Works around Python's netrc module rejecting files with open permissions.
    Credentials are never stored — they are read from disk per-request and
    injected directly into the Authorization header.
    """

    @staticmethod
    def _match_host(netrc_machine, request_host):
        """Check if a netrc machine entry matches the request host.

        Handles cases like netrc having 'github.com' matching 'api.github.com'.
        """
        return request_host == netrc_machine or request_host.endswith("." + netrc_machine)

    def __call__(self, r):
        host = requests.utils.urlparse(r.url).hostname
        netrc_path = os.path.expanduser("~/.netrc")
        login, password = None, None
        try:
            with open(netrc_path) as f:
                machine = None
                for line in f:
                    tokens = line.split()
                    i = 0
                    while i < len(tokens):
                        if tokens[i] == "machine":
                            i += 1
                            machine = tokens[i] if i < len(tokens) else None
                        elif tokens[i] == "login" and machine and self._match_host(machine, host):
                            i += 1
                            login = tokens[i] if i < len(tokens) else None
                        elif tokens[i] == "password" and machine and self._match_host(machine, host):
                            i += 1
                            password = tokens[i] if i < len(tokens) else None
                        i += 1
        except (FileNotFoundError, OSError):
            pass

        if login and password:
            r.headers["Authorization"] = requests.auth._basic_auth_str(login, password)
        return r


def _github_session():
    """Create a requests session that authenticates via ~/.netrc.

    Uses _NetrcAuth so credentials are read from disk on each request
    without being extracted or stored by our code.
    """
    session = requests.Session()
    session.auth = _NetrcAuth()
    session.headers.update({
        "Accept": "application/vnd.github.v3+json",
        "User-Agent": "marker-discovery-tool",
    })
    return session


def list_org_repos(org):
    """List all repositories in a GitHub organization.

    Returns list of dicts with 'name' and 'clone_url' keys.
    """
    session = _github_session()
    repos = []
    page = 1

    while True:
        url = f"{GITHUB_API}/orgs/{org}/repos"
        resp = session.get(url, params={"per_page": 100, "page": page})
        resp.raise_for_status()
        data = resp.json()
        if not data:
            break
        for repo in data:
            repos.append({
                "name": repo["name"],
                "clone_url": repo["clone_url"],
            })
        page += 1

    logger.info("Found %d repos in org %s", len(repos), org)
    return repos


def search_code_in_org(org, query):
    """Search for code in an organization using GitHub Search API.

    Returns a set of repository names that contain matches.
    Rate-limited to 10 req/min for code search.
    """
    session = _github_session()
    repo_names = set()
    page = 1
    search_query = f"{query} org:{org}"

    while True:
        url = f"{GITHUB_API}/search/code"
        resp = session.get(url, params={"q": search_query, "per_page": 100, "page": page})

        if resp.status_code == 403:
            # Rate limited — wait and retry
            retry_after = int(resp.headers.get("Retry-After", 60))
            logger.warning("Search API rate limited, waiting %ds...", retry_after)
            time.sleep(retry_after)
            continue

        resp.raise_for_status()
        data = resp.json()
        items = data.get("items", [])
        if not items:
            break

        for item in items:
            repo_names.add(item["repository"]["name"])

        # Check if there are more pages
        if len(items) < 100:
            break
        page += 1

        # Respect rate limit: 10 req/min for code search
        time.sleep(6)

    logger.info("Search '%s' in %s found matches in %d repos", query, org, len(repo_names))
    return repo_names


def search_all_markers_in_org(org):
    """Search for all marker API patterns in an organization.

    Runs three searches (t2_event, t2ValNotify, t2CountNotify) and merges results.
    Returns a set of repository names.
    """
    all_repos = set()
    for query in ["t2_event", "t2ValNotify", "t2CountNotify"]:
        matches = search_code_in_org(org, query)
        all_repos.update(matches)
    logger.info("Total repos with marker matches in %s: %d", org, len(all_repos))
    return all_repos


def check_branch_exists(org, repo, branch):
    """Check if a branch exists in a repository. Returns True/False."""
    session = _github_session()
    url = f"{GITHUB_API}/repos/{org}/{repo}/branches/{branch}"
    resp = session.get(url)
    return resp.status_code == 200


def clone_repo(org, repo, branch, target_dir):
    """Shallow clone a repo with branch fallback chain: specified → develop → main → skip.

    Returns (clone_path, actual_branch) on success, or (None, None) if all branches fail.
    """
    clone_url = f"https://github.com/{org}/{repo}.git"
    clone_path = os.path.join(target_dir, repo)

    fallback_branches = [branch]
    if branch != "develop":
        fallback_branches.append("develop")
    if branch != "main":
        fallback_branches.append("main")

    for b in fallback_branches:
        try:
            result = subprocess.run(
                ["git", "clone", "--depth", "1", "--branch", b, clone_url, clone_path],
                capture_output=True,
                text=True,
                timeout=120,
            )
            if result.returncode == 0:
                logger.info("Cloned %s/%s on branch %s", org, repo, b)
                return clone_path, b
            else:
                logger.debug("Branch %s not found for %s/%s", b, org, repo)
                # Clean up failed clone attempt
                if os.path.exists(clone_path):
                    shutil.rmtree(clone_path)
        except subprocess.TimeoutExpired:
            logger.warning("Clone timeout for %s/%s on branch %s", org, repo, b)
            if os.path.exists(clone_path):
                shutil.rmtree(clone_path)

    logger.warning("Skipping %s/%s — no valid branch found", org, repo)
    return None, None


def clone_matching_repos(org, repo_names, branch, temp_dir):
    """Clone a list of repos with branch fallback.

    Args:
        org: GitHub organization name
        repo_names: iterable of repo names to clone
        branch: target branch name
        temp_dir: directory to clone into

    Returns list of dicts: {'name': str, 'path': str, 'branch': str}
    """
    cloned = []
    for repo_name in sorted(repo_names):
        clone_path, actual_branch = clone_repo(org, repo_name, branch, temp_dir)
        if clone_path:
            cloned.append({
                "name": repo_name,
                "path": clone_path,
                "branch": actual_branch,
            })
    logger.info("Cloned %d/%d repos for org %s", len(cloned), len(list(repo_names)), org)
    return cloned


def create_temp_dir():
    """Create a temporary directory for cloning repos."""
    return tempfile.mkdtemp(prefix="marker_discovery_")


def cleanup_temp_dir(temp_dir):
    """Remove temporary clone directory."""
    if temp_dir and os.path.exists(temp_dir):
        shutil.rmtree(temp_dir)
        logger.info("Cleaned up temp directory: %s", temp_dir)

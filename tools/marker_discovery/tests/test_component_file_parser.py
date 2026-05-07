"""Tests for component_file_parser module — versions.txt parsing."""

import pytest

from tools.marker_discovery.component_file_parser import parse_component_file


@pytest.fixture
def versions_file(tmp_path):
    """Create a sample versions.txt file."""
    content = """\
b'https://github.com/rdkcentral/meta-rdk-iot'@a798f942e586cb29c9eb4687668ae3cbfe50edaa : a798f942e586cb29c9eb4687668ae3cbfe50edaa
b'ssh://gerrit.teamccp.com:29418/rdk/yocto_oe/layers/meta-rdk'@b1e496d0dfb0dddd118e90bb33588950bd7a4306 : b1e496d0dfb0dddd118e90bb33588950bd7a4306
https://github.com/rdkcentral/rbus.git@develop : 4a25e92112e827f7007de52488666c81c4564b5a
https://github.com/rdk-e/some-repo@main : deadbeef1234567890abcdef1234567890abcdef
https://github.com/Comcast/libparodus.git@master : 8263bb06c8c16dc114c800a3d29d0c8252f15619
https://github.com/rdkcentral/breakpad_wrapper.git@ : be8cd679e095cd300f77913863724fa5e39a6182
git://git.kernel.org/pub/scm/utils/dtc/dtc.git@master : 302fca9f4c283e1994cf0a5a9ce1cf43ca15e6d2
https://github.com/arsv/perl-cross/releases/download/1.3.7/perl-cross-1.3.7.tar.gz : md5sum None
http://github.com/ayourtch/nat46.git@master : 80dda1d08efe361b4f236eeae56015065cba1b1d
ssh://github.com/rdk-e/rdkservices-cpc@ : 1dff01bd7714ce9076b718f7138ca490a60b26f0
ssh://github.com/rdk-e/airplay-application-cpc@ : 43c9d71147fa8785f37eadab3143179fd86b01a1
b'ssh://git@github.com/rdk-e/meta-rdk-tools'@690b41b5ad8bc7a634ec9c50ee7019335e2e404f : 690b41b5ad8bc7a634ec9c50ee7019335e2e404f
b'ssh://git@github.com/rdk-common/meta-cspc-security-release'@e8050b5c28a796ecc67e9029094294f41d799d03 : e8050b5c28a796ecc67e9029094294f41d799d03
ssh://github.com/entos-xe/asanalytics.git@ : c32b5d703f1e6222bc3a59b653b456682704ae85
ssh://gerrit.teamccp.com:29418/rdk/components/generic/fonts/generic@stable2 : d9de2ee1005626201a9ec60c9f685a2450f8cc73
"""
    f = tmp_path / "versions.txt"
    f.write_text(content)
    return f


class TestParseComponentFile:
    def test_parses_all_github_repos(self, versions_file):
        components = parse_component_file(str(versions_file))
        names = {c['name'] for c in components}
        # Should include ALL GitHub repos regardless of org
        assert "meta-rdk-iot" in names
        assert "rbus" in names
        assert "some-repo" in names
        assert "rdkservices-cpc" in names
        assert "airplay-application-cpc" in names
        assert "meta-rdk-tools" in names
        assert "meta-cspc-security-release" in names
        # Comcast, ayourtch, entos-xe repos also included
        assert "libparodus" in names
        assert "nat46" in names
        assert "asanalytics" in names

    def test_skips_non_github_urls(self, versions_file):
        components = parse_component_file(str(versions_file))
        names = {c['name'] for c in components}
        # gerrit entries should be skipped (both b'ssh://gerrit and ssh://gerrit)
        assert "meta-rdk" not in names
        assert "generic" not in names
        # kernel.org should be skipped
        assert "dtc" not in names

    def test_skips_tarball_downloads(self, versions_file):
        components = parse_component_file(str(versions_file))
        names = {c['name'] for c in components}
        assert "perl-cross" not in names

    def test_extracts_org(self, versions_file):
        components = parse_component_file(str(versions_file))
        by_name = {c['name']: c for c in components}
        assert by_name["meta-rdk-iot"]["org"] == "rdkcentral"
        assert by_name["rbus"]["org"] == "rdkcentral"
        assert by_name["some-repo"]["org"] == "rdk-e"

    def test_extracts_commit_sha(self, versions_file):
        components = parse_component_file(str(versions_file))
        by_name = {c['name']: c for c in components}
        assert by_name["rbus"]["commit"] == "4a25e92112e827f7007de52488666c81c4564b5a"

    def test_extracts_branch(self, versions_file):
        components = parse_component_file(str(versions_file))
        by_name = {c['name']: c for c in components}
        assert by_name["rbus"]["branch"] == "develop"
        assert by_name["some-repo"]["branch"] == "main"

    def test_empty_branch(self, versions_file):
        components = parse_component_file(str(versions_file))
        by_name = {c['name']: c for c in components}
        # breakpad_wrapper has empty ref after @
        assert by_name["breakpad_wrapper"]["branch"] == ""

    def test_builds_clone_url(self, versions_file):
        components = parse_component_file(str(versions_file))
        by_name = {c['name']: c for c in components}
        # HTTPS source keeps HTTPS clone URL
        assert by_name["rbus"]["url"] == "https://github.com/rdkcentral/rbus.git"
        # SSH sources are converted to HTTPS clone URLs
        assert by_name["rdkservices-cpc"]["url"] == "https://github.com/rdk-e/rdkservices-cpc.git"
        assert by_name["meta-rdk-tools"]["url"] == "https://github.com/rdk-e/meta-rdk-tools.git"

    def test_handles_byte_string_prefix(self, versions_file):
        components = parse_component_file(str(versions_file))
        by_name = {c['name']: c for c in components}
        # b'...' format should still be parsed
        assert "meta-rdk-iot" in by_name

    def test_empty_file(self, tmp_path):
        f = tmp_path / "empty.txt"
        f.write_text("")
        components = parse_component_file(str(f))
        assert components == []

    def test_count_matches_real_file(self, tmp_path):
        """Test with a small realistic snippet."""
        content = """\
https://github.com/rdkcentral/advanced-security@develop : 81176604c5e971ca77a1ea70d57a9e3dda0a7a1e
https://github.com/rdkcentral/rbus.git@develop : 4a25e92112e827f7007de52488666c81c4564b5a
https://github.com/rdkcentral/OneWifi.git@develop : 4755197732ded811a114136441dfa34a5faf12cd
https://github.com/Comcast/libparodus.git@master : 8263bb06c8c16dc114c800a3d29d0c8252f15619
git://git.kernel.org/pub/scm/utils/dtc/dtc.git@master : 302fca9f4c283e1994cf0a5a9ce1cf43ca15e6d2
ssh://github.com/rdk-e/ipcontrol@ : 59e631bf8620fe1e2196c78240dbb8d8f424c70e
b'ssh://git@github.com/rdk-e/meta-rdk-tools'@690b41b5 : 690b41b5ad8bc7a634ec9c50ee7019335e2e404f
"""
        f = tmp_path / "versions.txt"
        f.write_text(content)
        components = parse_component_file(str(f))
        assert len(components) == 6  # 3 rdkcentral + 1 Comcast + 2 rdk-e

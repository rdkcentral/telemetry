#!/usr/bin/env python3
"""
L2 Test Gap Analyzer - Implementation Template

This script demonstrates the core logic for analyzing L2 test coverage gaps.
Adapt and extend based on specific project needs.

Modes:
  1. Analysis mode (default): Generate L2_TEST_GAP.md report
  2. Sync mode (--sync-features): Update .feature files from test implementations
"""

import os
import re
import json
import argparse
from pathlib import Path
from typing import Dict, List, Set, Tuple
from collections import defaultdict


class TestGapAnalyzer:
    """Analyzes test coverage gaps in L2 functional tests"""
    
    def __init__(self, workspace_root: str):
        self.workspace_root = Path(workspace_root)
        self.source_dir = self.workspace_root / "source"
        self.test_dir = self.workspace_root / "test" / "functional-tests"
        self.features_dir = self.test_dir / "features"
        self.tests_dir = self.test_dir / "tests"
        
        # Exclude non-production modules from coverage analysis
        # These are test/simulation utilities, not production code deployed to CPE
        self.excluded_modules = {
            'nativeProtocolSimulator',  # Test simulation code
            'privacycontrol',           # Boilerplate for proprietary extensions
            'testApp',                  # Test utility
            'test',                     # Test utility
            'docs'                      # Documentation
        }
        
        # Results storage
        self.coverage_data = {}
        self.gaps = []
        self.orphaned_tests = []
        self.missing_implementations = []
    
    
    # =========================================================================
    # Step 1: Feature File Analysis
    # =========================================================================
    
    def parse_feature_files(self) -> Dict[str, List[str]]:
        """Parse .feature files to extract scenarios"""
        features = {}
        
        if not self.features_dir.exists():
            print(f"Warning: Features directory not found: {self.features_dir}")
            return features
        
        for feature_file in self.features_dir.glob("*.feature"):
            scenarios = self._extract_scenarios(feature_file)
            features[feature_file.name] = scenarios
        
        return features
    
    
    def _extract_scenarios(self, feature_file: Path) -> List[str]:
        """Extract scenario names from Gherkin feature file"""
        scenarios = []
        
        with open(feature_file, 'r') as f:
            content = f.read()
            
            # Match: Scenario: <name>
            scenario_pattern = r'^\s*Scenario:\s*(.+?)$'
            for match in re.finditer(scenario_pattern, content, re.MULTILINE):
                scenario_name = match.group(1).strip()
                scenarios.append(scenario_name)
        
        return scenarios
    
    
    # =========================================================================
    # Step 2: Test Implementation Discovery
    # =========================================================================
    
    def discover_test_files(self) -> Dict[str, List[str]]:
        """Discover test files and their test functions"""
        tests = {}
        
        if not self.tests_dir.exists():
            print(f"Warning: Tests directory not found: {self.tests_dir}")
            return tests
        
        for test_file in self.tests_dir.glob("test_*.py"):
            test_functions = self._extract_test_functions(test_file)
            tests[test_file.name] = test_functions
        
        return tests
    
    
    def _extract_test_functions(self, test_file: Path) -> List[Dict[str, str]]:
        """Extract test function names and docstrings from Python file"""
        test_funcs = []
        
        with open(test_file, 'r') as f:
            content = f.read()
            
            # Match: def test_<name>( or async def test_<name>(
            # Capture optional docstring
            test_pattern = r'^\s*(?:async\s+)?def\s+(test_\w+)\s*\([^)]*\)\s*:(?:\s*"""([^"]*)"""|\s*\'\'\'([^\']*)\'\'\')?'
            for match in re.finditer(test_pattern, content, re.MULTILINE | re.DOTALL):
                test_name = match.group(1)
                docstring = match.group(2) or match.group(3) or ''
                docstring = docstring.strip().split('\n')[0] if docstring else ''
                test_funcs.append({
                    'name': test_name,
                    'description': docstring
                })
        
        return test_funcs
    
    
    # =========================================================================
    # Step 3: Source Code Analysis
    # =========================================================================
    
    def analyze_source_modules(self) -> Dict[str, Dict]:
        """Analyze source code modules and extract public APIs"""
        modules = {}
        
        for module_dir in self.source_dir.iterdir():
            if module_dir.is_dir() and not module_dir.name.startswith('.'):
                # Skip excluded modules (test utilities, non-production code)
                if module_dir.name in self.excluded_modules:
                    continue
                    
                module_data = self._analyze_module(module_dir)
                modules[module_dir.name] = module_data
        
        return modules
    
    
    def _analyze_module(self, module_dir: Path) -> Dict:
        """Analyze single module directory"""
        data = {
            'c_files': [],
            'h_files': [],
            'public_apis': [],
            'functions': []
        }
        
        # Find C source files
        for c_file in module_dir.glob("*.c"):
            data['c_files'].append(c_file.name)
            functions = self._extract_functions_from_c(c_file)
            data['functions'].extend(functions)
        
        # Find header files
        for h_file in module_dir.glob("*.h"):
            data['h_files'].append(h_file.name)
            apis = self._extract_public_apis_from_header(h_file)
            data['public_apis'].extend(apis)
        
        return data
    
    
    def _extract_functions_from_c(self, c_file: Path) -> List[str]:
        """Extract function names from C source file"""
        functions = []
        
        with open(c_file, 'r', errors='ignore') as f:
            content = f.read()
            
            # Simple pattern: returnType functionName(
            # This is a heuristic - may need refinement
            func_pattern = r'^[a-zA-Z_][\w\s\*]+\s+([a-zA-Z_]\w+)\s*\('
            for match in re.finditer(func_pattern, content, re.MULTILINE):
                func_name = match.group(1)
                # Filter out common keywords
                if func_name not in ['if', 'while', 'for', 'switch', 'return']:
                    functions.append(func_name)
        
        return functions
    
    
    def _extract_public_apis_from_header(self, h_file: Path) -> List[str]:
        """Extract public API declarations from header file"""
        apis = []
        
        with open(h_file, 'r', errors='ignore') as f:
            content = f.read()
            
            # Look for function declarations (not static)
            # Pattern: returnType functionName(...);
            api_pattern = r'^(?!static)\s*[a-zA-Z_][\w\s\*]+\s+([a-zA-Z_]\w+)\s*\([^)]*\)\s*;'
            for match in re.finditer(api_pattern, content, re.MULTILINE):
                api_name = match.group(1)
                apis.append(api_name)
        
        return apis
    
    
    # =========================================================================
    # Step 4: Coverage Mapping
    # =========================================================================
    
    def map_source_to_tests(self, modules: Dict, tests: Dict) -> Dict:
        """Map source code modules to test coverage"""
        coverage = {}
        
        for module_name, module_data in modules.items():
            public_apis = set(module_data['public_apis'])
            
            # Find tests that reference this module
            tested_apis = self._find_tested_apis(module_name, public_apis, tests)
            
            coverage_pct = 0
            if public_apis:
                coverage_pct = len(tested_apis) / len(public_apis) * 100
            
            coverage[module_name] = {
                'total_apis': len(public_apis),
                'tested_apis': len(tested_apis),
                'coverage': coverage_pct,
                'untested': list(public_apis - tested_apis),
                'tested': list(tested_apis)
            }
        
        return coverage
    
    
    def _find_tested_apis(self, module_name: str, apis: Set[str], tests: Dict) -> Set[str]:
        """Find which APIs from a module are referenced in tests"""
        tested = set()
        
        for test_file, test_funcs in tests.items():
            test_path = self.tests_dir / test_file
            
            with open(test_path, 'r', errors='ignore') as f:
                content = f.read()
                
                # Check if any API name appears in test file
                for api in apis:
                    if re.search(rf'\b{api}\b', content):
                        tested.add(api)
        
        return tested
    
    
    # =========================================================================
    # Step 5: Gap Identification
    # =========================================================================
    
    def identify_gaps(self, features: Dict, tests: Dict, coverage: Dict):
        """Identify testing gaps"""
        
        # Gap 1: Features without test implementation
        self._find_missing_test_implementations(features, tests)
        
        # Gap 2: Tests without feature documentation
        self._find_orphaned_tests(features, tests)
        
        # Gap 3: Untested source code
        self._find_untested_code(coverage)
    
    
    def _fuzzy_match_names(self, name1: str, name2: str) -> bool:
        """Fuzzy match two names (feature/test files)"""
        # Normalize both names
        norm1 = name1.lower().replace('_', '')
        norm2 = name2.lower().replace('_', '')
        
        # Direct substring match (either direction)
        if norm1 in norm2 or norm2 in norm1:
            return True
        
        # Check if they end with same pattern
        if norm1.endswith(norm2) or norm2.endswith(norm1):
            return True
        
        # Handle plural/singular (remove trailing 's')
        norm1_singular = norm1.rstrip('s')
        norm2_singular = norm2.rstrip('s')
        
        if norm1_singular in norm2 or norm2_singular in norm1:
            return True
        
        if norm1 in norm2_singular or norm2 in norm1_singular:
            return True
        
        # Similarity scoring for close matches
        if len(norm1) > 5 and len(norm2) > 5:
            # Common prefix length
            common_prefix = 0
            for i in range(min(len(norm1), len(norm2))):
                if norm1[i] == norm2[i]:
                    common_prefix += 1
                else:
                    break
            
            longer = max(len(norm1), len(norm2))
            similarity = common_prefix / longer
            
            if similarity > 0.7:  # 70% similar
                return True
        
        return False
    
    
    def _find_missing_test_implementations(self, features: Dict, tests: Dict):
        """Find feature scenarios without corresponding tests"""
        for feature_file, scenarios in features.items():
            # Use fuzzy matching to find corresponding test file
            base_name = feature_file.replace('.feature', '')
            
            matching_test = None
            for test_file in tests.keys():
                test_base = test_file.replace('test_', '').replace('.py', '')
                
                if self._fuzzy_match_names(base_name, test_base):
                    matching_test = test_file
                    break
            
            if not matching_test:
                self.missing_implementations.append({
                    'feature': feature_file,
                    'scenarios': scenarios,
                    'reason': f"No test file found"
                })
            else:
                # Check if number of tests matches scenarios
                test_count = len(tests[matching_test])
                scenario_count = len(scenarios)
                
                if test_count < scenario_count:
                    self.missing_implementations.append({
                        'feature': feature_file,
                        'scenarios': scenarios,
                        'implemented': test_count,
                        'total': scenario_count,
                        'reason': f"Partial implementation ({test_count}/{scenario_count})"
                    })
    
    
    def _find_orphaned_tests(self, features: Dict, tests: Dict):
        """Find tests without corresponding feature files"""
        
        for test_file, test_funcs in tests.items():
            # Extract base name
            base_name = test_file.replace('test_', '').replace('.py', '')
            
            # Use fuzzy matching to find corresponding feature file
            matching_feature = None
            for feature_file in features.keys():
                feature_base = feature_file.replace('.feature', '')
                
                if self._fuzzy_match_names(base_name, feature_base):
                    matching_feature = feature_file
                    break
            
            if not matching_feature:
                self.orphaned_tests.append({
                    'test_file': test_file,
                    'tests': test_funcs,
                    'count': len(test_funcs)
                })
    
    
    def _find_untested_code(self, coverage: Dict):
        """Identify untested source code and prioritize gaps"""
        for module_name, cov_data in coverage.items():
            if cov_data['coverage'] < 75:  # Threshold for "good" coverage
                priority = self._calculate_priority(module_name, cov_data)
                
                self.gaps.append({
                    'module': module_name,
                    'coverage': cov_data['coverage'],
                    'untested_apis': cov_data['untested'],
                    'priority': priority
                })
        
        # Sort gaps by priority
        self.gaps.sort(key=lambda x: x['priority'], reverse=True)
    
    
    def _calculate_priority(self, module_name: str, cov_data: Dict) -> str:
        """Calculate priority based on module criticality and coverage"""
        
        # Critical modules
        critical_modules = ['bulkdata', 'scheduler', 'ccspinterface']
        
        coverage_pct = cov_data['coverage']
        
        if module_name in critical_modules and coverage_pct < 40:
            return 'CRITICAL'
        elif module_name in critical_modules and coverage_pct < 75:
            return 'HIGH'
        elif coverage_pct < 40:
            return 'HIGH'
        elif coverage_pct < 60:
            return 'MEDIUM'
        else:
            return 'LOW'
    
    
    # =========================================================================
    # Step 6: Mindmap Generation
    # =========================================================================
    
    def _calculate_functional_module_coverage(self) -> Dict:
        """Calculate functional coverage by mapping features to source modules"""
        
        # Map feature areas to modules
        module_mapping = {
            'bootup': ['bulkdata', 'scheduler', 'ccspinterface'],
            'multiprofile': ['bulkdata', 'reportgen', 't2parser'],
            'singleprofile': ['bulkdata', 'reportgen'],
            'temp_profile': ['bulkdata', 'scheduler'],
            'xconf': ['xconf-client', 'protocol'],
            'race_conditions': ['scheduler', 'bulkdata'],
        }
        
        module_tests = {}
        
        # Count which modules are tested based on feature files
        for impl in self.missing_implementations:
            feature_name = impl['feature'].replace('.feature', '')
            implemented = impl.get('implemented', 0)
            total = impl.get('total', len(impl.get('scenarios', [])))
            
            # Map to modules
            for feature_key, modules in module_mapping.items():
                if feature_key in feature_name:
                    coverage_pct = (implemented / total * 100) if total > 0 else 0
                    
                    for module in modules:
                        if module not in module_tests:
                            module_tests[module] = {'scenarios': 0, 'tested': 0}
                        module_tests[module]['scenarios'] += total
                        module_tests[module]['tested'] += implemented
        
        # Calculate coverage per module
        result = {}
        for module, data in module_tests.items():
            if data['scenarios'] > 0:
                coverage = (data['tested'] / data['scenarios']) * 100
            else:
                coverage = 0
            result[module] = {'coverage': coverage, 'scenarios': data['scenarios'], 'tested': data['tested']}
        
        # Add untested modules
        all_modules = ['bulkdata', 'ccspinterface', 'commonlib', 'dcautil', 'protocol', 
                      'reportgen', 'scheduler', 't2dm', 't2parser', 't2ssp', 'utils', 'xconf-client']
        
        for module in all_modules:
            if module not in result:
                result[module] = {'coverage': 0, 'scenarios': 0, 'tested': 0}
        
        return result
    
    def generate_coverage_mindmap(self, coverage: Dict) -> str:
        """Generate Mermaid mindmap showing coverage"""
        
        lines = ["```mermaid", "mindmap", "  root((Telemetry 2.0))"]
        
        for module_name, cov_data in sorted(coverage.items()):
            coverage_pct = cov_data['coverage']
            
            # Determine status icon
            if coverage_pct >= 75:
                status = "✅"
            elif coverage_pct >= 40:
                status = "⚠️"
            else:
                status = "❌"
            
            lines.append(f"    {module_name} {status} {coverage_pct:.0f}%")
        
        lines.append("```")
        
        return '\n'.join(lines)
    
    
    # =========================================================================
    # Step 7: Report Generation
    # =========================================================================
    
    def generate_report(self) -> str:
        """Generate concise one-page L2_TEST_GAP.md report"""
        
        report_sections = []
        
        # Header (includes coverage summary)
        report_sections.append(self._generate_header())
        
        # Visual Coverage Map
        report_sections.append(self._generate_visual_section())
        
        # Feature-Test Sync (if any issues)
        sync_section = self._generate_feature_sync_section()
        if sync_section:
            report_sections.append(sync_section)
        
        # Top 5 Gaps
        report_sections.append(self._generate_gap_analysis_section())
        
        # Quick Actions
        report_sections.append(self._generate_recommendations())
        
        return '\n\n'.join(report_sections)
    
    
    def _generate_header(self) -> str:
        """Generate report header with functional test statistics"""
        total_gaps = len(self.gaps)
        critical_gaps = sum(1 for g in self.gaps if g['priority'] == 'CRITICAL')
        high_gaps = sum(1 for g in self.gaps if g['priority'] == 'HIGH')
        
        # Calculate functional test coverage (scenario-based, not API-based)
        total_scenarios = 0
        implemented_scenarios = 0
        
        for item in self.missing_implementations:
            total_scenarios += item.get('total', len(item.get('scenarios', [])))
            implemented_scenarios += item.get('implemented', 0)
        
        # Count tests from test files
        test_count = 0
        for test_file in (self.tests_dir).glob("test_*.py"):
            with open(test_file, 'r') as f:
                content = f.read()
                test_count += len(re.findall(r'^\s*(?:async\s+)?def\s+test_', content, re.MULTILINE))
        
        # Calculate functional coverage
        if total_scenarios > 0:
            functional_coverage = (implemented_scenarios / total_scenarios) * 100
        else:
            functional_coverage = 0
        
        return f"""# L2 Test Coverage Gap Analysis

**Generated**: {self._get_timestamp()} | **Test Functions**: {test_count} | **Scenario Coverage**: {functional_coverage:.0f}% | **Gaps**: {critical_gaps} critical, {high_gaps} high

## Test Execution Summary

- **L2 Functional Tests**: {test_count} test functions across 6 test files
- **Test Type**: Integration/functional tests (black-box system testing)
- **Test Approach**: End-to-end testing via RBUS, HTTP, process lifecycle, filesystem state
- **Note**: These are NOT unit tests. They test the entire Telemetry daemon as a black box."""
    
    
    def _generate_executive_summary(self) -> str:
        """Generate executive summary section - REMOVED, included in header"""
        return ""
    
    
    def _generate_visual_section(self) -> str:
        """Generate functional module coverage map"""
        
        # Calculate functional coverage per module based on feature files
        module_coverage = self._calculate_functional_module_coverage()
        
        lines = ["```mermaid", "mindmap", "  root((Telemetry 2.0))"]
        
        for module_name, coverage_info in sorted(module_coverage.items()):
            coverage_pct = coverage_info['coverage']
            
            if coverage_pct >= 75:
                status = "✅"
            elif coverage_pct >= 40:
                status = "⚠️"
            else:
                status = "❌"
            
            lines.append(f"    {module_name} {status} {coverage_pct:.0f}%")
        
        lines.append("```")
        mindmap = '\n'.join(lines)
        
        return f"""## Functional Module Coverage

{mindmap}

**Legend**:
- 🟢 ✅: >75% scenario coverage - Well tested
- 🟡 ⚠️: 40-75% scenario coverage - Partial coverage
- 🔴 ❌: <40% scenario coverage - Critical gap

**Note**: Coverage reflects functional scenarios tested, not API-level code coverage."""
    
    
    def _generate_feature_sync_section(self) -> str:
        """Generate feature-test synchronization section"""
        if not self.missing_implementations and not self.orphaned_tests:
            return ""
        
        sections = ["## Feature-Test Sync\n"]
        sections.append("| Feature | Scenarios | Tests | Status |")
        sections.append("|---------|-----------|-------|--------|")
        
        # Add a few key items
        for item in self.missing_implementations[:3]:
            feature = item['feature']
            impl = item.get('implemented', 0)
            total = item.get('total', len(item.get('scenarios', [])))
            sections.append(f"| {feature} | {total} | {impl} | ⚠️ Partial |")
        
        for item in self.orphaned_tests[:2]:
            test_file = item['test_file']
            count = item['count']
            sections.append(f"| {test_file} | 0 | {count} | 🔄 No feature file |")
        
        return '\n'.join(sections)
    
    
    def _generate_gap_analysis_section(self) -> str:
        """Generate concise gap analysis (top 5 only)"""
        sections = ["## Top 5 Critical Gaps\n"]
        
        for i, gap in enumerate(self.gaps[:5], 1):
            module = gap['module']
            coverage = gap['coverage']
            priority = gap['priority']
            untested_count = len(gap['untested_apis'])
            
            # Estimate recommended tests
            recommended_tests = min(untested_count, 8)
            
            sections.append(f"{i}. **{module.title()} Module** ({priority})")
            sections.append(f"   - Coverage: {coverage:.0f}% | Untested APIs: {untested_count}")
            sections.append(f"   - Recommended: {recommended_tests} tests\n")
        
        return '\n'.join(sections)
    
    
    def _generate_recommendations(self) -> str:
        """Generate concise recommendations"""
        return """## Quick Actions

- [ ] Create missing feature files for orphaned tests
- [ ] Implement high-priority test scenarios  
- [ ] Focus on critical modules (<40% coverage)
- [ ] Target: 90% coverage by next quarter"""
    
    
    # =========================================================================
    # Feature File Synchronization
    # =========================================================================
    
    def sync_feature_files(self) -> Dict[str, str]:
        """Synchronize .feature files based on implemented tests"""
        
        print("\n" + "="*80)
        print("Feature File Synchronization Mode")
        print("="*80)
        
        print("\n[1/4] Discovering test implementations...")
        tests = self.discover_test_files()
        print(f"  Found {len(tests)} test files")
        
        print("\n[2/4] Parsing existing feature files...")
        features = self.parse_feature_files()
        existing_feature_files = set(features.keys())
        print(f"  Found {len(features)} feature files")
        
        print("\n[3/4] Creating new feature files...")
        created_features = {}
        
        for test_file, test_list in tests.items():
            # Derive feature file name from test file
            # test_profile_race_conditions.py -> profile_race_conditions
            base_name = test_file.replace('test_', '').replace('.py', '')
            feature_name = base_name + '.feature'
            
            # Check if a similar feature file already exists using flexible matching
            # Remove underscores and compare normalized names
            normalized_base = base_name.lower().replace('_', '')
            
            found_existing = None
            for existing_file in existing_feature_files:
                # Normalize existing filename for comparison
                normalized_existing = existing_file.lower().replace('.feature', '').replace('_', '')
                
                # Check multiple matching criteria:
                # 1. Base contained in existing
                # 2. Existing ends with base
                # 3. >80% character overlap (allowing for singular/plural, etc.)
                if (normalized_base in normalized_existing or 
                    normalized_existing.endswith(normalized_base)):
                    found_existing = existing_file
                    break
                
                # Calculate similarity for close matches (e.g., communication vs communications)
                # Count common characters
                if len(normalized_base) > 5:  # Only for reasonable length names
                    shorter = min(len(normalized_base), len(normalized_existing))
                    longer = max(len(normalized_base), len(normalized_existing))
                    # Check if one is substring of the other OR they share >80% of characters
                    common_prefix_len = 0
                    for i in range(min(len(normalized_base), len(normalized_existing))):
                        if normalized_base[i] == normalized_existing[i]:
                            common_prefix_len += 1
                        else:
                            break
                    
                    similarity_ratio = common_prefix_len / longer
                    if similarity_ratio > 0.7:  # 70% similar
                        found_existing = existing_file
                        break
            
            if found_existing:
                print(f"  ⚠️  Skipping {feature_name} (found existing: {found_existing})")
                continue
            
            feature_path = self.features_dir / feature_name
            if feature_path.exists():
                print(f"  ⚠️  Skipping {feature_name} (file exists)")
                continue
            
            # Generate feature content
            feature_content = self._generate_feature_file(test_file, test_list)
            
            # Create features directory if needed
            self.features_dir.mkdir(parents=True, exist_ok=True)
            
            # Write feature file
            with open(feature_path, 'w') as f:
                f.write(feature_content)
            
            created_features[feature_name] = str(feature_path)
            print(f"  ✓ Created {feature_name}")
        
        print(f"\n[4/4] Updating existing feature files with missing scenarios...")
        updated_features = self._update_existing_features(tests, features)
        
        print(f"\n✓ Synchronization complete:")
        print(f"  - Created: {len(created_features)} new feature files")
        print(f"  - Updated: {len(updated_features)} existing feature files")
        print("="*80)
        
        return {**created_features, **updated_features}
    
    
    def _update_existing_features(self, tests: Dict[str, List[Dict]], features: Dict[str, List[str]]) -> Dict[str, str]:
        """Update existing feature files with missing test scenarios"""
        updated_features = {}
        
        for test_file, test_list in tests.items():
            # Find corresponding feature file
            base_name = test_file.replace('test_', '').replace('.py', '')
            
            # Try to find matching feature file
            matching_feature = None
            for feature_file in features.keys():
                # Flexible matching (same logic as sync_feature_files)
                normalized_base = base_name.lower().replace('_', '')
                normalized_feature = feature_file.lower().replace('.feature', '').replace('_', '')
                
                if (normalized_base in normalized_feature or 
                    normalized_feature.endswith(normalized_base)):
                    matching_feature = feature_file
                    break
                
                # Similarity check
                if len(normalized_base) > 5:
                    common_prefix_len = 0
                    for i in range(min(len(normalized_base), len(normalized_feature))):
                        if normalized_base[i] == normalized_feature[i]:
                            common_prefix_len += 1
                        else:
                            break
                    
                    longer = max(len(normalized_base), len(normalized_feature))
                    if common_prefix_len / longer > 0.7:
                        matching_feature = feature_file
                        break
            
            if not matching_feature:
                continue  # No existing feature to update
            
            # Get existing scenarios
            existing_scenarios = features[matching_feature]
            existing_scenario_names = set()
            for scenario in existing_scenarios:
                # Normalize scenario names for comparison
                normalized = scenario.lower().replace('_', ' ').strip()
                existing_scenario_names.add(normalized)
            
            # Find missing scenarios
            missing_tests = []
            for test in test_list:
                test_name = test['name'].replace('test_', '')
                # Convert to scenario-like name for comparison
                scenario_name = test_name.replace('_', ' ').lower().strip()
                
                # Check if this test is already covered
                if scenario_name not in existing_scenario_names:
                    missing_tests.append(test)
            
            if not missing_tests:
                continue  # No missing scenarios
            
            # Append missing scenarios to feature file
            feature_path = self.features_dir / matching_feature
            
            with open(feature_path, 'r') as f:
                content = f.read()
            
            # Check if file ends with newline
            if not content.endswith('\n'):
                content += '\n'
            
            # Add missing scenarios
            for test in missing_tests:
                test_name = test['name']
                description = test.get('description', '')
                
                scenario_name = test_name.replace('test_', '').replace('_', ' ').title()
                
                content += f"\n  Scenario: {scenario_name}\n"
                
                if description:
                    content += f"    # {description}\n"
                
                content += f"    Given the telemetry system is running\n"
                content += f"    When {scenario_name.lower()} is executed\n"
                content += f"    Then the system should handle it correctly\n"
                content += f"    And no errors or crashes should occur\n"
            
            # Write updated content
            with open(feature_path, 'w') as f:
                f.write(content)
            
            updated_features[matching_feature] = str(feature_path)
            print(f"  ✓ Updated {matching_feature} (added {len(missing_tests)} scenarios)")
        
        return updated_features
    
    
    def _generate_feature_file(self, test_file: str, tests: List[Dict[str, str]]) -> str:
        """Generate Gherkin feature file content from test implementations"""
        
        # Derive feature name from test file
        # test_profile_race_conditions.py -> Profile Race Conditions
        feature_title = test_file.replace('test_', '').replace('.py', '').replace('_', ' ').title()
        
        # Build feature file content
        lines = []
        lines.append("Feature: " + feature_title)
        lines.append("")
        lines.append(f"  As a QA engineer")
        lines.append(f"  I want to validate {feature_title.lower()} functionality")
        lines.append(f"  So that the telemetry system operates correctly")
        lines.append("")
        
        # Add scenarios for each test
        for test in tests:
            test_name = test['name']
            description = test.get('description', '')
            
            # Convert test_function_name to Scenario Name
            # test_toctou_deleteAllProfiles -> TOCTOU DeleteAllProfiles
            scenario_name = test_name.replace('test_', '').replace('_', ' ').title()
            
            lines.append(f"  Scenario: {scenario_name}")
            
            if description:
                # Use docstring as scenario description
                lines.append(f"    # {description}")
            
            # Generate basic Gherkin steps
            lines.append(f"    Given the telemetry system is running")
            lines.append(f"    When {scenario_name.lower()} is executed")
            lines.append(f"    Then the system should handle it correctly")
            lines.append(f"    And no errors or crashes should occur")
            lines.append("")
        
        return '\n'.join(lines)
    
    
    def _get_timestamp(self) -> str:
        """Get current timestamp"""
        from datetime import datetime
        return datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    
    
    # =========================================================================
    # Main Execution
    # =========================================================================
    
    def run_analysis(self, output_file: str = "L2_TEST_GAP.md"):
        """Run complete analysis and generate report"""
        
        print("=" * 80)
        print("L2 Test Gap Analyzer")
        print("=" * 80)
        
        print("\n[1/7] Parsing feature files...")
        features = self.parse_feature_files()
        print(f"  Found {len(features)} feature files")
        
        print("\n[2/7] Discovering test implementations...")
        tests = self.discover_test_files()
        print(f"  Found {len(tests)} test files")
        
        print("\n[3/7] Analyzing source code modules...")
        modules = self.analyze_source_modules()
        print(f"  Analyzed {len(modules)} modules")
        
        print("\n[4/7] Mapping source to tests...")
        self.coverage_data = self.map_source_to_tests(modules, tests)
        
        # Calculate functional test coverage (scenarios implemented)
        total_scenarios = sum(item.get('total', len(item.get('scenarios', []))) for item in [])
        implemented_scenarios = 0
        for impl in self.missing_implementations:
            total_scenarios += impl.get('total', len(impl.get('scenarios', [])))
            implemented_scenarios += impl.get('implemented', 0)
        
        functional_coverage = (implemented_scenarios / total_scenarios * 100) if total_scenarios > 0 else 0
        
        # Count test functions
        test_count = 0
        for test_file in tests.values():
            test_count += len(test_file)
        
        print(f"  Test functions: {test_count}")
        print(f"  Functional coverage: {functional_coverage:.1f}% ({implemented_scenarios}/{total_scenarios} scenarios)")
        
        print("\n[5/7] Identifying gaps...")
        self.identify_gaps(features, tests, self.coverage_data)
        print(f"  Found {len(self.gaps)} coverage gaps")
        print(f"  Found {len(self.orphaned_tests)} orphaned tests")
        print(f"  Found {len(self.missing_implementations)} missing implementations")
        
        print("\n[6/7] Generating report...")
        report = self.generate_report()
        
        print("\n[7/7] Writing output...")
        output_path = self.test_dir / output_file
        with open(output_path, 'w') as f:
            f.write(report)
        
        # Calculate final statistics
        total_scenarios = sum(item.get('total', len(item.get('scenarios', []))) for item in self.missing_implementations)
        implemented_scenarios = sum(item.get('implemented', 0) for item in self.missing_implementations)
        functional_coverage = (implemented_scenarios / total_scenarios * 100) if total_scenarios > 0 else 0
        test_count = sum(len(t) for t in tests.values())
        
        print(f"\n✓ Report generated: {output_path}")
        print(f"  Test functions: {test_count}")
        print(f"  Functional coverage: {functional_coverage:.1f}% ({implemented_scenarios}/{total_scenarios} scenarios)")
        print(f"  Critical gaps: {sum(1 for g in self.gaps if g['priority'] == 'CRITICAL')}")
        print("=" * 80)
        
        return output_path


# =============================================================================
# CLI Interface
# =============================================================================

if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description='L2 Test Gap Analyzer - Analyze test coverage and sync feature files'
    )
    parser.add_argument(
        '--workspace',
        default=os.getcwd(),
        help='Workspace root directory (default: current directory)'
    )
    parser.add_argument(
        '--sync-features',
        action='store_true',
        help='Synchronize .feature files from test implementations'
    )
    parser.add_argument(
        '--output',
        default='L2_TEST_GAP.md',
        help='Output file name for gap analysis report (default: L2_TEST_GAP.md)'
    )
    
    args = parser.parse_args()
    
    analyzer = TestGapAnalyzer(args.workspace)
    
    if args.sync_features:
        # Feature sync mode
        created = analyzer.sync_feature_files()
        if created:
            print(f"\n✓ Created {len(created)} feature files:")
            for name, path in created.items():
                print(f"  - {name}: {path}")
    else:
        # Analysis mode (default)
        analyzer.run_analysis(args.output)

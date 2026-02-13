#!/bin/bash

# Comprehensive memory leak testing script for curl_leak_test
# Usage: ./run_leak_test.sh [options]

set -e

echo "=== Curl Memory Leak Detection Test Suite ==="
echo ""

# Default values
ITERATIONS=20
INTERVAL=2
BUILD_TYPE="basic"
TEST_TYPE="run"
VERBOSE=false

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${GREEN}✓${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}⚠${NC} $1"
}

print_error() {
    echo -e "${RED}✗${NC} $1"
}

# Function to check if required tools are available
check_dependencies() {
    local missing_deps=0
    
    if ! command -v gcc &> /dev/null; then
        print_error "gcc compiler not found"
        missing_deps=1
    fi
    
    if ! command -v make &> /dev/null; then
        print_error "make build tool not found"
        missing_deps=1
    fi
    
    if ! pkg-config --exists libcurl; then
        print_error "libcurl development headers not found"
        print_warning "Install with: sudo apt-get install libcurl4-openssl-dev"
        missing_deps=1
    fi
    
    if [[ "$TEST_TYPE" == "valgrind" || "$TEST_TYPE" == "massif" ]] && ! command -v valgrind &> /dev/null; then
        print_error "valgrind not found for memory analysis"
        print_warning "Install with: sudo apt-get install valgrind"
        missing_deps=1
    fi
    
    if [ $missing_deps -eq 1 ]; then
        print_error "Missing dependencies. Please install them and try again."
        exit 1
    fi
    
    print_status "All dependencies satisfied"
}

# Function to display help
show_help() {
    cat << EOF
Curl Memory Leak Detection Test Suite

USAGE:
    $0 [OPTIONS]

OPTIONS:
    -i, --iterations N      Number of iterations (default: 20)
    -t, --interval N        Interval in seconds (default: 2)
    -v, --verbose          Enable verbose output
    
BUILD OPTIONS:
    --basic                Build basic version (default)
    --mtls                 Build with mTLS support
    --rdkb                 Build with RDKB support
    --full                 Build with all features
    
TEST OPTIONS:
    --run                  Run basic memory test (default)
    --quick                Quick test (5 iterations, 1s interval)
    --valgrind             Run with Valgrind leak detection
    --massif               Run with Massif memory profiler
    --stress               Run stress test (100 iterations)
    --all-tests            Run all test types sequentially
    
OTHER:
    --clean                Clean build artifacts and exit
    -h, --help             Show this help message

EXAMPLES:
    $0                                    # Basic test
    $0 --mtls --valgrind                 # mTLS build with leak detection
    $0 -i 50 -t 1 --stress               # Stress test: 50 iterations, 1s interval
    $0 --full --all-tests                # Full build, run all test types

INTERPRETING RESULTS:
    Memory Growth < 100KB:  Normal (likely not a leak)
    Memory Growth > 1MB:    Potential leak (investigate)
    
    Valgrind "definitely lost": Memory leak confirmed
    Valgrind "still reachable": May be acceptable (cleanup on exit)

EOF
}

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -i|--iterations)
            ITERATIONS="$2"
            shift 2
            ;;
        -t|--interval)
            INTERVAL="$2"
            shift 2
            ;;
        -v|--verbose)
            VERBOSE=true
            shift
            ;;
        --basic)
            BUILD_TYPE="basic"
            shift
            ;;
        --mtls)
            BUILD_TYPE="mtls"
            shift
            ;;
        --rdkb)
            BUILD_TYPE="rdkb"
            shift
            ;;
        --full)
            BUILD_TYPE="full"
            shift
            ;;
        --run)
            TEST_TYPE="run"
            shift
            ;;
        --quick)
            TEST_TYPE="quick"
            ITERATIONS=5
            INTERVAL=1
            shift
            ;;
        --valgrind)
            TEST_TYPE="valgrind"
            shift
            ;;
        --massif)
            TEST_TYPE="massif"
            shift
            ;;
        --stress)
            TEST_TYPE="stress"
            ITERATIONS=100
            INTERVAL=1
            shift
            ;;
        --all-tests)
            TEST_TYPE="all"
            shift
            ;;
        --clean)
            echo "Cleaning build artifacts..."
            make clean
            print_status "Clean complete"
            exit 0
            ;;
        -h|--help)
            show_help
            exit 0
            ;;
        *)
            print_error "Unknown option: $1"
            echo "Use --help for usage information"
            exit 1
            ;;
    esac
done

# Check dependencies
check_dependencies

# Build the application
echo "Building curl_leak_test with $BUILD_TYPE configuration..."
if [ "$VERBOSE" = true ]; then
    make build-$BUILD_TYPE
else
    make build-$BUILD_TYPE > /dev/null 2>&1
fi
print_status "Build complete"

# Function to run a single test
run_single_test() {
    local test_name="$1"
    local test_cmd="$2"
    
    echo ""
    echo "========================================"
    echo "Running $test_name"
    echo "========================================"
    
    if [ "$VERBOSE" = true ]; then
        eval $test_cmd
    else
        eval $test_cmd
    fi
    
    print_status "$test_name completed"
}

# Function to analyze results
analyze_results() {
    echo ""
    echo "========================================"
    echo "ANALYSIS SUMMARY"
    echo "========================================"
    
    # Check if valgrind report exists
    if [ -f "valgrind_report.txt" ]; then
        echo ""
        echo "=== Valgrind Memory Leak Analysis ==="
        
        # Check for memory leaks
        definitely_lost=$(grep "definitely lost:" valgrind_report.txt | awk '{print $4}' | sed 's/,//g' || echo "0")
        indirectly_lost=$(grep "indirectly lost:" valgrind_report.txt | awk '{print $4}' | sed 's/,//g' || echo "0")
        
        if [ "$definitely_lost" != "0" ] || [ "$indirectly_lost" != "0" ]; then
            print_error "Memory leaks detected!"
            echo "  Definitely lost: $definitely_lost bytes"
            echo "  Indirectly lost: $indirectly_lost bytes"
            echo "  See valgrind_report.txt for details"
        else
            print_status "No memory leaks detected by Valgrind"
        fi
        
        # Check for errors
        error_count=$(grep "ERROR SUMMARY:" valgrind_report.txt | awk '{print $4}' || echo "unknown")
        if [ "$error_count" != "0" ] && [ "$error_count" != "unknown" ]; then
            print_warning "$error_count errors reported by Valgrind"
        fi
    fi
    
    # Check if massif report exists
    if [ -f "massif_report.txt" ]; then
        echo ""
        echo "=== Massif Memory Growth Analysis ==="
        
        peak_usage=$(grep -A 1 "Peak" massif_report.txt | tail -1 | awk '{print $1}' || echo "unknown")
        if [ "$peak_usage" != "unknown" ]; then
            echo "  Peak memory usage: $peak_usage"
        fi
        
        # Look for growing allocations
        if grep -q "growing" massif_report.txt; then
            print_warning "Growing memory allocations detected"
            echo "  Check massif_report.txt for details"
        fi
    fi
    
    # Check if stress test log exists
    if [ -f "stress_test.log" ]; then
        echo ""
        echo "=== Stress Test Memory Growth Analysis ==="
        
        # Extract memory growth information
        if grep -q "Memory Usage Summary" stress_test.log; then
            memory_growth=$(grep "Growth.*VmRSS:" stress_test.log | tail -1 | awk '{print $6}')
            if [ ! -z "$memory_growth" ]; then
                growth_kb=$(echo $memory_growth | sed 's/[+kB]//g')
                if [ "$growth_kb" -gt 1024 ]; then
                    print_error "Significant memory growth: $memory_growth"
                    echo "  This indicates a potential memory leak"
                elif [ "$growth_kb" -gt 100 ]; then
                    print_warning "Moderate memory growth: $memory_growth"
                    echo "  Monitor for continued growth"
                else
                    print_status "Memory usage stable: $memory_growth"
                fi
            fi
        fi
    fi
    
    echo ""
    echo "=== Recommendations ==="
    if [ -f "valgrind_report.txt" ] && grep -q "definitely lost.*[1-9]" valgrind_report.txt; then
        echo "• Fix memory leaks shown in valgrind_report.txt"
        echo "• Focus on 'definitely lost' allocations first"
    fi
    
    if [ -f "massif_report.txt" ]; then
        echo "• Review massif_report.txt for memory allocation patterns"
        echo "• Look for functions that allocate but don't free memory"
    fi
    
    echo "• Run tests multiple times to confirm results"
    echo "• Use different iteration counts to verify growth patterns"
}

# Run tests based on selected type
case $TEST_TYPE in
    run)
        echo "Running basic test: $ITERATIONS iterations, ${INTERVAL}s interval"
        ./curl_leak_test $ITERATIONS $INTERVAL
        ;;
    quick)
        echo "Running quick test: 5 iterations, 1s interval"
        ./curl_leak_test 5 1
        ;;
    valgrind)
        run_single_test "Valgrind Memory Leak Detection" "make valgrind"
        ;;
    massif)
        run_single_test "Massif Memory Profiling" "make massif"
        ;;
    stress)
        run_single_test "Stress Test" "make stress"
        ;;
    all)
        echo "Running comprehensive test suite..."
        run_single_test "Quick Test" "./curl_leak_test 5 1"
        run_single_test "Valgrind Leak Detection" "make valgrind"
        run_single_test "Massif Memory Profiling" "make massif"
        run_single_test "Stress Test" "make stress"
        ;;
esac

# Analyze results
analyze_results

echo ""
print_status "Memory leak detection test suite completed!"
echo ""
echo "Files generated:"
if [ -f "valgrind_report.txt" ]; then echo "  • valgrind_report.txt - Detailed leak analysis"; fi
if [ -f "massif_report.txt" ]; then echo "  • massif_report.txt - Memory profiling data"; fi
if [ -f "stress_test.log" ]; then echo "  • stress_test.log - Stress test output"; fi
echo ""
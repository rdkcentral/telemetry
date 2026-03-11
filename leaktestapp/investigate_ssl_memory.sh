#!/bin/bash

# SSL Memory Investigation Script
# Purpose: Investigate the 3484KB → 3608KB SSL/OpenSSL memory increase
# with the reset fix as identified by the manager

echo "🔍 SSL MEMORY INCREASE INVESTIGATION"
echo "======================================"
echo ""
echo "Manager's Concern: SSL/OpenSSL memory increased from 3484 KB to 3608 KB (+124 KB)"
echo "with the reset fix. Need to determine if this is a leak or normal behavior."
echo ""

# Check if memory_verifier is compiled
if [ ! -f "./memory_verifier" ]; then
    echo "❌ memory_verifier not found. Compiling..."
    make clean && make
    if [ $? -ne 0 ]; then
        echo "❌ Compilation failed. Please fix build issues."
        exit 1
    fi
fi

# Check if valgrind is available
if ! command -v valgrind &> /dev/null; then
    echo "❌ Valgrind not found. Please install valgrind:"
    echo "   Ubuntu/Debian: sudo apt-get install valgrind"
    echo "   CentOS/RHEL: sudo yum install valgrind"
    echo "   MacOS: brew install valgrind"
    exit 1
fi

echo "✅ Prerequisites check passed"
echo ""
echo "🚀 Starting investigation workflow..."
echo ""

# Step 1: Baseline pool analysis (without reset)
echo "📊 STEP 1: Baseline Analysis (Pool without Reset)"
echo "------------------------------------------------"
./memory_verifier --valgrind-pool
if [ $? -ne 0 ]; then
    echo "⚠️  Baseline test had issues, but continuing..."
fi
echo ""

# Step 2: Comparative post-analysis
echo "📊 STEP 3: Comparative Analysis"
echo "-------------------------------"
./memory_verifier --post-analysis
echo ""

# Step 3: Deep SSL investigation (optional)
echo "📊 STEP 4: Deep SSL Investigation (Optional)"
echo "--------------------------------------------"
read -p "Run detailed SSL/OpenSSL memory profiling? (y/N): " -n 1 -r
echo ""
if [[ $REPLY =~ ^[Yy]$ ]]; then
    ./memory_verifier --valgrind-ssl
    echo ""
fi

# Generate final summary
echo "📋 INVESTIGATION SUMMARY"
echo "========================"
echo ""
echo "Generated Files:"
echo "----------------"
ls -la *.log *.out *.txt 2>/dev/null | grep -E "(valgrind|massif|analysis)" || echo "No analysis files found"
echo ""

echo "📂 Key Files to Review:"
echo "----------------------"
if [ -f "valgrind_pool_baseline.log" ]; then
    echo "✅ valgrind_pool_baseline.log - Baseline memory behavior"
fi
if [ -f "post_test_analysis_summary.txt" ]; then
    echo "✅ post_test_analysis_summary.txt - Complete impact analysis"
fi
if [ -f "massif_ssl_investigation.out" ]; then
    echo "✅ massif_ssl_investigation.out - Detailed SSL memory profile"
fi
if [ -f "massif_ssl_report.txt" ]; then
    echo "✅ massif_ssl_report.txt - Human-readable SSL memory report"
fi

echo ""
echo "🎯 Next Steps:"
echo "1. Review post_test_analysis_summary.txt for leak impact assessment"
echo "2. Check valgrind logs for specific SSL/OpenSSL memory patterns"
echo "3. If massif data available, analyze memory allocation hot spots" 
echo "4. Share findings with manager to justify reset fix safety"
echo ""
echo "✅ Investigation complete!"


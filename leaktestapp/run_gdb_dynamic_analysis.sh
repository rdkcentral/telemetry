#!/bin/bash
# Dynamic SSL Memory Analysis Script
# Usage: ./run_dynamic_analysis.sh [timeout_seconds] [iterations]
# Example: ./run_dynamic_analysis.sh 30 25
# 
# This script performs real-time analysis of SSL memory allocations
# during memory_verifier execution to complement static massif reports

# Get timeout from command line argument, default to 60 seconds
TIMEOUT_SECONDS=${1:-60}

# Get iteration count from command line argument, optional
ITERATIONS=${2}

OUTPUT_DIR="dynamic_analysis_results"
TIMESTAMP=$(date +"%Y%m%d_%H%M%S")
LOG_PREFIX="${OUTPUT_DIR}/dynamic_ssl_${TIMESTAMP}"

# Create output directory
mkdir -p "$OUTPUT_DIR"

echo "🔍 Starting Dynamic SSL Memory Analysis"
echo "   Timeout: $TIMEOUT_SECONDS seconds"
if [ -n "$ITERATIONS" ]; then
    echo "   Iterations: $ITERATIONS"
else
    echo "   Iterations: Using memory_verifier default (50)"
fi
echo "   Timestamp: $TIMESTAMP"
echo "   Output directory: $OUTPUT_DIR"
echo "   Log prefix: $LOG_PREFIX"
echo ""

# Check if memory_verifier exists
if [ ! -f "./memory_verifier" ]; then
    echo "❌ memory_verifier not found. Compiling..."
    gcc -g -O0 -Wall -Wextra -o memory_verifier memory_verifier.c -lcurl -lpthread
    if [ $? -ne 0 ]; then
        echo "❌ Compilation failed. Exiting."
        exit 1
    fi
    echo "✅ Compiled successfully"
fi

echo "=== 1. GDB Call Stack Analysis ===" | tee "${LOG_PREFIX}_analysis.log"

# Check if GDB script exists
if [ ! -f "gdb_batch_script.gdb" ]; then
    echo "❌ gdb_batch_script.gdb not found. Please ensure it exists in the current directory." | tee -a "${LOG_PREFIX}_analysis.log"
    exit 1
fi

echo "Running GDB dynamic analysis using gdb_batch_script.gdb..." | tee -a "${LOG_PREFIX}_analysis.log"
if [ -n "$ITERATIONS" ]; then
    echo "Setting ${TIMEOUT_SECONDS}-second timeout with $ITERATIONS iterations..." | tee -a "${LOG_PREFIX}_analysis.log"
else  
    echo "Setting ${TIMEOUT_SECONDS}-second timeout with default iterations..." | tee -a "${LOG_PREFIX}_analysis.log"
fi

# Use timeout command to automatically stop GDB after specified seconds
if [ -n "$ITERATIONS" ]; then
    echo "Setting T2_TEST_ITERATIONS=$ITERATIONS for this test..." | tee -a "${LOG_PREFIX}_analysis.log"
    T2_TEST_ITERATIONS=$ITERATIONS timeout ${TIMEOUT_SECONDS}s gdb -batch -x gdb_batch_script.gdb --args ./memory_verifier --pool 2>&1 | tee "${LOG_PREFIX}_gdb_trace.log"
else
    timeout ${TIMEOUT_SECONDS}s gdb -batch -x gdb_batch_script.gdb --args ./memory_verifier --pool 2>&1 | tee "${LOG_PREFIX}_gdb_trace.log"
fi

GDB_EXIT_CODE=$?
if [ $GDB_EXIT_CODE -eq 124 ]; then
    echo "⏰ GDB analysis stopped after $TIMEOUT_SECONDS seconds (timeout)" | tee -a "${LOG_PREFIX}_analysis.log"
elif [ $GDB_EXIT_CODE -eq 0 ]; then
    echo "✅ GDB analysis completed normally" | tee -a "${LOG_PREFIX}_analysis.log"
else
    echo "⚠️  GDB analysis stopped with exit code: $GDB_EXIT_CODE" | tee -a "${LOG_PREFIX}_analysis.log"
fi

echo ""
echo "=== Dynamic Analysis Summary ===" | tee -a "${LOG_PREFIX}_analysis.log"
echo "Analysis completed at: $(date)" | tee -a "${LOG_PREFIX}_analysis.log"
echo "" | tee -a "${LOG_PREFIX}_analysis.log"

# Generate summary of results
echo "📁 Generated Files:" | tee -a "${LOG_PREFIX}_analysis.log"
ls -la "${OUTPUT_DIR}"/dynamic_ssl_${TIMESTAMP}_* 2>/dev/null | tee -a "${LOG_PREFIX}_analysis.log"

echo "" | tee -a "${LOG_PREFIX}_analysis.log"
echo "📊 Quick Analysis:" | tee -a "${LOG_PREFIX}_analysis.log"

# Quick analysis of GDB trace
if [ -f "${LOG_PREFIX}_gdb_trace.log" ]; then
    # Count allocations
    EVP_COUNT=$(grep -c "EVP_CIPHER_fetch" "${LOG_PREFIX}_gdb_trace.log" 2>/dev/null || echo "0")
    CRYPTO_ALLOC_COUNT=$(grep -c "CRYPTO_zalloc\|CRYPTO_malloc" "${LOG_PREFIX}_gdb_trace.log" 2>/dev/null || echo "0")
    RAND_COUNT=$(grep -c "RAND_get0_primary" "${LOG_PREFIX}_gdb_trace.log" 2>/dev/null || echo "0")
    
    # Count deallocations  
    CRYPTO_FREE_COUNT=$(grep -c "CRYPTO_free" "${LOG_PREFIX}_gdb_trace.log" 2>/dev/null || echo "0")
    EVP_FREE_COUNT=$(grep -c "EVP_CIPHER_free" "${LOG_PREFIX}_gdb_trace.log" 2>/dev/null || echo "0")
    SSL_CTX_FREE_COUNT=$(grep -c "SSL_CTX_free" "${LOG_PREFIX}_gdb_trace.log" 2>/dev/null || echo "0")
    
    echo "   === ALLOCATION ANALYSIS ===" | tee -a "${LOG_PREFIX}_analysis.log"
    echo "   EVP_CIPHER_fetch calls: $EVP_COUNT" | tee -a "${LOG_PREFIX}_analysis.log"
    echo "   CRYPTO allocation calls: $CRYPTO_ALLOC_COUNT" | tee -a "${LOG_PREFIX}_analysis.log"  
    echo "   RAND_get0_primary calls: $RAND_COUNT" | tee -a "${LOG_PREFIX}_analysis.log"
    
    echo "   === DEALLOCATION ANALYSIS ===" | tee -a "${LOG_PREFIX}_analysis.log"
    echo "   CRYPTO_free calls: $CRYPTO_FREE_COUNT" | tee -a "${LOG_PREFIX}_analysis.log"
    echo "   EVP_CIPHER_free calls: $EVP_FREE_COUNT" | tee -a "${LOG_PREFIX}_analysis.log"  
    echo "   SSL_CTX_free calls: $SSL_CTX_FREE_COUNT" | tee -a "${LOG_PREFIX}_analysis.log"
    
    # MEMORY LEAK ANALYSIS
    echo "   === MEMORY LEAK ASSESSMENT ===" | tee -a "${LOG_PREFIX}_analysis.log"
    TOTAL_ALLOCS=$((EVP_COUNT + CRYPTO_ALLOC_COUNT))
    TOTAL_FREES=$((CRYPTO_FREE_COUNT + EVP_FREE_COUNT + SSL_CTX_FREE_COUNT))
    
    if [ $TOTAL_ALLOCS -gt 0 ] && [ $TOTAL_FREES -gt 0 ]; then
        if [ $TOTAL_ALLOCS -eq $TOTAL_FREES ]; then
            echo "   ✅ LIKELY NO LEAK: Allocations ($TOTAL_ALLOCS) = Deallocations ($TOTAL_FREES)" | tee -a "${LOG_PREFIX}_analysis.log"
        elif [ $TOTAL_ALLOCS -gt $TOTAL_FREES ]; then
            LEAK_DIFF=$((TOTAL_ALLOCS - TOTAL_FREES))
            echo "   ⚠️  POTENTIAL LEAK: $LEAK_DIFF more allocations than deallocations" | tee -a "${LOG_PREFIX}_analysis.log"
        else
            echo "   🔍 UNUSUAL: More deallocations than allocations (cleanup phase?)" | tee -a "${LOG_PREFIX}_analysis.log"
        fi
    elif [ $TOTAL_ALLOCS -gt 0 ] && [ $TOTAL_FREES -eq 0 ]; then
        echo "   🎯 OPERATIONAL MEMORY: $TOTAL_ALLOCS allocations, no immediate frees" | tee -a "${LOG_PREFIX}_analysis.log"
        echo "   📝 This suggests initialization memory (SSL contexts, ciphers)" | tee -a "${LOG_PREFIX}_analysis.log"
        if [ -n "$ITERATIONS" ]; then
            echo "   📊 During $ITERATIONS iterations - likely explains memory increase" | tee -a "${LOG_PREFIX}_analysis.log"
        else
            echo "   📊 Likely explains your memory increase - legitimate operational memory" | tee -a "${LOG_PREFIX}_analysis.log"
        fi
    else
        echo "   ℹ️  Insufficient data for leak analysis" | tee -a "${LOG_PREFIX}_analysis.log"
    fi
fi

echo "" | tee -a "${LOG_PREFIX}_analysis.log"
echo "✅ GDB analysis completed successfully!"
echo "📋 Main results in: ${LOG_PREFIX}_analysis.log"
echo "🔍 To analyze results, examine:"
echo "   • ${LOG_PREFIX}_gdb_trace.log (call stacks and function calls)"
echo "   • dynamic_gdb_callstack.log (detailed GDB output)"

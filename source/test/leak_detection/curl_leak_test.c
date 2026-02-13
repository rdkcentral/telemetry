/*
 * Standalone application to test multicurlinterface.c for memory leaks
 * This app periodically sends HTTP requests to identify memory leak sources
 * Integrated with telemetry build system for device deployment
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <stdarg.h>

// FIX: Use telemetry headers - will be found via autotools include paths
#include "telemetry2_0.h"
#include "multicurlinterface.h"  // FIX: Add this header for http_pool_* functions

// FIX: Remove stub functions - these will be provided by linked telemetry libraries
// The http library (libhttp.la) provides the multicurlinterface functions
// The xconf-client library provides mTLS functions
// The utils library provides logging functions

// Configuration
#define TEST_URL "https://httpbin.org/post"
#define TEST_GET_URL "https://httpbin.org/get"
#define TEST_INTERVAL_SECONDS 5
#define MAX_ITERATIONS 100  // 0 for infinite loop

// FIX: Add memory tracking structure
typedef struct {
    long initial_vmrss;
    long initial_vmsize;
    long peak_vmrss;
    long peak_vmsize;
    long current_vmrss;
    long current_vmsize;
} memory_stats_t;

static memory_stats_t mem_stats = {0};

// Global flag for graceful shutdown
static volatile sig_atomic_t keep_running = 1;

void signal_handler(int sig)
{
    printf("\nReceived signal %d, shutting down gracefully...\n", sig);
    keep_running = 0;
}

// FIX: Enhanced memory tracking function
void update_memory_stats(void)
{
    FILE *fp = fopen("/proc/self/status", "r");
    if (!fp)
    {
        return;
    }

    char line[256];
    long vmrss = 0, vmsize = 0;
    
    while (fgets(line, sizeof(line), fp))
    {
        if (strncmp(line, "VmRSS:", 6) == 0)
        {
            sscanf(line + 6, "%ld", &vmrss);
        }
        else if (strncmp(line, "VmSize:", 7) == 0)
        {
            sscanf(line + 7, "%ld", &vmsize);
        }
    }
    fclose(fp);
    
    mem_stats.current_vmrss = vmrss;
    mem_stats.current_vmsize = vmsize;
    
    // Update peaks
    if (vmrss > mem_stats.peak_vmrss)
        mem_stats.peak_vmrss = vmrss;
    if (vmsize > mem_stats.peak_vmsize)
        mem_stats.peak_vmsize = vmsize;
    
    // Set initial if first call
    if (mem_stats.initial_vmrss == 0)
    {
        mem_stats.initial_vmrss = vmrss;
        mem_stats.initial_vmsize = vmsize;
        mem_stats.peak_vmrss = vmrss;
        mem_stats.peak_vmsize = vmsize;
    }
}

void print_memory_usage(void)
{
    update_memory_stats();
    printf("VmSize: %8ld kB  |  VmRSS: %8ld kB\n", 
           mem_stats.current_vmsize, mem_stats.current_vmrss);
}

// FIX: Add memory summary function
void print_memory_summary(void)
{
    printf("\n=== Memory Usage Summary ===\n");
    printf("Initial - VmSize: %8ld kB  |  VmRSS: %8ld kB\n", 
           mem_stats.initial_vmsize, mem_stats.initial_vmrss);
    printf("Peak    - VmSize: %8ld kB  |  VmRSS: %8ld kB\n", 
           mem_stats.peak_vmsize, mem_stats.peak_vmrss);
    printf("Final   - VmSize: %8ld kB  |  VmRSS: %8ld kB\n", 
           mem_stats.current_vmsize, mem_stats.current_vmrss);
    printf("Growth  - VmSize: %+8ld kB  |  VmRSS: %+8ld kB\n", 
           mem_stats.current_vmsize - mem_stats.initial_vmsize,
           mem_stats.current_vmrss - mem_stats.initial_vmrss);
    
    // Detect potential leak
    long rss_growth = mem_stats.current_vmrss - mem_stats.initial_vmrss;
    if (rss_growth > 1024)  // More than 1MB growth
    {
        printf("\n⚠️  WARNING: Significant memory growth detected (%ld kB)\n", rss_growth);
        printf("    This may indicate a memory leak!\n");
    }
    else if (rss_growth > 0)
    {
        printf("\n✓ Memory usage appears stable (growth: %ld kB)\n", rss_growth);
    }
    else
    {
        printf("\n✓ Memory usage decreased (change: %ld kB)\n", rss_growth);
    }
}

void test_http_post(int iteration)
{
    printf("\n[Iteration %d] Testing HTTP POST...\n", iteration);
    
    // Create test payload
    char payload[512];
    snprintf(payload, sizeof(payload), 
             "{\"iteration\":%d,\"timestamp\":%lld,\"test\":\"memory_leak_detection\"}",
             iteration, (long long)time(NULL));

    T2ERROR result = http_pool_post(TEST_URL, payload);
    
    // FIX: Update memory stats after each request
    update_memory_stats();
    
    if (result == T2ERROR_SUCCESS)
    {
        printf("[Iteration %d] POST request successful\n", iteration);
    }
    else
    {
        printf("[Iteration %d] POST request failed with error: %d\n", iteration, result);
    }
}

void test_http_get(int iteration)
{
    printf("\n[Iteration %d] Testing HTTP GET...\n", iteration);
    
    char *response_data = NULL;
    T2ERROR result = http_pool_get(TEST_GET_URL, &response_data, false);
    
    // FIX: Update memory stats after each request
    update_memory_stats();
    
    if (result == T2ERROR_SUCCESS)
    {
        printf("[Iteration %d] GET request successful\n", iteration);
        if (response_data)
        {
            printf("[Iteration %d] Response size: %zu bytes\n", iteration, strlen(response_data));
            // FIX: Show first 100 chars for debugging
            printf("[Iteration %d] Response preview: %.100s%s\n", 
                   iteration, 
                   response_data, 
                   strlen(response_data) > 100 ? "..." : "");
            free(response_data);
            response_data = NULL;  // FIX: Set to NULL after free
        }
        else
        {
            printf("[Iteration %d] WARNING: Success but response_data is NULL\n", iteration);
        }
    }
    else
    {
        printf("[Iteration %d] GET request failed with error: %d\n", iteration, result);
        // FIX: Clean up response_data even on failure
        if (response_data)
        {
            free(response_data);
            response_data = NULL;
        }
    }
}

void test_mixed_requests(int iteration)
{
#if 0
    // Alternate between GET and POST
    if (iteration % 2 == 0)
    {
        test_http_post(iteration);
    }
    else
    {
        test_http_get(iteration);
    }
#endif
#if 1
    test_http_post(iteration);
#endif
}

int main(int argc, char *argv[])
{
    printf("=== Curl Memory Leak Test Application ===\n");
    printf("This app will periodically send HTTP requests to detect memory leaks\n");
    printf("Press Ctrl+C to stop gracefully\n\n");

    // Setup signal handlers for graceful shutdown
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    // Parse command line arguments
    int max_iterations = MAX_ITERATIONS;
    int interval = TEST_INTERVAL_SECONDS;
    
    if (argc > 1)
    {
        max_iterations = atoi(argv[1]);
        printf("Using max iterations: %d%s\n", 
               max_iterations, max_iterations == 0 ? " (infinite)" : "");
    }
    
    if (argc > 2)
    {
        interval = atoi(argv[2]);
        printf("Using interval: %d seconds\n", interval);
    }

    // FIX: Set environment variable for pool size with error checking
    if (setenv("T2_CONNECTION_POOL_SIZE", "2", 1) != 0)
    {
        printf("Warning: Failed to set T2_CONNECTION_POOL_SIZE environment variable\n");
    }
    
    // FIX: Remove init_connection_pool() call since it's static in multicurlinterface.c
    // The pool will auto-initialize on first http_pool_get/post call
    printf("\nConnection pool will initialize on first request...\n");

    printf("\nInitial memory usage:\n");
    print_memory_usage();

    int iteration = 0;
    while (keep_running && (max_iterations == 0 || iteration < max_iterations))
    {
        iteration++;
        
        printf("\n========================================\n");
        printf("Iteration: %d", iteration);
        if (max_iterations > 0) 
        {
            printf(" / %d", max_iterations);
        }
        printf("\n========================================\n");

        // Run the test
        test_mixed_requests(iteration);

        // Print memory usage every 5 iterations  
        if (iteration % 5 == 0)
        {
            printf("\n--- Memory Usage at iteration %d ---\n", iteration);
            print_memory_usage();
        }

        // Sleep before next iteration
        if (keep_running && (max_iterations == 0 || iteration < max_iterations))
        {
            printf("\nSleeping for %d seconds...\n", interval);
            sleep(interval);
        }
    }

    printf("\n\nFinal memory usage:\n");
    print_memory_usage();
    
    // FIX: Print detailed memory summary
    print_memory_summary();

    printf("\nCleaning up connection pool...\n");
    http_pool_cleanup();
    printf("Cleanup complete\n");
    
    // FIX: Check memory after cleanup
    printf("\nMemory after cleanup:\n");
    print_memory_usage();

    printf("\nTest completed. Total iterations: %d\n", iteration);
    return 0;
}

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

// FIX: Add request type enumeration
typedef enum
{
    REQUEST_POST = 1,
    REQUEST_GET = 2,
    REQUEST_ALTERNATE = 3
} request_type_t;

// FIX: Add forward declaration for init_connection_pool
extern T2ERROR init_connection_pool(void);

// Configuration
#define TEST_URL "https://httpbin.org/post"
#define TEST_GET_URL "https://httpbin.org/get"
#define TEST_INTERVAL_SECONDS 5
#define MAX_ITERATIONS 100  // 0 for infinite loop

// FIX: Add structure to store iteration-wise memory statistics
typedef struct
{
    int iteration;
    long vmrss;
    long vmsize;
    long delta_vmrss;  // Change from previous iteration
} iteration_memory_t;

// FIX: Add memory tracking structure with iteration storage and pool init tracking
typedef struct
{
    long initial_vmrss;
    long initial_vmsize;
    long peak_vmrss;
    long peak_vmsize;
    long current_vmrss;
    long current_vmsize;
    long pool_init_vmrss;     // FIX: Add pool initialization VmRSS tracking
    long pool_init_vmsize;    // FIX: Add pool initialization VmSize tracking
    iteration_memory_t iterations[1000];  // Store up to 1000 iterations
    int iteration_count;
} memory_stats_t;

static memory_stats_t mem_stats = {0};

// Global flag for graceful shutdown
static volatile sig_atomic_t keep_running = 1;

void signal_handler(int sig)
{
    printf("\nReceived signal %d, shutting down gracefully...\n", sig);
    keep_running = 0;
}

// FIX: Enhanced memory tracking function that stores iteration data
void update_memory_stats(int iteration)
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
    {
        mem_stats.peak_vmrss = vmrss;
    }
    if (vmsize > mem_stats.peak_vmsize)
    {
        mem_stats.peak_vmsize = vmsize;
    }

    // Set initial if first call
    if (mem_stats.initial_vmrss == 0)
    {
        mem_stats.initial_vmrss = vmrss;
        mem_stats.initial_vmsize = vmsize;
        mem_stats.peak_vmrss = vmrss;
        mem_stats.peak_vmsize = vmsize;
    }

    // Store iteration data if we have an iteration number
    if (iteration > 0 && mem_stats.iteration_count < 1000)
    {
        iteration_memory_t *current_iter = &mem_stats.iterations[mem_stats.iteration_count];
        current_iter->iteration = iteration;
        current_iter->vmrss = vmrss;
        current_iter->vmsize = vmsize;

        if (mem_stats.iteration_count > 0)
        {
            iteration_memory_t *previous_iter = &mem_stats.iterations[mem_stats.iteration_count - 1];
            current_iter->delta_vmrss = vmrss - previous_iter->vmrss;
        }
        else
        {
            current_iter->delta_vmrss = 0;  // No delta for first iteration
        }

        mem_stats.iteration_count++;
    }
}

void print_memory_usage(void)
{
    update_memory_stats(0);  // Call without iteration tracking
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

// FIX: Add function to print memory delta for each iteration
void print_memory_delta(int iteration)
{
    if (iteration == 1)
    {
        // First iteration - no previous data to compare
        printf("Memory delta: N/A (first iteration)\n");
        return;
    }

    long vmrss_delta = mem_stats.iterations[iteration - 1].delta_vmrss;

    printf("Memory delta from previous iteration:\n");
    printf("  VmRSS:  %+8ld kB", vmrss_delta);
    if (vmrss_delta > 0)
    {
        printf(" (increased)");
    }
    else if (vmrss_delta < 0)
    {
        printf(" (decreased)");
    }
    else
    {
        printf(" (unchanged)");
    }
    printf("\n");

    // Highlight significant VmRSS increases
    if (vmrss_delta > 100)
    {
        printf("⚠️  WARNING: Significant VmRSS increase (%+ld kB) in this iteration!\n", vmrss_delta);
    }
}

// FIX: Add function to print iteration-wise VmRSS statistics table including pool init
void print_iteration_vmrss_table(void)
{
    if (mem_stats.iteration_count == 0)
    {
        printf("\nNo iteration data collected.\n");
        return;
    }

    printf("\n=== VmRSS Statistics Per Iteration ===\n");
    printf("┌─────────────┬─────────────┬─────────────┬─────────────┐\n");
    printf("│ Iteration   │ VmRSS (kB)  │ Delta (kB)  │ Status      │\n");
    printf("├─────────────┼─────────────┼─────────────┼─────────────┤\n");

    // FIX: Add pool initialization row first
    long pool_init_delta = mem_stats.pool_init_vmrss - mem_stats.initial_vmrss;
    printf("│ Pool Init   │ %11ld │ %+11ld │ %-11s │\n",
           mem_stats.pool_init_vmrss,
           pool_init_delta,
           "Pool Setup");

    for (int i = 0; i < mem_stats.iteration_count; i++)
    {
        iteration_memory_t *iter = &mem_stats.iterations[i];

        // Status based on delta
        const char* status;
        if (i == 0)
        {
            // FIX: For first iteration, compare against pool init instead of treating as baseline
            long first_iter_delta = iter->vmrss - mem_stats.pool_init_vmrss;
            iter->delta_vmrss = first_iter_delta;  // Update the stored delta

            if (first_iter_delta > 100)
            {
                status = "⚠️  High Inc";
            }
            else if (first_iter_delta > 10)
            {
                status = "↗️  Increase";
            }
            else if (first_iter_delta < -10)
            {
                status = "↘️  Decrease";
            }
            else
            {
                status = "✓ Stable";
            }
        }
        else if (iter->delta_vmrss > 100)
        {
            status = "⚠️  High Inc";
        }
        else if (iter->delta_vmrss > 10)
        {
            status = "↗️  Increase";
        }
        else if (iter->delta_vmrss < -10)
        {
            status = "↘️  Decrease";
        }
        else
        {
            status = "✓ Stable";
        }

        printf("│ %11d │ %11ld │ %+11ld │ %-11s │\n",
               iter->iteration,
               iter->vmrss,
               iter->delta_vmrss,
               status);
    }

    printf("└─────────────┴─────────────┴─────────────┴─────────────┘\n");

    // FIX: Calculate statistics including pool init
    long total_growth_from_init = mem_stats.iterations[mem_stats.iteration_count - 1].vmrss - mem_stats.initial_vmrss;
    long request_only_growth = mem_stats.iterations[mem_stats.iteration_count - 1].vmrss - mem_stats.pool_init_vmrss;
    long max_delta = 0, min_delta = 0;
    int positive_deltas = 0, negative_deltas = 0, zero_deltas = 0;

    for (int i = 1; i < mem_stats.iteration_count; i++)
    {
        long delta = mem_stats.iterations[i].delta_vmrss;
        if (delta > max_delta)
        {
            max_delta = delta;
        }
        if (delta < min_delta)
        {
            min_delta = delta;
        }
        if (delta > 0)
        {
            positive_deltas++;
        }
        else if (delta < 0)
        {
            negative_deltas++;
        }
        else
        {
            zero_deltas++;
        }
    }

    printf("\n=== VmRSS Analysis ===\n");
    printf("Pool initialization cost:   %+ld kB\n", pool_init_delta);
    printf("Total growth from start:    %+ld kB\n", total_growth_from_init);
    printf("Growth from requests only:  %+ld kB\n", request_only_growth);
    printf("Largest single increase:    %+ld kB\n", max_delta);
    printf("Largest single decrease:    %+ld kB\n", min_delta);
    printf("Iterations with increase:   %d\n", positive_deltas);
    printf("Iterations with decrease:   %d\n", negative_deltas);
    printf("Iterations stable:          %d\n", zero_deltas);

    // Memory leak assessment
    if (request_only_growth > 1000)
    {
        printf("\n🚨 MEMORY LEAK DETECTED: Request growth > 1MB\n");
    }
    else if (request_only_growth > 100)
    {
        printf("\n⚠️  POTENTIAL LEAK: Moderate request growth detected\n");
    }
    else if (positive_deltas > (mem_stats.iteration_count - 1) * 0.7)
    {
        printf("\n⚠️  POTENTIAL LEAK: Frequent memory increases\n");
    }
    else
    {
        printf("\n✅ MEMORY USAGE APPEARS STABLE\n");
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
    update_memory_stats(iteration);

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
    update_memory_stats(iteration);

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

// FIX: Update test function to accept request type
void run_test_request(int iteration, request_type_t request_type)
{
    switch (request_type)
    {
    case REQUEST_POST:
        test_http_post(iteration);
        break;
    case REQUEST_GET:
        test_http_get(iteration);
        break;
    case REQUEST_ALTERNATE:
        // Alternate between GET and POST
        if (iteration % 2 == 0)
        {
            test_http_post(iteration);
        }
        else
        {
            test_http_get(iteration);
        }
        break;
    default:
        printf("[Iteration %d] ERROR: Unknown request type\n", iteration);
        break;
    }
}

// FIX: Add help function
void print_usage(const char* program_name)
{
    printf("Usage: %s <iterations> <interval> <request_type>\n", program_name);
    printf("\nParameters:\n");
    printf("  iterations    - Number of iterations (0 for infinite)\n");
    printf("  interval      - Interval between requests in seconds\n");
    printf("  request_type  - Type of HTTP request:\n");
    printf("                  1 = POST only\n");
    printf("                  2 = GET only\n");
    printf("                  3 = Alternate between POST and GET\n");
    printf("\nExamples:\n");
    printf("  %s 10 2 1     # 10 POST requests, 2 second intervals\n", program_name);
    printf("  %s 20 5 2     # 20 GET requests, 5 second intervals\n", program_name);
    printf("  %s 15 3 3     # 15 requests alternating POST/GET, 3 second intervals\n", program_name);
    printf("  %s 0 10 3     # Infinite alternating requests, 10 second intervals\n", program_name);
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
    request_type_t request_type = REQUEST_ALTERNATE;

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

    if (argc > 3)
    {
        int type = atoi(argv[3]);
        if (type >= REQUEST_POST && type <= REQUEST_ALTERNATE)
        {
            request_type = (request_type_t)type;
        }
        else
        {
            printf("Invalid request type: %d\n", type);
            print_usage(argv[0]);
            return 1;
        }
    }

    if (argc < 2)
    {
        print_usage(argv[0]);
        return 1;
    }

    // Print request type information
    const char* request_type_str;
    switch (request_type)
    {
    case REQUEST_POST:
        request_type_str = "POST only";
        break;
    case REQUEST_GET:
        request_type_str = "GET only";
        break;
    case REQUEST_ALTERNATE:
        request_type_str = "Alternating POST/GET";
        break;
    default:
        request_type_str = "Unknown";
        break;
    }
    printf("Using request type: %s\n", request_type_str);

    // FIX: Set environment variable for pool size with error checking
    if (setenv("T2_CONNECTION_POOL_SIZE", "2", 1) != 0)
    {
        printf("Warning: Failed to set T2_CONNECTION_POOL_SIZE environment variable\n");
    }

    printf("\nInitial memory usage (before pool initialization):\n");
    print_memory_usage();

    // FIX: Explicitly initialize connection pool and print memory stats after
    printf("\nInitializing connection pool...\n");
    T2ERROR init_result = init_connection_pool();
    if (init_result != T2ERROR_SUCCESS)
    {
        printf("ERROR: Failed to initialize connection pool (error: %d)\n", init_result);
        return 1;
    }
    printf("Connection pool initialized successfully!\n");

    // FIX: Print memory statistics after pool initialization
    printf("\nMemory usage after pool initialization:\n");
    print_memory_usage();

    // FIX: Store pool initialization memory stats
    mem_stats.pool_init_vmrss = mem_stats.current_vmrss;
    mem_stats.pool_init_vmsize = mem_stats.current_vmsize;

    // Calculate memory used by pool initialization
    long pool_init_memory = mem_stats.current_vmrss - mem_stats.initial_vmrss;
    printf("Memory used by pool initialization: %+ld kB\n", pool_init_memory);

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
        run_test_request(iteration, request_type);

        // Print memory usage every iteration
        printf("\n--- Memory Usage at iteration %d ---\n", iteration);
        print_memory_usage();

        // Print memory delta for this iteration
        print_memory_delta(iteration);

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

    // FIX: Print iteration-wise VmRSS statistics table
    print_iteration_vmrss_table();

    printf("\nCleaning up connection pool...\n");
    http_pool_cleanup();
    printf("Cleanup complete\n");

    // FIX: Check memory after cleanup
    printf("\nMemory after cleanup:\n");
    print_memory_usage();

    printf("\nTest completed. Total iterations: %d\n", iteration);
    return 0;
}

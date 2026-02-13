CC = gcc
CFLAGS = -Wall -Wextra -g -O0 -fno-omit-frame-pointer

# FIX: Correct include paths - telemetry2_0.h is in ../../../include (relative to leak_detection dir)
CFLAGS += -I../../../include
CFLAGS += -I../../protocol/http
CFLAGS += -I../../commonlib
CFLAGS += -I../../busInterface
CFLAGS += -I../../xconf-client

# FIX: Add PIC flags and additional include paths for shared library compatibility
CFLAGS += -fPIC
CFLAGS += -I../../ccspinterface
CFLAGS += -I../../bulkdata
CFLAGS += -I../../utils

# Libraries
LDFLAGS = -lcurl -lpthread

# FIX: Add shared library flags
SHARED_LDFLAGS = -shared -fPIC -lcurl

# Source files
MULTI_CURL_SRC = ../../protocol/http/multicurlinterface.c
# FIX: Remove non-existent t2log_wrapper.c 
TEST_SRC = curl_leak_test.c

# Object files - remove t2log_wrapper.o since the file doesn't exist
OBJS = curl_leak_test.o multicurlinterface.o

# Conditional compilation
ifdef LIBRDKCERTSEL_BUILD
    CFLAGS += -DLIBRDKCERTSEL_BUILD
    # FIX: Add RdkCertSelector library support
    LDFLAGS += -lRdkCertSelector
    SHARED_LDFLAGS += -lRdkCertSelector
    OBJS += t2MtlsUtils.o
    MTLS_SRC = ../../xconf-client/t2MtlsUtils.c
endif

ifdef LIBRDKCONFIG_BUILD
    CFLAGS += -DLIBRDKCONFIG_BUILD
    LDFLAGS += -lrdkconfig
    SHARED_LDFLAGS += -lrdkconfig
endif

ifdef ENABLE_RDKB_SUPPORT
    CFLAGS += -DENABLE_RDKB_SUPPORT
    OBJS += busInterface.o
    LDFLAGS += -lrbus
    SHARED_LDFLAGS += -lrbus
    BUS_SRC = ../../busInterface/busInterface.c
    
    # FIX: Add dbus include paths for RDKB support
    CFLAGS += -I/usr/include/dbus-1.0
    CFLAGS += -I/usr/lib/x86_64-linux-gnu/dbus-1.0/include
    CFLAGS += -I/usr/include/ccsp
endif

TARGET = curl_leak_test
SHARED_LIB = libhttp_test.so

.PHONY: all clean run valgrind massif quick-leak help shared

all: $(TARGET)

# FIX: Add shared library target
shared: $(SHARED_LIB)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)
	@echo ""
	@echo "✓ Build successful: $(TARGET)"
	@echo "  Run with: ./$(TARGET) [iterations] [interval_seconds]"
	@echo ""

# FIX: Shared library build target
$(SHARED_LIB): $(OBJS)
	$(CC) $(SHARED_LDFLAGS) $(OBJS) -o $(SHARED_LIB)
	@echo ""
	@echo "✓ Shared library built: $(SHARED_LIB)"
	@echo ""

# Compile individual object files
curl_leak_test.o: $(TEST_SRC)
	$(CC) $(CFLAGS) -c $(TEST_SRC) -o $@

multicurlinterface.o: $(MULTI_CURL_SRC)
	$(CC) $(CFLAGS) -c $(MULTI_CURL_SRC) -o $@

# Optional object files
t2MtlsUtils.o: $(MTLS_SRC)
	$(CC) $(CFLAGS) -c $(MTLS_SRC) -o $@

busInterface.o: $(BUS_SRC)
	$(CC) $(CFLAGS) -c $(BUS_SRC) -o $@

clean:
	rm -f $(OBJS) $(TARGET) $(SHARED_LIB) *.log *.txt massif.out.* *.csv
	@echo "✓ Cleaned build artifacts"

# Run basic test
run: $(TARGET)
	@echo "Running basic leak test (20 iterations, 2 second intervals)..."
	./$(TARGET) 20 2

# Quick test (5 iterations)
quick: $(TARGET)
	@echo "Running quick test (5 iterations, 1 second intervals)..."
	./$(TARGET) 5 1

# Memory leak detection with Valgrind
valgrind: $(TARGET)
	@echo "Running Valgrind memory leak detection..."
	@echo "(This may take a while...)"
	valgrind \
		--leak-check=full \
		--show-leak-kinds=all \
		--track-origins=yes \
		--verbose \
		--log-file=valgrind_report.txt \
		./$(TARGET) 10 1
	@echo ""
	@echo "✓ Valgrind analysis complete!"
	@echo "  Report saved to: valgrind_report.txt"
	@echo ""
	@echo "=== LEAK SUMMARY ==="
	@grep -A 10 "LEAK SUMMARY" valgrind_report.txt || echo "No leak summary found"
	@echo ""
	@echo "=== ERROR SUMMARY ==="  
	@grep -A 5 "ERROR SUMMARY" valgrind_report.txt || echo "No error summary found"

# Memory profiling with Massif
massif: $(TARGET)
	@echo "Running Valgrind Massif memory profiler..."
	valgrind \
		--tool=massif \
		--massif-out-file=massif.out \
		--time-unit=ms \
		--detailed-freq=1 \
		./$(TARGET) 30 1
	@echo ""
	@echo "✓ Massif profiling complete!"
	@echo "  Raw data: massif.out"
	@echo "  Generating human-readable report..."
	ms_print massif.out > massif_report.txt
	@echo "  Report saved to: massif_report.txt"
	@echo ""
	@echo "=== PEAK MEMORY USAGE ==="
	@grep -A 5 -B 5 "peak" massif_report.txt | head -15

# Quick leak check
quick-leak: $(TARGET)
	@echo "Running quick Valgrind check (5 iterations)..."
	valgrind --leak-check=full --show-leak-kinds=all ./$(TARGET) 5 1

# Stress test
stress: $(TARGET)
	@echo "Running stress test (100 iterations, 1 second intervals)..."
	@echo "This will take about 2 minutes..."
	./$(TARGET) 100 1 2>&1 | tee stress_test.log
	@echo ""
	@echo "✓ Stress test complete! Log saved to: stress_test.log"

# Build variants
build-basic:
	make clean
	make

build-mtls:
	make clean  
	make LIBRDKCERTSEL_BUILD=1

build-rdkb:
	make clean
	make ENABLE_RDKB_SUPPORT=1

build-full:
	make clean
	make LIBRDKCERTSEL_BUILD=1 ENABLE_RDKB_SUPPORT=1 LIBRDKCONFIG_BUILD=1

# FIX: Add shared library build variants
build-shared:
	make clean
	make shared

build-shared-mtls:
	make clean
	make shared LIBRDKCERTSEL_BUILD=1

build-shared-rdkb:
	make clean
	make shared ENABLE_RDKB_SUPPORT=1

build-shared-full:
	make clean
	make shared LIBRDKCERTSEL_BUILD=1 ENABLE_RDKB_SUPPORT=1 LIBRDKCONFIG_BUILD=1

# Help
help:
	@echo "Curl Memory Leak Detection Test"
	@echo "================================"
	@echo ""
	@echo "Build targets:"
	@echo "  all          - Build basic version"
	@echo "  shared       - Build shared library version"
	@echo "  build-basic  - Build without optional features"
	@echo "  build-mtls   - Build with mTLS support"
	@echo "  build-rdkb   - Build with RDKB support"
	@echo "  build-full   - Build with all features"
	@echo "  build-shared - Build shared library (basic)"
	@echo "  build-shared-mtls   - Build shared library with mTLS"
	@echo "  build-shared-rdkb   - Build shared library with RDKB"
	@echo "  build-shared-full   - Build shared library with all features"
	@echo ""
	@echo "Test targets:"
	@echo "  run          - Run basic test (20 iterations, 2s interval)"
	@echo "  quick        - Run quick test (5 iterations, 1s interval)"
	@echo "  valgrind     - Run with memory leak detection"
	@echo "  massif       - Run with memory profiler"
	@echo "  quick-leak   - Quick Valgrind check"
	@echo "  stress       - Stress test (100 iterations)"
	@echo ""
	@echo "Other:"
	@echo "  clean        - Remove build artifacts"
	@echo "  help         - Show this help"
	@echo ""
	@echo "Examples:"
	@echo "  make run"
	@echo "  make build-mtls && make valgrind"
	@echo "  make shared"
	@echo "  make build-shared-full"
	@echo "  ./curl_leak_test 50 3"

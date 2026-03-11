CC = gcc
CFLAGS = -Wall -Wextra -g -O0
LDFLAGS = -lcurl -lpthread

TARGET = memory_verifier

.PHONY: all clean run help valgrind-investigate

all: $(TARGET)
	@echo ""
	@echo "✓ Memory Verifier built successfully: $(TARGET)"
	@echo "  Run with: make run"
	@echo ""

$(TARGET): memory_verifier.c
	$(CC) $(CFLAGS) -o $(TARGET) memory_verifier.c $(LDFLAGS)

run: $(TARGET)
	@echo "🔍 Running Memory Leak Verification..."
	@echo ""
	./$(TARGET)

valgrind-investigate: $(TARGET)
	@echo "🧪 Running SSL Memory Investigation..."
	@echo ""
	@if [ -f "investigate_ssl_memory.sh" ]; then \
		chmod +x investigate_ssl_memory.sh 2>/dev/null || true; \
		./investigate_ssl_memory.sh; \
	elif [ -f "investigate_ssl_memory.bat" ]; then \
		cmd //c investigate_ssl_memory.bat; \
	else \
		echo "❌ Investigation script not found"; \
		exit 1; \
	fi

clean:
	rm -f $(TARGET) 
	rm -f *.log *.out *.txt valgrind_*.sh
	@echo "✓ Cleaned memory verifier and investigation files"

help:
	@echo "Memory Leak Verifier with Valgrind Integration"
	@echo "=============================================="
	@echo ""
	@echo "Standalone tool to verify curl/OpenSSL has no memory leaks"
	@echo ""
	@echo "Commands:"
	@echo "  make                   - Build the verifier"
	@echo "  make run               - Build and run verification (RECOMMENDED)"
	@echo "  make valgrind-investigate - Run SSL memory investigation (NEW)"
	@echo "  make clean             - Remove build artifacts and logs"
	@echo "  make help              - Show this help"
	@echo ""
	@echo "Quick Start:"
	@echo "  make run               # Standard memory verification"
	@echo "  make valgrind-investigate # Investigate SSL memory increase"
	@echo ""
	@echo "Standard Tests:"
	@echo "  ✓ Basic HTTP client memory growth"
	@echo "  ✓ Handle reuse and connection pooling"
	@echo "  ✓ Memory stability over time"
	@echo "  ✓ Library version compatibility"
	@echo ""
	@echo "NEW: Valgrind Investigation (for SSL 3484KB→3608KB issue):"
	@echo "  ✓ Pool baseline analysis"
	@echo "  ✓ Reset fix impact analysis"
	@echo "  ✓ Comparative leak assessment"
	@echo "  ✓ SSL/OpenSSL deep investigation"
	@echo ""
	@echo "Individual Valgrind Tests:"
	@echo "  ./memory_verifier --valgrind-pool    # Baseline"
	@echo "  ./memory_verifier --valgrind-reset   # Reset fix"
	@echo "  ./memory_verifier --post-analysis    # Compare"

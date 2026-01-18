# Lab 3 - Operating Systems Synchronization
# HCMC University of Technology
# Root Makefile - Compile all problems

# Compiler settings
CC = gcc
CFLAGS = -pthread -Wall -std=gnu11
LDFLAGS = -pthread

# Color output
GREEN = \033[0;32m
YELLOW = \033[1;33m
BLUE = \033[0;34m
NC = \033[0m # No Color

# All problems
PROBLEMS = problem1_seqlock problem2_aggsum problem3_logbuf \
           problem4_convar problem5_detector problem6_resource_async \
           problem7_lockfree

.PHONY: all clean test help $(PROBLEMS)

all:
	@echo "$(BLUE)========================================$(NC)"
	@echo "$(BLUE)  Lab 3 - Synchronization Problems$(NC)"
	@echo "$(BLUE)========================================$(NC)"
	@$(MAKE) -C problem1_seqlock
	@$(MAKE) -C problem2_aggsum
	@$(MAKE) problem3_logbuf
	@$(MAKE) problem4_convar
	@$(MAKE) problem5_detector
	@$(MAKE) problem6_resource_async
	@$(MAKE) problem7_lockfree
	@echo "$(GREEN)✓ All problems compiled successfully!$(NC)"

# Problem 1: Sequence Lock (has its own Makefile)
problem1_seqlock:
	@echo "$(YELLOW)Building Problem 1: Sequence Lock...$(NC)"
	@$(MAKE) -C problem1_seqlock

# Problem 2: Aggregated Sum (has its own Makefile)
problem2_aggsum:
	@echo "$(YELLOW)Building Problem 2: Aggregated Sum...$(NC)"
	@$(MAKE) -C problem2_aggsum

# Problem 3: Logger Buffer
problem3_logbuf:
	@echo "$(YELLOW)Building Problem 3: Logger Buffer...$(NC)"
	@$(CC) $(CFLAGS) -o problem3_logbuf/logbuf problem3_logbuf/logbuf.c $(LDFLAGS)

# Problem 4: Condition Variables
problem4_convar:
	@echo "$(YELLOW)Building Problem 4: Condition Variables...$(NC)"
	@$(CC) $(CFLAGS) -o problem4_convar/condvar problem4_convar/condvar.c $(LDFLAGS)

# Problem 5: Periodic Detector
problem5_detector:
	@echo "$(YELLOW)Building Problem 5: Periodic Detector...$(NC)"
	@$(CC) $(CFLAGS) -o problem5_detector/detector problem5_detector/detector.c $(LDFLAGS)

# Problem 6: Async Resource Allocation
problem6_resource_async:
	@echo "$(YELLOW)Building Problem 6: Async Resource...$(NC)"
	@$(CC) $(CFLAGS) -o problem6_resource_async/resource_async \
		problem6_resource_async/resource_async.c $(LDFLAGS)

# Problem 7: Lock-Free Stack
problem7_lockfree:
	@echo "$(YELLOW)Building Problem 7: Lock-Free Stack...$(NC)"
	@$(CC) $(CFLAGS) -o problem7_lockfree/lockfree problem7_lockfree/lockfree.c $(LDFLAGS)

# Clean all
clean:
	@echo "$(YELLOW)Cleaning all problems...$(NC)"
	@$(MAKE) -C problem1_seqlock clean 2>/dev/null || true
	@$(MAKE) -C problem2_aggsum clean 2>/dev/null || true
	@rm -f problem3_logbuf/logbuf
	@rm -f problem4_convar/condvar
	@rm -f problem5_detector/detector
	@rm -f problem6_resource_async/resource_async
	@rm -f problem7_lockfree/lockfree
	@echo "$(GREEN)✓ Cleaned successfully!$(NC)"

# Test all problems
test: all
	@echo "$(BLUE)========================================$(NC)"
	@echo "$(BLUE)  Running All Tests$(NC)"
	@echo "$(BLUE)========================================$(NC)"
	@echo "\n$(YELLOW)=== Problem 1: Sequence Lock ===$(NC)"
	@echo "  [Test 1] Basic seqlock test:"
	@cd problem1_seqlock && ./seqlock || true
	@echo "\n  [Test 2] Multi-thread test:"
	@cd problem1_seqlock && ./test_multithread || true
	@echo "\n$(YELLOW)=== Problem 2: Aggregated Sum ===$(NC)"
	@cd problem2_aggsum && ./aggsum 150 8 1024 || true
	@echo "\n$(YELLOW)=== Problem 3: Logger Buffer ===$(NC)"
	@cd problem3_logbuf && timeout 3s ./logbuf || true
	@echo "\n$(YELLOW)=== Problem 4: Condition Variables ===$(NC)"
	@cd problem4_convar && timeout 5s ./condvar || true
	@echo "\n$(YELLOW)=== Problem 5: Periodic Detector ===$(NC)"
	@cd problem5_detector && timeout 5s ./detector || true
	@echo "\n$(YELLOW)=== Problem 6: Async Resource ===$(NC)"
	@cd problem6_resource_async && ./resource_async || true
	@echo "\n$(YELLOW)=== Problem 7: Lock-Free Stack ===$(NC)"
	@cd problem7_lockfree && ./lockfree || true
	@echo "\n$(GREEN)✓ All tests completed!$(NC)"

# Help menu
help:
	@echo "Lab 3 - Synchronization Problems"
	@echo "=================================="
	@echo "Available targets:"
	@echo "  make                  - Build all problems"
	@echo "  make clean            - Clean all compiled files"
	@echo "  make test             - Build and run all tests"
	@echo "  make help             - Show this help menu"
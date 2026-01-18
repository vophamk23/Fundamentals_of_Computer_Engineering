# Makefile for Petri Net Analyzer Project (Task 1 + Task 2 + Task 3 + Task 4 + Task 5)

# Compiler settings
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Wno-unused-parameter -Wno-unused-variable -g -Iinclude -Isrc -Icudd/cudd -Icudd/include -Icudd/util -Icudd/mtr -Icudd/st
INCLUDES = -Iinclude -Isrc
LDFLAGS = -Lcudd/cudd/.libs -lcudd

# Directories
SRC_DIR = src
CORE_DIR = $(SRC_DIR)/core
PARSER_DIR = $(SRC_DIR)/parser
EXPLICIT_DIR = $(SRC_DIR)/explicit
SYMBOLIC_DIR = $(SRC_DIR)/symbolic
ILP_DIR = $(SRC_DIR)/ILP
TEST_DIR = $(SRC_DIR)/tests
INCLUDE_DIR = include
OUTPUT_DIR = output

# Source files
CORE_SRC = $(CORE_DIR)/PetriNet.cpp
PARSER_SRC = $(PARSER_DIR)/PNMLParser.cpp
EXPLICIT_SRC = $(EXPLICIT_DIR)/ExplicitReachability.cpp
SYMBOLIC_SRC = $(SYMBOLIC_DIR)/BDDReachability.cpp
ILP_DEADLOCK_SRC = $(ILP_DIR)/DeadlockDetector.cpp
ILP_OPT_SRC = $(ILP_DIR)/OptimizationSolver.cpp
XML_SRC = $(INCLUDE_DIR)/tinyxml2.cpp

# Object files
CORE_OBJ = $(CORE_SRC:.cpp=.o)
PARSER_OBJ = $(PARSER_SRC:.cpp=.o)
EXPLICIT_OBJ = $(EXPLICIT_SRC:.cpp=.o)
SYMBOLIC_OBJ = $(SYMBOLIC_SRC:.cpp=.o)
ILP_DEADLOCK_OBJ = $(ILP_DEADLOCK_SRC:.cpp=.o)
ILP_OPT_OBJ = $(ILP_OPT_SRC:.cpp=.o)
XML_OBJ = $(XML_SRC:.cpp=.o)

# Common objects
COMMON_OBJ = $(CORE_OBJ) $(PARSER_OBJ) $(XML_OBJ)
TASK2_OBJ = $(COMMON_OBJ) $(EXPLICIT_OBJ)
TASK3_OBJ = $(COMMON_OBJ) $(SYMBOLIC_OBJ)
TASK4_OBJ = $(COMMON_OBJ) $(SYMBOLIC_OBJ) $(ILP_DEADLOCK_OBJ)
TASK5_OBJ = $(COMMON_OBJ) $(SYMBOLIC_OBJ) $(ILP_OPT_OBJ)

# Test executables
TASK1_EXE = task1_test
BFS_EXE = bfs_test
DFS_EXE = dfs_test
BDD_EXE = bdd_test
DEADLOCK_EXE = deadlock_test
OPT_EXE = optimization_test

# Phony targets
.PHONY: all help clean build_all
.PHONY: task1 task2 task3 task4 task5
.PHONY: test test1 test2 test3 test4 test5 testv test1v test2v test3v test4v test5v
.PHONY: test1_1 test1_2 test1_3 test1_4 test1_5 test1_6
.PHONY: test1_1v test1_2v test1_3v test1_4v test1_5v test1_6v
.PHONY: test2_bfs test2_dfs test2_bfs_v test2_dfs_v
.PHONY: test3_bdd test3_bdd_v
.PHONY: test4_deadlock test4_deadlock_v
.PHONY: test5_opt test5_opt_v

# ============================================
#           DEFAULT TARGET
# ============================================
all: help

# ============================================
#           BUILD COMMANDS
# ============================================

# Build Task 1
task1: $(TASK1_EXE)
	@echo ""
	@echo "✓ Task 1 test executable built successfully!"
	@echo "  - ./$(TASK1_EXE)"
	@echo "Run 'make test1' to run all Task 1 tests"

$(TASK1_EXE): $(TEST_DIR)/test_petriNet.cpp $(COMMON_OBJ)
	@echo "🔨 Building $(TASK1_EXE)..."
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^

# Build Task 2
task2: $(BFS_EXE) $(DFS_EXE)
	@echo ""
	@echo "✓ Task 2 test executables built successfully!"
	@echo "  - ./$(BFS_EXE)"
	@echo "  - ./$(DFS_EXE)"
	@echo "Run 'make test2' to run all Task 2 tests"

$(BFS_EXE): $(TEST_DIR)/test_BFS.cpp $(TASK2_OBJ)
	@echo "🔨 Building $(BFS_EXE)..."
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^

$(DFS_EXE): $(TEST_DIR)/test_DFS.cpp $(TASK2_OBJ)
	@echo "🔨 Building $(DFS_EXE)..."
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^

# Build Task 3
task3: $(BDD_EXE)
	@echo ""
	@echo "✓ Task 3 test executable built successfully!"
	@echo "  - ./$(BDD_EXE)"
	@echo "Run 'make test3' to run all Task 3 tests"

$(BDD_EXE): $(TEST_DIR)/test_BDD.cpp $(TASK3_OBJ)
	@echo "🔨 Building $(BDD_EXE)..."
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^ $(LDFLAGS)

# Build Task 4
task4: $(DEADLOCK_EXE)
	@echo ""
	@echo "✓ Task 4 test executable built successfully!"
	@echo "  - ./$(DEADLOCK_EXE)"
	@echo "Run 'make test4' to run all Task 4 tests"
	@echo ""
	@echo "⚠️  Note: Requires ILP solver (CBC or GLPK)"
	@echo "  Install: sudo apt-get install coinor-cbc"
	@echo "  Or:      sudo apt-get install glpk-utils"

$(DEADLOCK_EXE): $(TEST_DIR)/test_Deadlock.cpp $(TASK4_OBJ)
	@echo "🔨 Building $(DEADLOCK_EXE)..."
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^ $(LDFLAGS)

# Build Task 5 - NEW!
task5: $(OPT_EXE)
	@echo ""
	@echo "✓ Task 5 test executable built successfully!"
	@echo "  - ./$(OPT_EXE)"
	@echo "Run 'make test5' to run all Task 5 tests"

$(OPT_EXE): $(TEST_DIR)/test_Optimization.cpp $(TASK5_OBJ)
	@echo "🔨 Building $(OPT_EXE)..."
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^ $(LDFLAGS)

# Build all tasks
build_all: task1 task2 task3 task4 task5
	@echo ""
	@echo "========================================="
	@echo "✓ All tasks built successfully!"
	@echo "========================================="

# Compile object files
%.o: %.cpp
	@echo "📝 Compiling $<..."
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Special rule for symbolic objects (needs CUDD)
$(SYMBOLIC_OBJ): $(SYMBOLIC_SRC)
	@echo "📝 Compiling $< (with CUDD)..."
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Special rule for ILP deadlock detector
$(ILP_DEADLOCK_OBJ): $(ILP_DEADLOCK_SRC)
	@echo "📝 Compiling $< (ILP Deadlock Detector)..."
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Special rule for ILP optimization solver - NEW!
$(ILP_OPT_OBJ): $(ILP_OPT_SRC)
	@echo "📝 Compiling $< (ILP Optimization Solver)..."
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# ============================================
#           TEST COMMANDS - ALL
# ============================================

# Run all tests (Task 1 + Task 2 + Task 3 + Task 4 + Task 5) - COMPACT
test: test1 test2 test3 test4 test5
	@echo ""
	@echo "========================================="
	@echo "✓ All tests completed!"
	@echo "========================================="

# Run all tests (Task 1 + Task 2 + Task 3 + Task 4 + Task 5) - VERBOSE ⭐ NEW!
testv: test1v test2v test3v test4v test5v
	@echo ""
	@echo "========================================="
	@echo "✓ All tests completed (Verbose)!"
	@echo "========================================="
	@echo "📁 Output files saved to:"
	@echo "  - $(OUTPUT_DIR)/test_task1.txt"
	@echo "  - $(OUTPUT_DIR)/test_bfs.txt"
	@echo "  - $(OUTPUT_DIR)/test_dfs.txt"
	@echo "  - $(OUTPUT_DIR)/test_task3.txt"
	@echo "  - $(OUTPUT_DIR)/test_deadlock.txt"
	@echo "  - $(OUTPUT_DIR)/test_optimization.txt"
	@echo "========================================="

# ============================================
#           TASK 1 TESTS
# ============================================

# Run all Task 1 tests - COMPACT
test1: $(TASK1_EXE)
	@echo ""
	@echo "========================================="
	@echo "   Running Task 1 Tests (Compact)"
	@echo "========================================="
	@./$(TASK1_EXE)
	@echo ""

# Run all Task 1 tests - VERBOSE
test1v: $(TASK1_EXE)
	@mkdir -p $(OUTPUT_DIR)
	@echo ""
	@echo "========================================="
	@echo "   Running Task 1 Tests (Verbose)"
	@echo "========================================="
	@./$(TASK1_EXE) -v | tee $(OUTPUT_DIR)/test_task1.txt
	@echo ""
	@echo "Output saved to: $(OUTPUT_DIR)/test_task1.txt"

# Run specific Task 1 test - COMPACT
test1_1: $(TASK1_EXE)
	@echo "🚀 Running Task 1 - Test 1..."
	@./$(TASK1_EXE) 1

test1_2: $(TASK1_EXE)
	@echo "🚀 Running Task 1 - Test 2..."
	@./$(TASK1_EXE) 2

test1_3: $(TASK1_EXE)
	@echo "🚀 Running Task 1 - Test 3..."
	@./$(TASK1_EXE) 3

test1_4: $(TASK1_EXE)
	@echo "🚀 Running Task 1 - Test 4..."
	@./$(TASK1_EXE) 4

test1_5: $(TASK1_EXE)
	@echo "🚀 Running Task 1 - Test 5..."
	@./$(TASK1_EXE) 5

test1_6: $(TASK1_EXE)
	@echo "🚀 Running Task 1 - Test 6..."
	@./$(TASK1_EXE) 6

# Run specific Task 1 test - VERBOSE
test1_1v: $(TASK1_EXE)
	@echo "🚀 Running Task 1 - Test 1 (VERBOSE)..."
	@./$(TASK1_EXE) 1 -v

test1_2v: $(TASK1_EXE)
	@echo "🚀 Running Task 1 - Test 2 (VERBOSE)..."
	@./$(TASK1_EXE) 2 -v

test1_3v: $(TASK1_EXE)
	@echo "🚀 Running Task 1 - Test 3 (VERBOSE)..."
	@./$(TASK1_EXE) 3 -v

test1_4v: $(TASK1_EXE)
	@echo "🚀 Running Task 1 - Test 4 (VERBOSE)..."
	@./$(TASK1_EXE) 4 -v

test1_5v: $(TASK1_EXE)
	@echo "🚀 Running Task 1 - Test 5 (VERBOSE)..."
	@./$(TASK1_EXE) 5 -v

test1_6v: $(TASK1_EXE)
	@echo "🚀 Running Task 1 - Test 6 (VERBOSE)..."
	@./$(TASK1_EXE) 6 -v

# ============================================
#           TASK 2 TESTS
# ============================================

# Run all Task 2 tests - COMPACT
test2: test2_bfs test2_dfs

# Run all Task 2 tests - VERBOSE
test2v: test2_bfs_v test2_dfs_v
	@echo ""
	@echo "========================================="
	@echo "✓ All Task 2 tests completed (Verbose)!"
	@echo "========================================="
	@echo "Output files saved to:"
	@echo "  - $(OUTPUT_DIR)/test_bfs.txt"
	@echo "  - $(OUTPUT_DIR)/test_dfs.txt"
	@echo "========================================="

# Run BFS test - COMPACT
test2_bfs: $(BFS_EXE)
	@mkdir -p $(OUTPUT_DIR)
	@echo ""
	@echo "========================================="
	@echo "   Running BFS Tests (Compact)"
	@echo "========================================="
	@./$(BFS_EXE) --compact
	@echo ""

# Run BFS test - VERBOSE
test2_bfs_v: $(BFS_EXE)
	@mkdir -p $(OUTPUT_DIR)
	@echo ""
	@echo "========================================="
	@echo "   Running BFS Tests (Verbose)"
	@echo "========================================="
	@./$(BFS_EXE) | tee $(OUTPUT_DIR)/test_bfs.txt
	@echo ""
	@echo "Output saved to: $(OUTPUT_DIR)/test_bfs.txt"

# Run DFS test - COMPACT
test2_dfs: $(DFS_EXE)
	@mkdir -p $(OUTPUT_DIR)
	@echo ""
	@echo "========================================="
	@echo "   Running DFS Tests (Compact)"
	@echo "========================================="
	@./$(DFS_EXE) --compact
	@echo ""

# Run DFS test - VERBOSE
test2_dfs_v: $(DFS_EXE)
	@mkdir -p $(OUTPUT_DIR)
	@echo ""
	@echo "========================================="
	@echo "   Running DFS Tests (Verbose)"
	@echo "========================================="
	@./$(DFS_EXE) | tee $(OUTPUT_DIR)/test_dfs.txt
	@echo ""
	@echo "Output saved to: $(OUTPUT_DIR)/test_dfs.txt"

# ============================================
#           TASK 3 TESTS
# ============================================

# Run all Task 3 tests - COMPACT
test3: $(BDD_EXE)
	@mkdir -p $(OUTPUT_DIR)
	@echo ""
	@echo "========================================="
	@echo "   Running Task 3 Tests (Compact)"
	@echo "========================================="
	@./$(BDD_EXE) --compact
	@echo ""

# Run all Task 3 tests - VERBOSE
test3v: $(BDD_EXE)
	@mkdir -p $(OUTPUT_DIR)
	@echo ""
	@echo "========================================="
	@echo "   Running Task 3 Tests (Verbose)"
	@echo "========================================="
	@./$(BDD_EXE) | tee $(OUTPUT_DIR)/test_task3.txt
	@echo ""
	@echo "Output saved to: $(OUTPUT_DIR)/test_task3.txt"
	@echo "========================================="

# ============================================
#           TASK 4 TESTS 
# ============================================

# Run all Task 4 tests - COMPACT
test4: $(DEADLOCK_EXE)
	@mkdir -p $(OUTPUT_DIR)
	@echo ""
	@echo "========================================="
	@echo "   Running Task 4 Tests (Compact)"
	@echo "========================================="
	@./$(DEADLOCK_EXE) --compact
	@echo ""

# Run all Task 4 tests - VERBOSE
test4v: $(DEADLOCK_EXE)
	@mkdir -p $(OUTPUT_DIR)
	@echo ""
	@echo "========================================="
	@echo "   Running Task 4 Tests (Verbose)"
	@echo "========================================="
	@./$(DEADLOCK_EXE) | tee $(OUTPUT_DIR)/test_deadlock.txt
	@echo ""
	@echo "Output saved to: $(OUTPUT_DIR)/test_deadlock.txt"
	@echo "========================================="

# ============================================
#           TASK 5 TESTS - NEW!
# ============================================

# Run all Task 5 tests - COMPACT
test5: $(OPT_EXE)
	@mkdir -p $(OUTPUT_DIR)
	@echo ""
	@echo "========================================="
	@echo "   Running Task 5 Tests (Compact)"
	@echo "========================================="
	@./$(OPT_EXE) --compact
	@echo ""

# Run all Task 5 tests - VERBOSE
test5v: $(OPT_EXE)
	@mkdir -p $(OUTPUT_DIR)
	@echo ""
	@echo "========================================="
	@echo "   Running Task 5 Tests (Verbose)"
	@echo "========================================="
	@./$(OPT_EXE) | tee $(OUTPUT_DIR)/test_optimization.txt
	@echo ""
	@echo "Output saved to: $(OUTPUT_DIR)/test_optimization.txt"
	@echo "========================================="

# Check if ILP solver is available
check-solver:
	@echo "🔍 Checking for ILP solvers..."
	@which cbc > /dev/null 2>&1 && echo "  ✓ CBC found at: $$(which cbc)" || echo "  ✗ CBC not found"
	@which glpsol > /dev/null 2>&1 && echo "  ✓ GLPK found at: $$(which glpsol)" || echo "  ✗ GLPK not found"
	@echo ""
	@which cbc > /dev/null 2>&1 || which glpsol > /dev/null 2>&1 || \
		(echo "⚠️  No ILP solver found! Install one of:"; \
		 echo "  sudo apt-get install coinor-cbc"; \
		 echo "  sudo apt-get install glpk-utils")

# ============================================
#           UTILITY COMMANDS
# ============================================

# Clean build files
clean:
	@echo "🧹 Cleaning build files..."
	@rm -f $(CORE_OBJ) $(PARSER_OBJ) $(EXPLICIT_OBJ) $(SYMBOLIC_OBJ)
	@rm -f $(ILP_DEADLOCK_OBJ) $(ILP_OPT_OBJ) $(XML_OBJ)
	@rm -f $(TEST_DIR)/*.o
	@rm -f $(TASK1_EXE) $(BFS_EXE) $(DFS_EXE) $(BDD_EXE) $(DEADLOCK_EXE) $(OPT_EXE)
	@rm -f *.lp *.sol
	@rm -rf $(OUTPUT_DIR)
	@echo "✓ All build files cleaned"

# Show help
help:
	@echo "=========================================="
	@echo "   Petri Net Analyzer - Unified Build"
	@echo "=========================================="
	@echo ""
	@echo "📦 BUILD COMMANDS:"
	@echo "  make task1        - Build Task 1 test executable"
	@echo "  make task2        - Build Task 2 test executables"
	@echo "  make task3        - Build Task 3 BDD test executable"
	@echo "  make task4        - Build Task 4 ILP Deadlock test"
	@echo "  make task5        - Build Task 5 Optimization test"
	@echo "  make build_all    - Build all tasks"
	@echo ""
	@echo "🧪 TEST COMMANDS (ALL):"
	@echo "  make test         - Run all tests (compact)"
	@echo "  make testv        - Run all tests (verbose)"
	@echo "  make check-solver - Check ILP solver availability"
	@echo ""
	@echo "🧪 TASK 1 TESTS (test_petriNet.cpp):"
	@echo "  make test1        - Run all Task 1 tests (compact)"
	@echo "  make test1v       - Run all Task 1 tests (verbose)"
	@echo "  make test1_N      - Run specific test N (1-6)"
	@echo "  make test1_Nv     - Run specific test N (verbose)"
	@echo ""
	@echo "  Examples:"
	@echo "    make test1_1    - Run test 1 (compact)"
	@echo "    make test1_2v   - Run test 2 (verbose)"
	@echo ""
	@echo "🧪 TASK 2 TESTS (test_BFS.cpp & test_DFS.cpp):"
	@echo "  make test2        - Run all Task 2 tests (compact)"
	@echo "  make test2v       - Run all Task 2 tests (verbose)"
	@echo "  make test2_bfs    - Run BFS test (compact)"
	@echo "  make test2_bfs_v  - Run BFS test (verbose)"
	@echo "  make test2_dfs    - Run DFS test (compact)"
	@echo "  make test2_dfs_v  - Run DFS test (verbose)"
	@echo ""
	@echo "🧪 TASK 3 TESTS (test_BDD.cpp):"
	@echo "  make test3        - Run all Task 3 tests (compact)"
	@echo "  make test3v       - Run all Task 3 tests (verbose)"
	@echo ""
	@echo "🧪 TASK 4 TESTS (test_Deadlock.cpp):"
	@echo "  make test4        - Run all Task 4 tests (compact)"
	@echo "  make test4v       - Run all Task 4 tests (verbose)"
	@echo ""
	@echo "🧪 TASK 5 TESTS (test_Optimization.cpp):"
	@echo "  make test5        - Run all Task 5 tests (compact)"
	@echo "  make test5v       - Run all Task 5 tests (verbose)"
	@echo ""
	@echo "🧹 UTILITY:"
	@echo "  make clean        - Remove all build files"
	@echo "  make help         - Show this help message"
	@echo ""
	@echo "=========================================="
	@echo "⚡ QUICK START:"
	@echo "  1. make build_all  (build everything)"
	@echo "  2. make test       (run all tests compact)"
	@echo "  3. make testv      (run all tests verbose)"
	@echo "  Or run individual:"
	@echo "  4. make test1v     (Task 1 verbose)"
	@echo "  5. make test2v     (Task 2 verbose)"
	@echo "  6. make test3v     (Task 3 verbose)"
	@echo "  7. make test4v     (Task 4 verbose)"
	@echo "  8. make test5v     (Task 5 verbose)"
	@echo "=========================================="
	@echo ""
	@echo "📝 OUTPUT FILES:"
	@echo "  Task 1: output/test_task1.txt"
	@echo "  Task 2: output/test_bfs.txt"
	@echo "          output/test_dfs.txt"
	@echo "  Task 3: output/test_task3.txt"
	@echo "  Task 4: output/test_deadlock.txt"
	@echo "  Task 5: output/test_optimization.txt"
	@echo "=========================================="
	@echo ""
	@echo "📋 TEST FILES MAPPING:"
	@echo "  Task 1: src/tests/test_petriNet.cpp → task1_test"
	@echo "  Task 2: src/tests/test_BFS.cpp → bfs_test"
	@echo "          src/tests/test_DFS.cpp → dfs_test"
	@echo "  Task 3: src/tests/test_BDD.cpp → bdd_test"
	@echo "  Task 4: src/tests/test_Deadlock.cpp → deadlock_test"
	@echo "  Task 5: src/tests/test_Optimization.cpp → optimization_test"
	@echo "=========================================="
	@echo ""
	@echo "⚠️  TASK 4 & 5 REQUIREMENTS:"
	@echo "  - CUDD library (for BDD)"
	@echo "  - ILP Solver: CBC or GLPK (optional for Task 5)"
	@echo "  Run 'make check-solver' to verify"
	@echo "=========================================="
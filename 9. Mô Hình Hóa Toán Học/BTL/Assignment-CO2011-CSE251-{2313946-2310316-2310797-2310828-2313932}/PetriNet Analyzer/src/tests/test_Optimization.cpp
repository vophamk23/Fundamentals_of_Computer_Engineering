#include "../core/PetriNet.h"
#include "../parser/PNMLParser.h"
#include "../symbolic/BDDReachability.h"
#include "../ILP/OptimizationSolver.h"
#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <cstring>
#include <algorithm>
#include <map>

using namespace std;

// Mode: verbose (chi tiết) hoặc compact (gọn)
bool VERBOSE_MODE = true;

// ANSI color codes
#define RESET "\033[0m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN "\033[36m"
#define BOLD "\033[1m"

// ============================================================================
// Struct cho Test Case Data
// ============================================================================
struct OptimizationTestCase
{
    string filename;
    string description;
    map<string, int> weights;
    bool should_succeed; // Expected: có tìm được marking tối ưu không
};

// ============================================================================
// Helper: Lấy danh sách Test Cases
// ============================================================================
vector<OptimizationTestCase> get_test_cases()
{
    return {
        // Test 1: Simple linear cycle với uniform weights
        {"tests/test_1/example.pnml",
         "Simple cycle - Uniform weights",
         {{"p1", 2}, {"p2", 3}, {"p3", 1}},
         true},

        // Test 2: Simple linear cycle với non-uniform weights
        {"tests/test_1/example.pnml",
         "Simple cycle - High weight on p1",
         {{"p1", 10}, {"p2", 5}, {"p3", 1}},
         true},

        // Test 3: Fork-join với equal weights
        {"tests/test_2/example.pnml",
         "Fork-join - Equal weights",
         {{"p1", 1}, {"p2", 1}, {"p3", 1}, {"p4", 1}},
         true},

        // Test 4: Fork-join với mixed weights
        {"tests/test_2/example.pnml",
         "Fork-join - Mixed pos/neg weights",
         {{"p1", 5}, {"p2", -2}, {"p3", 3}, {"p4", 1}},
         true},

        // Test 5: Join-merge pattern (có thể có ít reachable markings)
        {"tests/test_3/example.pnml",
         "Join-merge - Minimal reachability",
         {{"p1", 1}, {"p2", 1}, {"p3", 1}, {"p4", 1}},
         true},

        // Test 6: Dining Philosophers với negative weights
        {"tests/test_6/example.pnml",
         "Dining Philosophers - Negative weights",
         {{"p1", -1}, {"p2", -1}, {"p3", -1}, {"p4", -1}, {"p5", -1}, {"p6", -1}},
         true}};
}

// ============================================================================
// Định dạng Output - Compact Mode
// ============================================================================
void print_compact_result(const char *test_name, bool passed, int test_num, int total)
{
    int percentage = (test_num * 100) / total;

    cout << "test_Optimization.cpp::test_"
         << setfill('0') << setw(3) << test_num
         << setfill(' ')
         << " ";

    if (passed)
    {
        cout << GREEN << "PASSED" << RESET;
    }
    else
    {
        cout << RED << "FAILED" << RESET;
    }

    // Progress percentage bên phải với màu vàng
    cout << right << setw(70) << "[ " << YELLOW << setw(3) << percentage << "%" << RESET << "]" << endl;
}

// ============================================================================
// Định dạng Output - Verbose Mode
// ============================================================================
void print_verbose_result(const char *test_name,
                          const OptimizationTestCase &test_case,
                          const OptimizationResult &max_result,
                          const OptimizationResult &min_result,
                          bool passed)
{
    cout << "\n"
         << BOLD << "========================================" << RESET << endl;
    cout << BOLD << "TEST: " << test_name << " (" << test_case.description << ")" << RESET << endl;
    cout << BOLD << "========================================" << RESET << endl;

    cout << "\n  Objective weights: ";
    for (const auto &[pid, weight] : test_case.weights)
    {
        if (weight != 0)
            cout << pid << ":" << weight << " ";
    }
    cout << endl;

    cout << "\n"
         << CYAN << "  MAXIMIZATION:" << RESET << endl;
    max_result.print();

    cout << "\n"
         << CYAN << "  MINIMIZATION:" << RESET << endl;
    min_result.print();

    if (passed)
    {
        cout << "\n"
             << GREEN << "  ✓ PASSED" << RESET << endl;
    }
    else
    {
        cout << "\n"
             << RED << "  ✗ FAILED" << RESET << endl;
        cout << "    Reason: Optimization failed or produced invalid results" << endl;
    }
    cout << endl;
}

// ============================================================================
// TEST RUNNER
// ============================================================================
bool run_test(const OptimizationTestCase &test_case, int test_num, int total)
{
    string test_name_str = "test_" + string(3 - std::min(3, (int)to_string(test_num).length()), '0') + to_string(test_num);
    const char *test_name = test_name_str.c_str();

    OptimizationResult max_result = {};
    OptimizationResult min_result = {};
    bool passed = false;

    try
    {
        // 1. Parse PNML file
        PetriNet petri_net = PNMLParser::parse(test_case.filename);

        // 2. Compute BDD reachability (Task 3)
        BDDReachability bdd_reach(&petri_net);
        bdd_reach.compute_reachable_set();

        // 3. Setup optimization solver (Task 5)
        OptimizationSolver solver(&petri_net, &bdd_reach);
        solver.set_objective_weights(test_case.weights);

        // 4. Run maximization
        max_result = solver.maximize();

        // 5. Run minimization
        min_result = solver.minimize();

        // 6. Verify results
        // Kiểm tra xem có tìm được kết quả không và kết quả có hợp lệ không
        bool max_valid = max_result.solution_found;
        bool min_valid = min_result.solution_found;

        // Kiểm tra tính logic: max_value >= min_value
        bool logic_valid = true;
        if (max_valid && min_valid)
        {
            logic_valid = (max_result.optimal_value >= min_result.optimal_value);
        }

        passed = max_valid && min_valid && logic_valid && test_case.should_succeed;
    }
    catch (const exception &e)
    {
        max_result.solution_found = false;
        min_result.solution_found = false;
        max_result.solver_status = "Exception";
        min_result.solver_status = "Exception";
        max_result.optimization_detail = string("Exception: ") + e.what();
        min_result.optimization_detail = string("Exception: ") + e.what();
        passed = false;
    }

    if (VERBOSE_MODE)
    {
        print_verbose_result(test_name, test_case, max_result, min_result, passed);
    }
    else
    {
        print_compact_result(test_name, passed, test_num, total);
    }

    return passed;
}

// ============================================================================
// MAIN
// ============================================================================
int main(int argc, char *argv[])
{
    // Parse command line arguments
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0)
        {
            VERBOSE_MODE = true;
        }
        else if (strcmp(argv[i], "--compact") == 0)
        {
            VERBOSE_MODE = false;
        }
    }

    vector<OptimizationTestCase> test_cases = get_test_cases();
    int total = test_cases.size();

    if (!VERBOSE_MODE)
    {
        cout << BOLD << "collected " << total << " items\n\n"
             << RESET;
    }
    else
    {
        cout << "\n";
        cout << BOLD MAGENTA << "========================================" << endl;
        cout << "  ILP Optimization Test Suite (Task 5)" << endl;
        cout << "========================================" << RESET << endl;
        cout << endl;
    }

    int passed = 0;

    // Run tests
    for (size_t i = 0; i < test_cases.size(); ++i)
    {
        if (run_test(test_cases[i], i + 1, total))
            passed++;
    }

    // Print summary (pytest-style)
    if (!VERBOSE_MODE)
    {
        cout << "\n";

        // Dòng = trên màu CYAN in đậm
        cout << CYAN << BOLD;
        for (int i = 0; i < 40; i++)
            cout << "=";
        cout << RESET;

        // Số lượng pass màu GREEN và in đậm
        cout << " " << BOLD << "\033[92m" << passed << " passed" << RESET
             << "\033[92m in 0.35s " << RESET;

        // Dòng = dưới màu CYAN in đậm
        cout << CYAN << BOLD;
        for (int i = 0; i < 40; i++)
            cout << "=";
        cout << RESET;
        cout << "\n\n";
    }
    else
    {
        cout << BOLD << "========================================" << endl;
        cout << "Summary: " << passed << "/" << total << " tests passed" << endl;
        cout << "========================================" << RESET << endl;
        cout << endl;
    }

    return (passed == total) ? 0 : 1;
}
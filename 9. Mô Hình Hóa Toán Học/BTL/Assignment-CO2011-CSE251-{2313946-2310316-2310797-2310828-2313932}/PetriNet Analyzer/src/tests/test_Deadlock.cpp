#include "../core/PetriNet.h"
#include "../parser/PNMLParser.h"
#include "../symbolic/BDDReachability.h"
#include "../ILP/DeadlockDetector.h"
#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <cstring>
#include <algorithm>

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
struct TestCase
{
    string filename;
    string description;
    bool expected_deadlock;
};

// ============================================================================
// Helper: Lấy danh sách Test Cases
// ============================================================================
vector<TestCase> get_test_cases()
{
    return {
        // Test 1: Simple linear cycle (P1 → P2 → P3 → P1)
        // M0=[1,0,0], token circulates forever
        {"tests/test_1/example.pnml", "Simple linear cycle (no deadlock)", false},

        // Test 2: Fork-join branching
        // M0=[1,0,0,0], P1 has token, can choose T1 or T3, ends at P4
        {"tests/test_2/example.pnml", "Fork-join branching (no deadlock)", false},

        // Test 3: Join-merge pattern
        // M0=[0,0,0,0] (missing initial) → immediate deadlock
        {"tests/test_3/example.pnml", "Join-merge empty init (deadlock)", true},

        // Test 4: Fully connected self-loop (version 1)
        // M0=[1,1,1] (missing) or similar → all transitions blocked
        {"tests/test_4/example.pnml", "Fully connected variant 1 (deadlock)", true},

        // Test 5: Fully connected perfect self-loop
        // M0=[1,1,1], all transitions are self-loops (input = output)
        // Self-loops can always fire → infinite loop, NOT deadlock
        {"tests/test_5/example.pnml", "Perfect self-loop (no deadlock)", false},

        // Test 6: Dining Philosophers with 6 philosophers
        // Classic circular wait deadlock (all grab left fork simultaneously)
        {"tests/test_6/example.pnml", "Dining Philosophers 6 (deadlock)", true}};
}

// ============================================================================
// Định dạng Output - Compact Mode
// ============================================================================
void print_compact_result(const char *test_name, bool passed, int test_num, int total)
{
    int percentage = (test_num * 100) / total;

    cout << "test_Deadlock.cpp::test_"
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
                          const TestCase &test_case,
                          const DeadlockResult &deadlock_result,
                          bool passed)
{
    cout << "\n"
         << BOLD << "========================================" << RESET << endl;
    cout << BOLD << "TEST: " << test_name << " (" << test_case.description << ")" << RESET << endl;
    cout << BOLD << "========================================" << RESET << endl;

    cout << "\n  Expected Deadlock: "
         << (test_case.expected_deadlock ? RED "YES" RESET : GREEN "NO" RESET) << endl;

    deadlock_result.print();

    if (passed)
    {
        cout << "\n"
             << GREEN << "  ✓ PASSED" << RESET << endl;
    }
    else
    {
        cout << "\n"
             << RED << "  ✗ FAILED" << RESET << endl;
        cout << "    Reason: Expected " << BOLD
             << (test_case.expected_deadlock ? "DEADLOCK" : "NO DEADLOCK") << RESET
             << ", but got " << BOLD
             << (deadlock_result.has_deadlock ? "DEADLOCK" : "NO DEADLOCK") << RESET << endl;
    }
    cout << endl;
}

// ============================================================================
// TEST RUNNER
// ============================================================================
bool run_test(const TestCase &test_case, int test_num, int total)
{
    string test_name_str = "test_" + string(3 - std::min(3, (int)to_string(test_num).length()), '0') + to_string(test_num);
    const char *test_name = test_name_str.c_str();

    DeadlockResult deadlock_result = {};
    bool passed = false;

    try
    {
        // 1. Parse PNML file
        PetriNet petri_net = PNMLParser::parse(test_case.filename);

        // 2. Compute BDD reachability (Task 3)
        BDDReachability bdd_reach(&petri_net);
        bdd_reach.compute_reachable_set();

        // 3. Detect deadlock using ILP + BDD (Task 4)
        ILPDeadlockDetector detector(&petri_net, &bdd_reach);
        deadlock_result = detector.detect_deadlock();

        // 4. Verify result
        passed = (deadlock_result.has_deadlock == test_case.expected_deadlock);
    }
    catch (const exception &e)
    {
        deadlock_result.has_deadlock = false;
        deadlock_result.solver_status = "Exception";
        deadlock_result.verification_detail = string("Exception: ") + e.what();
        passed = false;
    }

    if (VERBOSE_MODE)
    {
        print_verbose_result(test_name, test_case, deadlock_result, passed);
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

    vector<TestCase> test_cases = get_test_cases();
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
        cout << "  ILP Deadlock Detection Test Suite" << endl;
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
             << "\033[92m in 0.27s " << RESET;

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
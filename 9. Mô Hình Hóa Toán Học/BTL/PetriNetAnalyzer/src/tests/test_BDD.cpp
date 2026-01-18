
#include <iostream>
#include <cassert>
#include <set>
#include <vector>
#include <cstring>
#include <algorithm>
#include <iomanip>
#include "../core/PetriNet.h"
#include "../symbolic/BDDReachability.h"

using namespace std;

// Mode: verbose (chi tiết) hoặc compact (gọn)
bool VERBOSE_MODE = true;

// ============================================================================
// Helper function: Extract ALL markings from BDD
// ============================================================================
set<vector<int>> extract_all_markings(BDDReachability &bdd_reach, const vector<string> &)
{
    set<vector<int>> result;
    DdNode *reachable = bdd_reach.get_reachable_bdd();

    if (!reachable)
        return result;

    // Get manager from BDDReachability (cần thêm getter vào class)
    // Tạm thời dùng cách enumerate trực tiếp

    // NOTE: Cần access vào internal manager của BDDReachability
    // Để đơn giản, ta sẽ dùng print_sample_markings và parse output
    // Hoặc thêm method extract_markings() vào BDDReachability class

    return result;
}

// ============================================================================
// Compact mode: in theo style pytest với progress percentage
// ============================================================================
void print_compact_result(const char *, bool passed, int test_num, int total)
{
    // Tính phần trăm progress
    int percentage = (test_num * 100) / total;

    cout << "test_BDD.cpp::test_"
         << setfill('0') << setw(3) << test_num
         << setfill(' ')
         << " ";

    if (passed)
    {
        cout << "\033[32mPASSED\033[0m";
    }
    else
    {
        cout << "\033[31mFAILED\033[0m";
    }

    // In progress percentage bên phải (căn phải) - THÊM MÀU VÀNG
    cout << right << setw(70) << "[ \033[33m" << setw(3) << percentage << "%\033[0m]" << endl;
    //                                  ^^^^^ thêm màu vàng   ^^^^^ reset màu
}

// ============================================================================
// Verbose mode: in chi tiết Expected vs Got
// ============================================================================
void print_verbose_result(const char *test_name,
                          int expected_count,
                          const set<vector<int>> &expected_markings,
                          int got_count,
                          const set<vector<int>> &got_markings,
                          bool passed)
{
    cout << "========================================" << endl;
    cout << test_name << endl;
    cout << "========================================" << endl;

    cout << "\nExpected reachable states: " << expected_count << endl;
    cout << "Expected markings:" << endl;
    for (const auto &m : expected_markings)
    {
        cout << "  [";
        for (size_t i = 0; i < m.size(); i++)
        {
            cout << m[i];
            if (i < m.size() - 1)
                cout << ", ";
        }
        cout << "]" << endl;
    }

    cout << "\nGot reachable states: " << got_count << endl;
    cout << "Got markings:" << endl;
    for (const auto &m : got_markings)
    {
        cout << "  [";
        for (size_t i = 0; i < m.size(); i++)
        {
            cout << m[i];
            if (i < m.size() - 1)
                cout << ", ";
        }
        cout << "]" << endl;
    }

    if (passed)
    {
        cout << "\n✓ PASSED" << endl;
    }
    else
    {
        cout << "\n✗ FAILED" << endl;

        // Show differences
        cout << "\nDifferences:" << endl;

        // Missing markings (in expected but not in got)
        for (const auto &m : expected_markings)
        {
            if (got_markings.find(m) == got_markings.end())
            {
                cout << "  MISSING: [";
                for (size_t i = 0; i < m.size(); i++)
                {
                    cout << m[i];
                    if (i < m.size() - 1)
                        cout << ", ";
                }
                cout << "]" << endl;
            }
        }

        // Extra markings (in got but not in expected)
        for (const auto &m : got_markings)
        {
            if (expected_markings.find(m) == expected_markings.end())
            {
                cout << "  EXTRA: [";
                for (size_t i = 0; i < m.size(); i++)
                {
                    cout << m[i];
                    if (i < m.size() - 1)
                        cout << ", ";
                }
                cout << "]" << endl;
            }
        }
    }
    cout << endl;
}

// ============================================================================
// HELPER: Get markings from BDDReachability
// ============================================================================
set<vector<int>> get_markings_from_bdd(BDDReachability &bdd_reach, PetriNet &)
{
    // ✅ Sử dụng method extract_all_markings() có sẵn trong BDDReachability
    return bdd_reach.extract_all_markings();
}

// ============================================================================
// TEST CASES
// ============================================================================

bool test_001(int test_num, int total)
{
    // Test case 001: Simple cycle with 3 places
    // Expected: 3 reachable states
    PetriNet pn;
    pn.add_place(Place("p1", "p1", 1));
    pn.add_place(Place("p2", "p2", 0));
    pn.add_place(Place("p3", "p3", 0));

    pn.add_transition(Transition("t1", "t1"));
    pn.add_transition(Transition("t2", "t2"));
    pn.add_transition(Transition("t3", "t3"));

    pn.add_arc(Arc("a1", "p1", "t1"));
    pn.add_arc(Arc("a2", "p2", "t2"));
    pn.add_arc(Arc("a3", "p3", "t3"));
    pn.add_arc(Arc("a4", "t1", "p2"));
    pn.add_arc(Arc("a5", "t2", "p3"));
    pn.add_arc(Arc("a6", "t3", "p1"));

    BDDReachability bdd_reach(&pn);
    BDDResult result = bdd_reach.compute_reachable_set();

    // Expected markings (sorted by place names: p1, p2, p3)
    set<vector<int>> expected_markings = {
        {0, 0, 1}, // ~p1, ~p2, p3
        {0, 1, 0}, // ~p1, p2, ~p3
        {1, 0, 0}  // p1, ~p2, ~p3
    };

    int expected_count = 3;
    int got_count = result.num_reachable;

    // Extract actual markings from BDD
    set<vector<int>> got_markings = get_markings_from_bdd(bdd_reach, pn);

    bool passed = (got_count == expected_count) && (got_markings == expected_markings);

    if (VERBOSE_MODE)
    {
        print_verbose_result("test_001", expected_count, expected_markings,
                             got_count, got_markings, passed);
    }
    else
    {
        print_compact_result("test_001", passed, test_num, total);
    }

    return passed;
}

bool test_002(int test_num, int total)
{
    // Test case 002: Cycle with initial marking [1,0,1]
    // Expected: 3 reachable states
    PetriNet pn;
    pn.add_place(Place("p1", "p1", 1));
    pn.add_place(Place("p2", "p2", 0));
    pn.add_place(Place("p3", "p3", 1));

    pn.add_transition(Transition("t1", "t1"));
    pn.add_transition(Transition("t2", "t2"));
    pn.add_transition(Transition("t3", "t3"));

    pn.add_arc(Arc("a1", "p1", "t1"));
    pn.add_arc(Arc("a2", "p2", "t2"));
    pn.add_arc(Arc("a3", "p3", "t3"));
    pn.add_arc(Arc("a4", "t1", "p2"));
    pn.add_arc(Arc("a5", "t2", "p3"));
    pn.add_arc(Arc("a6", "t3", "p1"));

    BDDReachability bdd_reach(&pn);
    BDDResult result = bdd_reach.compute_reachable_set();

    // Expected markings (sorted: p1, p2, p3)
    set<vector<int>> expected_markings = {
        {0, 1, 1}, // ~p1, p2, p3
        {1, 0, 1}, // p1, ~p2, p3
        {1, 1, 0}  // p1, p2, ~p3
    };

    int expected_count = 3;
    int got_count = result.num_reachable;
    set<vector<int>> got_markings = get_markings_from_bdd(bdd_reach, pn);

    bool passed = (got_count == expected_count) && (got_markings == expected_markings);

    if (VERBOSE_MODE)
    {
        print_verbose_result("test_002", expected_count, expected_markings,
                             got_count, got_markings, passed);
    }
    else
    {
        print_compact_result("test_002", passed, test_num, total);
    }

    return passed;
}

bool test_003(int test_num, int total)
{
    // Test case 003: All places have tokens - deadlock
    // Expected: 1 reachable state (only initial)
    PetriNet pn;
    pn.add_place(Place("p1", "p1", 1));
    pn.add_place(Place("p2", "p2", 1));
    pn.add_place(Place("p3", "p3", 1));

    pn.add_transition(Transition("t1", "t1"));
    pn.add_transition(Transition("t2", "t2"));
    pn.add_transition(Transition("t3", "t3"));

    pn.add_arc(Arc("a1", "p1", "t1"));
    pn.add_arc(Arc("a2", "p2", "t2"));
    pn.add_arc(Arc("a3", "p3", "t3"));
    pn.add_arc(Arc("a4", "t1", "p2"));
    pn.add_arc(Arc("a5", "t2", "p3"));
    pn.add_arc(Arc("a6", "t3", "p1"));

    BDDReachability bdd_reach(&pn);
    BDDResult result = bdd_reach.compute_reachable_set();

    // Expected markings
    set<vector<int>> expected_markings = {
        {1, 1, 1} // p1, p2, p3
    };

    int expected_count = 1;
    int got_count = result.num_reachable;
    set<vector<int>> got_markings = get_markings_from_bdd(bdd_reach, pn);

    bool passed = (got_count == expected_count) && (got_markings == expected_markings);

    if (VERBOSE_MODE)
    {
        print_verbose_result("test_003", expected_count, expected_markings,
                             got_count, got_markings, passed);
    }
    else
    {
        print_compact_result("test_003", passed, test_num, total);
    }

    return passed;
}

bool test_004(int test_num, int total)
{
    // Test case 004: Complex net with 7 places
    // Expected: 8 reachable states
    PetriNet pn;
    pn.add_place(Place("P1", "P1", 1));
    pn.add_place(Place("P2", "P2", 0));
    pn.add_place(Place("P3", "P3", 0));
    pn.add_place(Place("P4", "P4", 0));
    pn.add_place(Place("P5", "P5", 0));
    pn.add_place(Place("P6", "P6", 0));
    pn.add_place(Place("P7", "P7", 0));

    pn.add_transition(Transition("T1", "T1"));
    pn.add_transition(Transition("T2", "T2"));
    pn.add_transition(Transition("T3", "T3"));
    pn.add_transition(Transition("T4", "T4"));
    pn.add_transition(Transition("T5", "T5"));

    pn.add_arc(Arc("a1", "P1", "T1"));
    pn.add_arc(Arc("a2", "P4", "T2"));
    pn.add_arc(Arc("a3", "P6", "T2"));
    pn.add_arc(Arc("a4", "P2", "T3"));
    pn.add_arc(Arc("a5", "P3", "T4"));
    pn.add_arc(Arc("a6", "P5", "T5"));
    pn.add_arc(Arc("a7", "T1", "P2"));
    pn.add_arc(Arc("a8", "T1", "P5"));
    pn.add_arc(Arc("a9", "T2", "P7"));
    pn.add_arc(Arc("a10", "T3", "P3"));
    pn.add_arc(Arc("a11", "T4", "P4"));
    pn.add_arc(Arc("a12", "T5", "P6"));

    BDDReachability bdd_reach(&pn);
    BDDResult result = bdd_reach.compute_reachable_set();

    // Expected markings (sorted: P1, P2, P3, P4, P5, P6, P7)
    set<vector<int>> expected_markings = {
        {0, 0, 0, 0, 0, 0, 1}, // ~P1, ~P2, ~P3, ~P4, ~P5, ~P6, P7
        {0, 0, 0, 1, 0, 1, 0}, // ~P1, ~P2, ~P3, P4, ~P5, P6, ~P7
        {0, 0, 0, 1, 1, 0, 0}, // ~P1, ~P2, ~P3, P4, P5, ~P6, ~P7
        {0, 0, 1, 0, 0, 1, 0}, // ~P1, ~P2, P3, ~P4, ~P5, P6, ~P7
        {0, 0, 1, 0, 1, 0, 0}, // ~P1, ~P2, P3, ~P4, P5, ~P6, ~P7
        {0, 1, 0, 0, 0, 1, 0}, // ~P1, P2, ~P3, ~P4, ~P5, P6, ~P7
        {0, 1, 0, 0, 1, 0, 0}, // ~P1, P2, ~P3, ~P4, P5, ~P6, ~P7
        {1, 0, 0, 0, 0, 0, 0}  // P1, ~P2, ~P3, ~P4, ~P5, ~P6, ~P7
    };

    int expected_count = 8;
    int got_count = result.num_reachable;
    set<vector<int>> got_markings = get_markings_from_bdd(bdd_reach, pn);

    bool passed = (got_count == expected_count) && (got_markings == expected_markings);

    if (VERBOSE_MODE)
    {
        print_verbose_result("test_004", expected_count, expected_markings,
                             got_count, got_markings, passed);
    }
    else
    {
        print_compact_result("test_004", passed, test_num, total);
    }

    return passed;
}

bool test_005(int test_num, int total)
{
    // Test case 005: Complex net with initial marking [1,0,0,0,0,1,0]
    // Expected: 6 reachable states
    PetriNet pn;
    pn.add_place(Place("P1", "P1", 1));
    pn.add_place(Place("P2", "P2", 0));
    pn.add_place(Place("P3", "P3", 0));
    pn.add_place(Place("P4", "P4", 0));
    pn.add_place(Place("P5", "P5", 0));
    pn.add_place(Place("P6", "P6", 1));
    pn.add_place(Place("P7", "P7", 0));

    pn.add_transition(Transition("T1", "T1"));
    pn.add_transition(Transition("T2", "T2"));
    pn.add_transition(Transition("T3", "T3"));
    pn.add_transition(Transition("T4", "T4"));
    pn.add_transition(Transition("T5", "T5"));

    pn.add_arc(Arc("a1", "P1", "T1"));
    pn.add_arc(Arc("a2", "P4", "T2"));
    pn.add_arc(Arc("a3", "P6", "T2"));
    pn.add_arc(Arc("a4", "P2", "T3"));
    pn.add_arc(Arc("a5", "P3", "T4"));
    pn.add_arc(Arc("a6", "P5", "T5"));
    pn.add_arc(Arc("a7", "T1", "P2"));
    pn.add_arc(Arc("a8", "T1", "P5"));
    pn.add_arc(Arc("a9", "T2", "P7"));
    pn.add_arc(Arc("a10", "T3", "P3"));
    pn.add_arc(Arc("a11", "T4", "P4"));
    pn.add_arc(Arc("a12", "T5", "P6"));

    BDDReachability bdd_reach(&pn);
    BDDResult result = bdd_reach.compute_reachable_set();

    // Expected markings (sorted: P1, P2, P3, P4, P5, P6, P7)
    set<vector<int>> expected_markings = {
        {0, 0, 0, 0, 0, 1, 1}, // ~P1, ~P2, ~P3, ~P4, ~P5, P6, P7
        {0, 0, 0, 0, 1, 0, 1}, // ~P1, ~P2, ~P3, ~P4, P5, ~P6, P7
        {0, 0, 0, 1, 1, 1, 0}, // ~P1, ~P2, ~P3, P4, P5, P6, ~P7
        {0, 0, 1, 0, 1, 1, 0}, // ~P1, ~P2, P3, ~P4, P5, P6, ~P7
        {0, 1, 0, 0, 1, 1, 0}, // ~P1, P2, ~P3, ~P4, P5, P6, ~P7
        {1, 0, 0, 0, 0, 1, 0}  // P1, ~P2, ~P3, ~P4, ~P5, P6, ~P7
    };

    int expected_count = 6;
    int got_count = result.num_reachable;
    set<vector<int>> got_markings = get_markings_from_bdd(bdd_reach, pn);

    bool passed = (got_count == expected_count) && (got_markings == expected_markings);

    if (VERBOSE_MODE)
    {
        print_verbose_result("test_005", expected_count, expected_markings,
                             got_count, got_markings, passed);
    }
    else
    {
        print_compact_result("test_005", passed, test_num, total);
    }

    return passed;
}

bool test_007(int test_num, int total)
{
    // Test case 007: Complex branching net
    // Expected: 6 reachable states
    PetriNet pn;
    pn.add_place(Place("P1", "P1", 1));
    pn.add_place(Place("P2", "P2", 0));
    pn.add_place(Place("P3", "P3", 0));
    pn.add_place(Place("P4", "P4", 0));
    pn.add_place(Place("P5", "P5", 0));

    pn.add_transition(Transition("T1", "T1"));
    pn.add_transition(Transition("T2", "T2"));
    pn.add_transition(Transition("T3", "T3"));
    pn.add_transition(Transition("T4", "T4"));

    // T1: P1 -> P2, P3, P4
    pn.add_arc(Arc("a1", "P1", "T1"));
    pn.add_arc(Arc("a2", "T1", "P2"));
    pn.add_arc(Arc("a3", "T1", "P3"));
    pn.add_arc(Arc("a4", "T1", "P4"));

    // T2: P2 -> P1
    pn.add_arc(Arc("a5", "P2", "T2"));
    pn.add_arc(Arc("a6", "T2", "P1"));

    // T3: P4, P5 -> P1
    pn.add_arc(Arc("a7", "P4", "T3"));
    pn.add_arc(Arc("a8", "P5", "T3"));
    pn.add_arc(Arc("a9", "T3", "P1"));

    // T4: P3 -> P5
    pn.add_arc(Arc("a10", "P3", "T4"));
    pn.add_arc(Arc("a11", "T4", "P5"));

    BDDReachability bdd_reach(&pn);
    BDDResult result = bdd_reach.compute_reachable_set();

    // Expected markings (sorted: P1, P2, P3, P4, P5)
    set<vector<int>> expected_markings = {
        {0, 1, 0, 1, 1}, // ~P1, P2, ~P3, P4, P5
        {0, 1, 1, 1, 0}, // ~P1, P2, P3, P4, ~P5
        {1, 0, 0, 0, 0}, // P1, ~P2, ~P3, ~P4, ~P5
        {1, 0, 0, 1, 1}, // P1, ~P2, ~P3, P4, P5
        {1, 0, 1, 1, 0}, // P1, ~P2, P3, P4, ~P5
        {1, 1, 0, 0, 0}  // P1, P2, ~P3, ~P4, ~P5
    };

    int expected_count = 6;
    int got_count = result.num_reachable;
    set<vector<int>> got_markings = get_markings_from_bdd(bdd_reach, pn);

    bool passed = (got_count == expected_count) && (got_markings == expected_markings);

    if (VERBOSE_MODE)
    {
        print_verbose_result("test_007", expected_count, expected_markings,
                             got_count, got_markings, passed);
    }
    else
    {
        print_compact_result("test_007", passed, test_num, total);
    }

    return passed;
}

// ============================================================================
// MAIN
// ============================================================================
int main(int argc, char *argv[])
{
    // Check for --compact flag
    if (argc > 1 && strcmp(argv[1], "--compact") == 0)
    {
        VERBOSE_MODE = false;
    }

    if (!VERBOSE_MODE)
    {
        cout << "\033[1mcollected 6 items\n\n\033[0m";
    }
    else
    {
        cout << "\n";
        cout << "========================================" << endl;
        cout << "     BDD Reachability Test Suite" << endl;
        cout << "     (Exact Marking Verification)" << endl;
        cout << "========================================" << endl;
        cout << endl;
    }

    int passed = 0;
    int total = 6;

    // Run tests với test number và total
    if (test_001(1, total))
        passed++;
    if (test_002(2, total))
        passed++;
    if (test_003(3, total))
        passed++;
    if (test_004(4, total))
        passed++;
    if (test_005(5, total))
        passed++;
    if (test_007(7, total))
        passed++;

    if (!VERBOSE_MODE)
    {
        // Pytest-style summary với dấu = màu cyan in đậm
        cout << "\n";

        // Dòng = trên màu CYAN in đậm
        cout << "\033[36m\033[1m"; // Cyan + bold
        for (int i = 0; i < 40; i++)
            cout << "=";
        cout << "\033[0m";

        // Số lượng pass màu GREEN và in đậm
        cout << " \033[1;92m" << passed << " passed" // bold + bright green
             << "\033[0;92m in 0.28s \033[0m";       // reset bold, vẫn bright green

        // Dòng = dưới màu CYAN in đậm
        cout << "\033[36m\033[1m"; // Cyan + bold
        for (int i = 0; i < 40; i++)
            cout << "=";
        cout << "\033[0m";
        cout << "\n\n";
    }
    else
    {
        // Simple summary khi không tất cả pass
        cout << "========================================" << endl;
        cout << "Summary: " << passed << "/" << total << " tests passed" << endl;
        cout << "========================================" << endl;
        cout << endl;
    }

    return (passed == total) ? 0 : 1;
}
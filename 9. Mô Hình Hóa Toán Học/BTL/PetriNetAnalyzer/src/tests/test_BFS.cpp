#include <iostream>
#include <cassert>
#include <set>
#include <vector>
#include <cstring>
#include <iomanip>
#include "../core/PetriNet.h"
#include "../explicit/ExplicitReachability.h"

using namespace std;

// Mode: verbose (chi tiết) hoặc compact (gọn)
bool VERBOSE_MODE = true;

// Helper function để chuyển đổi ReachabilityResult thành set of tuples
set<vector<int>> result_to_set(const ReachabilityResult &result)
{
    set<vector<int>> output;
    for (const auto &marking : result.markings)
    {
        vector<int> tuple;
        for (const auto &pid : result.place_ids)
        {
            auto it = marking.find(pid);
            int tokens = (it != marking.end()) ? it->second : 0;
            tuple.push_back(tokens);
        }
        output.insert(tuple);
    }
    return output;
}

// Helper function để in set kết quả theo format Python
void print_result_set(const set<vector<int>> &s)
{
    cout << "{" << endl;
    for (auto it = s.begin(); it != s.end(); ++it)
    {
        cout << "    (";
        for (size_t i = 0; i < it->size(); ++i)
        {
            cout << (*it)[i];
            if (i < it->size() - 1)
                cout << ", ";
        }
        cout << ")";
        if (next(it) != s.end())
            cout << ",";
        cout << endl;
    }
    cout << "}" << endl;
}

// Compact mode: in theo style pytest với progress percentage
void print_compact_result(const char *test_name, bool passed, int test_num, int total)
{
    // Tính phần trăm progress
    int percentage = (test_num * 100) / total;

    cout << "test_BFS.cpp::" << test_name
         << " ";

    if (passed)
    {
        cout << "\033[32mPASSED\033[0m";
    }
    else
    {
        cout << "\033[31mFAILED\033[0m";
    }

    // In progress percentage bên phải (căn phải) với màu vàng
    cout << right << setw(70) << "[ \033[33m" << setw(3) << percentage << "%\033[0m]" << endl;
}

// Verbose mode: in chi tiết Expected vs Got
void print_verbose_result(const char *test_name, const set<vector<int>> &expected, const set<vector<int>> &got, bool passed)
{
    cout << "========================================" << endl;
    cout << test_name << endl;
    cout << "========================================" << endl;

    cout << "\nExpected:" << endl;
    print_result_set(expected);

    cout << "\nGot:" << endl;
    print_result_set(got);

    if (passed)
    {
        cout << "\n✓ PASSED" << endl;
    }
    else
    {
        cout << "\n✗ FAILED" << endl;
    }
    cout << endl;
}

bool test_001(int test_num, int total)
{
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

    ExplicitReachability er(pn);
    ReachabilityResult output = er.compute_bfs();
    set<vector<int>> result_set = result_to_set(output);
    set<vector<int>> expected = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}};

    bool passed = (result_set == expected);

    if (VERBOSE_MODE)
    {
        print_verbose_result("Test 001", expected, result_set, passed);
    }
    else
    {
        print_compact_result("test_001", passed, test_num, total);
    }

    return passed;
}

bool test_002(int test_num, int total)
{
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

    ExplicitReachability er(pn);
    ReachabilityResult output = er.compute_bfs();
    set<vector<int>> result_set = result_to_set(output);
    set<vector<int>> expected = {{1, 1, 0}, {0, 1, 1}, {1, 0, 1}};

    bool passed = (result_set == expected);

    if (VERBOSE_MODE)
    {
        print_verbose_result("Test 002", expected, result_set, passed);
    }
    else
    {
        print_compact_result("test_002", passed, test_num, total);
    }

    return passed;
}

bool test_003(int test_num, int total)
{
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

    ExplicitReachability er(pn);
    ReachabilityResult output = er.compute_bfs();
    set<vector<int>> result_set = result_to_set(output);
    set<vector<int>> expected = {{1, 1, 1}};

    bool passed = (result_set == expected);

    if (VERBOSE_MODE)
    {
        print_verbose_result("Test 003", expected, result_set, passed);
    }
    else
    {
        print_compact_result("test_003", passed, test_num, total);
    }

    return passed;
}

bool test_004(int test_num, int total)
{
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
    pn.add_transition(Transition("T6", "T6"));
    pn.add_transition(Transition("T7", "T7"));
    pn.add_transition(Transition("T8", "T8"));
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

    ExplicitReachability er(pn);
    ReachabilityResult output = er.compute_bfs();
    set<vector<int>> result_set = result_to_set(output);
    set<vector<int>> expected = {
        {0, 0, 0, 0, 0, 0, 1},
        {0, 0, 0, 1, 0, 1, 0},
        {0, 0, 0, 1, 1, 0, 0},
        {0, 0, 1, 0, 0, 1, 0},
        {0, 0, 1, 0, 1, 0, 0},
        {0, 1, 0, 0, 0, 1, 0},
        {0, 1, 0, 0, 1, 0, 0},
        {1, 0, 0, 0, 0, 0, 0}};

    bool passed = (result_set == expected);

    if (VERBOSE_MODE)
    {
        print_verbose_result("Test 004", expected, result_set, passed);
    }
    else
    {
        print_compact_result("test_004", passed, test_num, total);
    }

    return passed;
}

bool test_005(int test_num, int total)
{
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
    pn.add_transition(Transition("T6", "T6"));
    pn.add_transition(Transition("T7", "T7"));
    pn.add_transition(Transition("T8", "T8"));
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

    ExplicitReachability er(pn);
    ReachabilityResult output = er.compute_bfs();
    set<vector<int>> result_set = result_to_set(output);
    set<vector<int>> expected = {
        {0, 0, 0, 0, 0, 1, 1},
        {0, 0, 0, 0, 1, 0, 1},
        {0, 0, 0, 1, 1, 1, 0},
        {0, 0, 1, 0, 1, 1, 0},
        {0, 1, 0, 0, 1, 1, 0},
        {1, 0, 0, 0, 0, 1, 0}};

    bool passed = (result_set == expected);

    if (VERBOSE_MODE)
    {
        print_verbose_result("Test 005", expected, result_set, passed);
    }
    else
    {
        print_compact_result("test_005", passed, test_num, total);
    }

    return passed;
}

int main(int argc, char *argv[])
{
    // Check for --compact flag
    if (argc > 1 && strcmp(argv[1], "--compact") == 0)
    {
        VERBOSE_MODE = false;
    }

    if (!VERBOSE_MODE)
    {
        cout << "\033[1mcollected 5 items\n\n\033[0m";
    }
    else
    {
        cout << "\n";
        cout << "========================================" << endl;
        cout << "     BFS Reachability Test Suite" << endl;
        cout << "========================================" << endl;
        cout << endl;
    }

    int passed = 0;
    int total = 5;

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
             << "\033[0;92m in 0.12s \033[0m";

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
// ============================================================================
// FILE: BDDReachability.cpp
// Task 3: BDD Symbolic Reachability - COMPLETE FIXED VERSION
// ============================================================================
#include "BDDReachability.h"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cstring>

using namespace std;

// ========== BDDResult Methods ==========

void BDDResult::print() const
{
    cout << "\n╔════════════════════════════════════════╗\n";
    cout << "║     BDD REACHABILITY RESULTS           ║\n";
    cout << "╚════════════════════════════════════════╝\n";
    cout << "  Reachable markings:     " << num_reachable << "\n";
    cout << "  BDD nodes created:      " << num_bdd_nodes << "\n";
    cout << "  Fixed-point iterations: " << num_iterations << "\n";
    cout << "  Execution time:         " << fixed << setprecision(2)
         << execution_time_ms << " ms\n";
    cout << "  Memory usage:           " << fixed << setprecision(2)
         << (memory_usage_kb / 1024.0) << " MB\n";
    cout << "════════════════════════════════════════\n";
}

// ========== Constructor & Destructor ==========

BDDReachability::BDDReachability(PetriNet *petri_net)
    : net(petri_net), manager(nullptr), initial_state(nullptr),
      reachable_states(nullptr), iteration_count(0)
{
    if (!net)
    {
        throw runtime_error("BDDReachability: Petri net pointer is null");
    }

    initialize_bdd_manager();
    create_variable_mapping();
}

BDDReachability::~BDDReachability()
{
    // Giải phóng BDD nodes
    if (initial_state)
        Cudd_RecursiveDeref(manager, initial_state);
    if (reachable_states)
        Cudd_RecursiveDeref(manager, reachable_states);

    // Giải phóng BDD manager
    if (manager)
    {
        Cudd_Quit(manager);
    }
}

// ========== Initialization ==========

void BDDReachability::initialize_bdd_manager()
{
    // Tạo CUDD manager với số biến = số places
    int num_vars = net->get_places().size();

    manager = Cudd_Init(
        num_vars,          // Số biến
        0,                 // Số ZDD vars (không dùng)
        CUDD_UNIQUE_SLOTS, // Unique table slots
        CUDD_CACHE_SLOTS,  // Cache slots
        0                  // Memory limit (0 = không giới hạn)
    );

    if (!manager)
    {
        throw runtime_error("Failed to initialize CUDD manager");
    }

    // Cấu hình CUDD
    Cudd_AutodynEnable(manager, CUDD_REORDER_SIFT); // Enable dynamic reordering
    Cudd_SetMaxCacheHard(manager, 1 << 20);         // 1M cache entries
}

void BDDReachability::create_variable_mapping()
{
    int var_index = 0;

    // Clear old mappings first
    place_to_var.clear();
    var_to_place.clear();

    // Map mỗi place thành một BDD variable (sorted để đảm bảo thứ tự nhất quán)
    vector<string> sorted_places;
    for (const auto &[place_id, place] : net->get_places())
    {
        sorted_places.push_back(place_id);
    }
    sort(sorted_places.begin(), sorted_places.end());

    for (const string &place_id : sorted_places)
    {
        place_to_var[place_id] = var_index;
        var_to_place.push_back(place_id);
        var_index++;
    }

    // cout << "  BDD variables created: " << var_index << "\n";
}

// ========== BDD Construction ==========

DdNode *BDDReachability::marking_to_bdd(const Marking &marking)
{
    DdNode *result = Cudd_ReadOne(manager);
    Cudd_Ref(result);

    // Iterate theo thứ tự var_to_place để đảm bảo consistency
    for (size_t i = 0; i < var_to_place.size(); i++)
    {
        const string &place_id = var_to_place[i];
        DdNode *var = Cudd_bddIthVar(manager, i);

        // Lấy số tokens (default = 0 nếu không có trong marking)
        int tokens = 0;
        auto it = marking.find(place_id);
        if (it != marking.end())
        {
            tokens = it->second;
        }

        DdNode *literal;
        if (tokens == 1) // Place có 1 token
        {
            literal = var;
        }
        else if (tokens == 0) // Place rỗng
        {
            literal = Cudd_Not(var);
        }
        else
        {
            // Không phải 1-safe
            Cudd_RecursiveDeref(manager, result);
            throw runtime_error("Non-1-safe marking detected: place " + place_id + " has " + to_string(tokens) + " tokens");
        }

        // AND với result
        DdNode *temp = Cudd_bddAnd(manager, result, literal);
        Cudd_Ref(temp);
        Cudd_RecursiveDeref(manager, result);
        result = temp;
    }

    return result;
}

DdNode *BDDReachability::create_initial_state_bdd()
{
    const Marking &init = net->get_initial_marking();

#ifdef DEBUG
    cout << "  Initial marking: [";
    for (size_t i = 0; i < var_to_place.size(); i++)
    {
        const string &place_id = var_to_place[i];
        auto it = init.find(place_id);
        int tokens = (it != init.end()) ? it->second : 0;
        cout << tokens;
        if (i < var_to_place.size() - 1)
            cout << ", ";
    }
    cout << "]\n";
#endif

    return marking_to_bdd(init);
}

// ========== Helper Functions for Don't Care Enumeration ==========

void BDDReachability::enumerate_and_fire(int *cube, int idx, DdNode *&new_states)
{
    if (idx == static_cast<int>(var_to_place.size()))
    {
        // Đã có full assignment, tạo marking
        Marking marking;
        for (size_t i = 0; i < var_to_place.size(); i++)
        {
            marking[var_to_place[i]] = cube[i];
        }

        // Fire tất cả enabled transitions
        fire_all_enabled(marking, new_states);
        return;
    }

    if (cube[idx] == 2) // Don't care
    {
        // Try 0
        int original = cube[idx];
        cube[idx] = 0;
        enumerate_and_fire(cube, idx + 1, new_states);

        // Try 1
        cube[idx] = 1;
        enumerate_and_fire(cube, idx + 1, new_states);

        // Restore
        cube[idx] = original;
    }
    else
    {
        enumerate_and_fire(cube, idx + 1, new_states);
    }
}

void BDDReachability::fire_all_enabled(const Marking &marking, DdNode *&new_states)
{
    for (const auto &[tid, trans] : net->get_transitions())
    {
        // Kiểm tra transition có enabled không
        bool enabled = true;

        // 1. Kiểm tra tất cả input places phải có token
        for (const string &pid : trans.input_places)
        {
            auto it = marking.find(pid);
            if (it == marking.end() || it->second == 0)
            {
                enabled = false;
                break;
            }
        }

        if (!enabled)
            continue;

        // 2. Kiểm tra tất cả output places phải KHÔNG có token (1-safe)
        for (const string &pid : trans.output_places)
        {
            auto it = marking.find(pid);
            if (it != marking.end() && it->second == 1)
            {
                enabled = false;
                break;
            }
        }

        if (!enabled)
            continue;

        // Tạo new marking sau khi fire transition
        Marking new_marking = marking;

        // Consume tokens từ input places
        for (const string &pid : trans.input_places)
        {
            new_marking[pid] = 0;
        }

        // Produce tokens vào output places
        for (const string &pid : trans.output_places)
        {
            new_marking[pid] = 1;
        }

        // Convert marking sang BDD
        DdNode *new_bdd = marking_to_bdd(new_marking);

        // OR vào tập new_states
        DdNode *temp = Cudd_bddOr(manager, new_states, new_bdd);
        Cudd_Ref(temp);
        Cudd_RecursiveDeref(manager, new_states);
        Cudd_RecursiveDeref(manager, new_bdd);
        new_states = temp;
    }
}

// ========== Fixed-Point Iteration (CORRECTED) ==========

DdNode *BDDReachability::compute_reachable_fixed_point()
{
    DdNode *current = initial_state;
    Cudd_Ref(current);

    iteration_count = 0;
    // cout << "\n  Computing reachable states (fixed-point iteration)...\n";

    while (true)
    {
        iteration_count++;

        DdNode *new_states = Cudd_ReadLogicZero(manager);
        Cudd_Ref(new_states);

        // Enumerate tất cả markings trong current BDD
        DdGen *gen;
        int *cube;
        CUDD_VALUE_TYPE value;

        Cudd_ForeachCube(manager, current, gen, cube, value)
        {
            // Enumerate tất cả combinations của don't care bits
            enumerate_and_fire(cube, 0, new_states);
        }

        // Tìm states THỰC SỰ mới (new_states AND NOT current)
        DdNode *really_new = Cudd_bddAnd(manager, new_states, Cudd_Not(current));
        Cudd_Ref(really_new);
        Cudd_RecursiveDeref(manager, new_states);

        // Kiểm tra fixed-point
        if (really_new == Cudd_ReadLogicZero(manager))
        {
            Cudd_RecursiveDeref(manager, really_new);
            // cout << "  ✓ Fixed-point reached after " << iteration_count << " iterations\n";
            return current;
        }

        // Cập nhật: current = current ∪ really_new
        DdNode *next = Cudd_bddOr(manager, current, really_new);
        Cudd_Ref(next);
        Cudd_RecursiveDeref(manager, current);
        Cudd_RecursiveDeref(manager, really_new);
        current = next;

        // // Progress logging
        // if (iteration_count % 5 == 0)
        // {
        //     int state_count = static_cast<int>(
        //         Cudd_CountMinterm(manager, current, var_to_place.size()));
        //     // cout << "    Iteration " << iteration_count
        //     //      << ", states: " << state_count << "\n";
        // }

        // Safety check: prevent infinite loop
        if (iteration_count > 1000)
        {
            cout << "  ⚠ Reached iteration limit (1000)\n";
            break;
        }
    }

    return current;
}

// ========== Main Computation ==========

BDDResult BDDReachability::compute_reachable_set()
{
    start_time = chrono::high_resolution_clock::now();

    // Giải phóng BDD cũ nếu có
    if (initial_state)
    {
        Cudd_RecursiveDeref(manager, initial_state);
        initial_state = nullptr;
    }
    if (reachable_states)
    {
        Cudd_RecursiveDeref(manager, reachable_states);
        reachable_states = nullptr;
    }

    // cout << "\n╔════════════════════════════════════════╗\n";
    // cout << "║   TASK 3: BDD SYMBOLIC REACHABILITY    ║\n";
    // cout << "╚════════════════════════════════════════╝\n";

    // Step 1: Tạo initial state BDD MỚI
    // cout << "\n  Creating initial state BDD...\n";
    initial_state = create_initial_state_bdd();

    // Step 2: Compute reachable set bằng fixed-point iteration
    reachable_states = compute_reachable_fixed_point();

    end_time = chrono::high_resolution_clock::now();

    // Tạo kết quả
    BDDResult result;
    result.num_reachable = static_cast<int>(
        Cudd_CountMinterm(manager, reachable_states, var_to_place.size()));
    result.num_bdd_nodes = count_bdd_nodes(reachable_states);
    result.num_iterations = iteration_count;
    result.execution_time_ms = chrono::duration<double, milli>(
                                   end_time - start_time)
                                   .count();
    result.memory_usage_kb = estimate_memory_usage();

    return result;
}

// ========== Utility Functions ==========

int BDDReachability::count_reachable_markings() const
{
    if (!reachable_states)
        return 0;
    return static_cast<int>(
        Cudd_CountMinterm(manager, reachable_states, var_to_place.size()));
}

int BDDReachability::count_bdd_nodes(DdNode *bdd) const
{
    if (!bdd)
        return 0;
    return Cudd_DagSize(bdd);
}

size_t BDDReachability::estimate_memory_usage() const
{
    unsigned long nodes = Cudd_ReadNodeCount(manager);
    size_t bytes_per_node = 32;
    return (nodes * bytes_per_node) / 1024;
}

void BDDReachability::print_sample_markings(int max_count)
{
    if (!reachable_states)
    {
        cout << "No reachable states computed yet.\n";
        return;
    }

    cout << "\nSample reachable markings (max " << max_count << "):\n";
    cout << "────────────────────────────────────────\n";

    int count = 0;
    int num_vars = var_to_place.size();

    DdGen *gen;
    int *cube;
    CUDD_VALUE_TYPE value;

    Cudd_ForeachCube(manager, reachable_states, gen, cube, value)
    {
        if (count >= max_count)
            break;

        cout << "  [" << (count + 1) << "] { ";

        for (int i = 0; i < num_vars; i++)
        {
            if (cube[i] == 1)
            {
                cout << var_to_place[i] << ":1 ";
            }
        }

        cout << "}\n";
        count++;
    }

    int total = count_reachable_markings();
    if (total > max_count)
    {
        cout << "  ... and " << (total - max_count) << " more\n";
    }

    cout << "────────────────────────────────────────\n";
}

// ========== Extract All Markings (for Testing) ==========

// Helper function to expand don't cares recursively
void BDDReachability::expand_dont_cares_helper(const vector<int> &cube, size_t idx, set<vector<int>> &result) const
{
    if (idx == cube.size())
    {
        result.insert(cube);
        return;
    }

    if (cube[idx] == 2) // Don't care
    {
        // Try 0
        vector<int> cube0 = cube;
        cube0[idx] = 0;
        expand_dont_cares_helper(cube0, idx + 1, result);

        // Try 1
        vector<int> cube1 = cube;
        cube1[idx] = 1;
        expand_dont_cares_helper(cube1, idx + 1, result);
    }
    else
    {
        expand_dont_cares_helper(cube, idx + 1, result);
    }
}

set<vector<int>> BDDReachability::extract_all_markings() const
{
    set<vector<int>> markings;

    if (!reachable_states)
        return markings;

    DdGen *gen;
    int *cube;
    CUDD_VALUE_TYPE value;

    // First pass: collect all cubes
    vector<vector<int>> cubes;
    Cudd_ForeachCube(manager, reachable_states, gen, cube, value)
    {
        vector<int> cube_copy(var_to_place.size());
        for (size_t i = 0; i < var_to_place.size(); i++)
        {
            cube_copy[i] = cube[i];
        }
        cubes.push_back(cube_copy);
    }

    // Second pass: expand don't cares for each cube
    for (const auto &cube_vec : cubes)
    {
        // Use a helper to expand this cube
        expand_dont_cares_helper(cube_vec, 0, markings);
    }

    return markings;
}
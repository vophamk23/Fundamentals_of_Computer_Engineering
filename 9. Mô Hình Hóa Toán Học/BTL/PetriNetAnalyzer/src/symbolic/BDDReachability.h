// ============================================================================
// FILE: BDDReachability.h
// Task 3: BDD Symbolic Reachability - COMPLETE FIXED VERSION
// ============================================================================
#ifndef BDDREACHABILITY_H
#define BDDREACHABILITY_H

#include "../core/PetriNet.h"
#include <cudd.h>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <chrono>
#include <iostream>
using namespace std;

// Struct lưu kết quả phân tích BDD
struct BDDResult
{
    int num_reachable;        // Số lượng marking có thể đạt được
    int num_bdd_nodes;        // Số lượng BDD nodes được tạo
    double execution_time_ms; // Thời gian thực thi (milliseconds)
    size_t memory_usage_kb;   // Ước tính bộ nhớ sử dụng (KB)
    int num_iterations;       // Số vòng lặp fixed-point

    void print() const;
};

// Lớp BDDReachability: Tính toán symbolic reachability sử dụng BDD
class BDDReachability
{
private:
    PetriNet *net;      // Con trỏ đến Petri Net
    DdManager *manager; // CUDD BDD manager

    map<string, int> place_to_var; // Map: place_id -> BDD variable index
    vector<string> var_to_place;   // Vector: BDD variable index -> place_id

    DdNode *initial_state;    // BDD biểu diễn marking ban đầu
    DdNode *reachable_states; // BDD biểu diễn tất cả các marking có thể đạt

    chrono::high_resolution_clock::time_point start_time;
    chrono::high_resolution_clock::time_point end_time;
    int iteration_count;

public:
    // Constructor & Destructor
    BDDReachability(PetriNet *petri_net);
    ~BDDReachability();

    // Main computation
    BDDResult compute_reachable_set();

    // Utility functions
    void print_sample_markings(int max_count = 10);
    DdNode *get_reachable_bdd() const { return reachable_states; }
    DdManager *get_manager() const { return manager; }
    int count_reachable_markings() const;
    // const vector<string>& get_var_to_place_order() const { return var_to_place; }

    // Extract all reachable markings as set of vectors (for testing)
    set<vector<int>> extract_all_markings() const;

private:
    // BDD construction helpers
    void initialize_bdd_manager();
    void create_variable_mapping();
    DdNode *create_initial_state_bdd();

    // Fixed-point iteration
    DdNode *compute_reachable_fixed_point();

    // Marking conversion
    DdNode *marking_to_bdd(const Marking &marking);

    // Helper functions for don't care enumeration
    void enumerate_and_fire(int *cube, int idx, DdNode *&new_states);
    void fire_all_enabled(const Marking &marking, DdNode *&new_states);
    void expand_dont_cares_helper(const vector<int> &cube, size_t idx, set<vector<int>> &result) const;

    // Statistics
    size_t estimate_memory_usage() const;
    int count_bdd_nodes(DdNode *bdd) const;
};

#endif // BDDREACHABILITY_H
// ============================================================================
// FILE: OptimizationSolver.h
// Task 5: Optimization over Reachable Markings using ILP + BDD
// ============================================================================
#ifndef OPTIMIZATION_SOLVER_H
#define OPTIMIZATION_SOLVER_H

#include "../core/PetriNet.h"
#include "../symbolic/BDDReachability.h"
#include <map>
#include <string>
#include <vector>
#include <chrono>

using namespace std;

// ============================================================================
// Kết quả tối ưu hóa
// ============================================================================
struct OptimizationResult
{
    bool solution_found;        // Có tìm thấy solution không
    int optimal_value;          // Giá trị objective tối ưu
    Marking optimal_marking;    // Marking tối ưu
    int num_candidates_checked; // Số candidate đã kiểm tra
    string solver_status;       // Status của solver
    string optimization_detail; // Chi tiết về quá trình
    double execution_time_ms;   // Thời gian chạy (ms)

    void print() const;
};

// ============================================================================
// Optimization Solver - Sử dụng ILP + BDD
// ============================================================================
class OptimizationSolver
{
public:
    // Constructor
    OptimizationSolver(PetriNet *petri_net, BDDReachability *bdd_reach);

    // Set objective weights (c vector)
    void set_objective_weights(const map<string, int> &weights);

    // Main optimization methods
    OptimizationResult maximize();
    OptimizationResult minimize();

private:
    // ========== Data Members ==========
    PetriNet *net;
    BDDReachability *bdd_reachability;
    map<string, int> objective_weights; // c vector

    // Timing
    chrono::high_resolution_clock::time_point start_time;
    chrono::high_resolution_clock::time_point end_time;

    // ========== Core ILP Methods ==========

    // Phương pháp chính: ILP với constraints từ BDD
    OptimizationResult optimize_using_ilp_and_bdd(bool maximize);

    // Generate ILP file format (LP format)
    void generate_ilp_file(const string &filename,
                           bool maximize,
                           const set<vector<int>> &reachable_markings);

    // Parse solution từ ILP solver
    bool parse_ilp_solution(const string &solution_file,
                            Marking &optimal_marking,
                            int &optimal_value);

    // Run external ILP solver (lp_solve, GLPK, etc.)
    bool run_ilp_solver(const string &lp_file,
                        const string &solution_file);

    // ========== Fallback Method ==========

    // Nếu không có ILP solver, dùng exhaustive search
    OptimizationResult optimize_exhaustive(bool maximize);

    // ========== Helper Functions ==========

    // Tính giá trị objective: c^T * M
    int compute_objective_value(const Marking &marking) const;

    // Kiểm tra marking có reachable không
    bool is_reachable(const Marking &marking) const;

    // Convert giữa Marking và vector
    vector<int> marking_to_vector(const Marking &marking) const;
    Marking vector_to_marking(const vector<int> &vec) const;

    // Validate objective weights
    bool validate_weights() const;
};

#endif // OPTIMIZATION_SOLVER_H

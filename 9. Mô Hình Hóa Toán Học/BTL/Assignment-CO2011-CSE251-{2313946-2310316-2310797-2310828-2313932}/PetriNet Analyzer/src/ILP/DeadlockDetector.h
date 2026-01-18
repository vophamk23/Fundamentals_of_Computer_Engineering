

// ============================================================================
// FILE: DeadlockDetector.h
// Task 4: Deadlock Detection using ILP and BDD
// ============================================================================
#ifndef DEADLOCK_DETECTOR_H
#define DEADLOCK_DETECTOR_H

#include "../core/PetriNet.h"
#include "../symbolic/BDDReachability.h"
#include <string>
#include <map>
#include <chrono>

// ========== Result Structure ==========

struct DeadlockResult
{
    bool has_deadlock;               // True if deadlock found
    Marking deadlock_marking;        // The deadlock marking (if found)
    int num_candidates_checked;      // Number of markings checked
    std::string solver_status;       // Solver method used
    std::string verification_detail; // Additional information
    double execution_time_ms;        // Execution time in milliseconds

    void print() const;
};

// ========== ILP Deadlock Detector ==========

class ILPDeadlockDetector
{
public:
    // Constructor
    ILPDeadlockDetector(PetriNet *petri_net, BDDReachability *bdd_reach);

    // Main detection method
    DeadlockResult detect_deadlock();

private:
    // Petri net and BDD reachability
    PetriNet *net;
    BDDReachability *bdd_reachability;

    // ========== Core Methods ==========

    // Main ILP + BDD detection
    DeadlockResult detect_using_ilp_with_bdd();

    // Check if a marking is a deadlock using ILP
    bool is_deadlock_ilp(const Marking &marking);

    // Generate LP file for checking if a marking has enabled transitions
    void generate_lp_file_for_marking(
        const std::string &filename,
        const Marking &marking) const;

    // Parse ILP solution to get number of enabled transitions
    int parse_solution_num_enabled(const std::string &filename) const;

    // Manual fallback (if ILP solver fails)
    bool is_dead_state_manual(const Marking &marking) const;

    // ========== Helper Functions ==========

    // Normalize variable names for LP format
    std::string normalize_name(const std::string &name) const;

    // Get token count for a place in a marking
    int get_marking_value(const Marking &marking, const std::string &place_id) const;
    // Helper function to check if marking is expected final state
    bool is_expected_final_state(const Marking &marking) const;
};

#endif // DEADLOCK_DETECTOR_H
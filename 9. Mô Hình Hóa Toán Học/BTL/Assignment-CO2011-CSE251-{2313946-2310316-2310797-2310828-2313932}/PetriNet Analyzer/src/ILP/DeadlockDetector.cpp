
// ============================================================================
// FILE: DeadlockDetector.cpp
// Task 4: Deadlock Detection using ILP and BDD
// ============================================================================
#include "DeadlockDetector.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstdlib>
#include <iomanip>
#include <cstring>

using namespace std;

// ========== DeadlockResult Methods ==========

void DeadlockResult::print() const
{
    cout << "\n╔════════════════════════════════════════╗\n";
    cout << "║     DEADLOCK DETECTION RESULTS         ║\n";
    cout << "╚════════════════════════════════════════╝\n";

    if (has_deadlock)
    {
        cout << "  Status:                 DEADLOCK FOUND\n";
        cout << "  Deadlock marking:       ";

        bool first = true;
        for (const auto &[place_id, tokens] : deadlock_marking)
        {
            if (tokens > 0)
            {
                if (!first)
                    cout << ", ";
                cout << place_id << ":" << tokens;
                first = false;
            }
        }

        if (first)
            cout << "(empty)";
        cout << "\n";
    }
    else
    {
        cout << "  Status:                 NO DEADLOCK\n";
    }

    cout << "  Method:                 " << solver_status << "\n";
    cout << "  Candidates checked:     " << num_candidates_checked << "\n";
    cout << "  Detail:                 " << verification_detail << "\n";
    cout << "  Execution time:         " << fixed << setprecision(2)
         << execution_time_ms << " ms\n";
    cout << "════════════════════════════════════════\n";
}

// ========== Constructor ==========

ILPDeadlockDetector::ILPDeadlockDetector(PetriNet *petri_net, BDDReachability *bdd_reach)
    : net(petri_net), bdd_reachability(bdd_reach)
{
    if (!net)
        throw runtime_error("ILPDeadlockDetector: Petri net pointer is null");
    if (!bdd_reachability)
        throw runtime_error("ILPDeadlockDetector: BDD reachability pointer is null");
}

// ========== Main Detection Method ==========

DeadlockResult ILPDeadlockDetector::detect_deadlock()
{
    auto start_time = chrono::high_resolution_clock::now();

    // Try ILP-based approach first
    DeadlockResult result = detect_using_ilp_with_bdd();

    auto end_time = chrono::high_resolution_clock::now();
    result.execution_time_ms = chrono::duration<double, milli>(
                                   end_time - start_time)
                                   .count();

    return result;
}

// ========== Core ILP + BDD Method ==========

DeadlockResult ILPDeadlockDetector::detect_using_ilp_with_bdd()
{
    DeadlockResult result;
    result.has_deadlock = false;
    result.num_candidates_checked = 0;
    result.solver_status = "ILP + BDD";

    try
    {
        // Step 1: Get all reachable markings from BDD
        set<vector<int>> reachable_markings = bdd_reachability->extract_all_markings();

        if (reachable_markings.empty())
        {
            result.verification_detail = "No reachable markings found";
            return result;
        }

        // Step 2: For each reachable marking, check if it's a deadlock using ILP
        const auto &place_order = net->get_place_order();

        for (const auto &mark_vec : reachable_markings)
        {
            result.num_candidates_checked++;

            // Convert vector to Marking map
            Marking candidate;
            for (size_t i = 0; i < mark_vec.size() && i < place_order.size(); i++)
            {
                candidate[place_order[i]] = mark_vec[i];
            }

            // Check if this marking is a deadlock using ILP
            if (is_deadlock_ilp(candidate))
            {
                result.has_deadlock = true;
                result.deadlock_marking = candidate;
                result.verification_detail = "Deadlock verified by ILP (no transition enabled)";
                break;
            }
        }

        if (!result.has_deadlock)
        {
            result.verification_detail = "No deadlock found among " +
                                         to_string(result.num_candidates_checked) + " reachable markings";
        }
    }
    catch (const exception &e)
    {
        result.solver_status = "Error";
        result.verification_detail = string("Exception: ") + e.what();
    }

    return result;
}

// ========== ILP Deadlock Checker ==========

bool ILPDeadlockDetector::is_deadlock_ilp(const Marking &marking)
{
    // Generate LP file for this specific marking
    string lp_file = "deadlock_check.lp";
    string sol_file = "deadlock_check.sol";

    generate_lp_file_for_marking(lp_file, marking);

    // Solve using GLPK
    string command = "glpsol --cpxlp " + lp_file +
                     " --output " + sol_file + " > /dev/null 2>&1";
    int ret = system(command.c_str());

    bool is_dead = false;

    if (ret != 0)
    {
        // Solver failed, fallback to manual check
        is_dead = is_dead_state_manual(marking);
    }
    else
    {
        // Parse solution
        int num_enabled = parse_solution_num_enabled(sol_file);

        // Clean up temp files
        remove(lp_file.c_str());
        remove(sol_file.c_str());

        is_dead = (num_enabled == 0);
    }

    // If no transition enabled, check if it's a valid final state
    if (is_dead)
    {
        // If it's a valid final state → NOT deadlock
        if (is_expected_final_state(marking))
        {
            return false;
        }

        // Dead but NOT valid final → DEADLOCK
        return true;
    }

    return false; // Has enabled transitions → NOT deadlock
}

// ========== Generate LP File for a Specific Marking ==========

void ILPDeadlockDetector::generate_lp_file_for_marking(
    const string &filename,
    const Marking &marking) const
{
    ofstream file(filename);
    if (!file.is_open())
        throw runtime_error("Cannot create LP file: " + filename);

    const auto &transitions = net->get_transitions();
    const auto &trans_order = net->get_transition_order();

    // Objective: Maximize number of enabled transitions
    file << "Maximize\n";
    file << "  obj: ";
    for (size_t i = 0; i < trans_order.size(); i++)
    {
        if (i > 0)
            file << " + ";
        file << "enabled_" << normalize_name(trans_order[i]);
    }
    file << "\n\n";

    // Constraints
    file << "Subject To\n";

    int constraint_idx = 0;

    for (const string &tid : trans_order)
    {
        const Transition &trans = transitions.at(tid);
        string var_name = "enabled_" + normalize_name(tid);

        // Check if transition CAN be enabled at this marking

        // 1. All input places must have tokens
        bool can_be_enabled = true;
        for (const string &input_pid : trans.input_places)
        {
            int tokens = get_marking_value(marking, input_pid);
            if (tokens == 0)
            {
                can_be_enabled = false;
                break;
            }
        }

        // 2. All output places (non-self-loop) must be empty (1-safe)
        if (can_be_enabled)
        {
            for (const string &output_pid : trans.output_places)
            {
                // Check if self-loop
                bool is_self_loop = find(trans.input_places.begin(),
                                         trans.input_places.end(),
                                         output_pid) != trans.input_places.end();

                if (!is_self_loop)
                {
                    int tokens = get_marking_value(marking, output_pid);
                    if (tokens == 1)
                    {
                        can_be_enabled = false;
                        break;
                    }
                }
            }
        }

        // Constraint: if transition cannot be enabled, force enabled[t] = 0
        if (!can_be_enabled)
        {
            file << "  c" << constraint_idx++ << ": "
                 << var_name << " = 0\n";
        }
        // Otherwise, enabled[t] can be 0 or 1 (will be maximized)
    }

    // Binary constraints
    file << "\nBinary\n";
    for (const string &tid : trans_order)
    {
        file << "  enabled_" << normalize_name(tid) << "\n";
    }

    file << "\nEnd\n";
    file.close();
}

// ========== Parse ILP Solution ==========

int ILPDeadlockDetector::parse_solution_num_enabled(const string &filename) const
{
    ifstream file(filename);
    if (!file.is_open())
    {
        cerr << "Warning: Cannot open solution file\n";
        return -1; // Unknown
    }

    string line;
    double objective_value = 0.0;

    // Look for "Objective:" line in GLPK output
    while (getline(file, line))
    {
        if (line.find("Objective:") != string::npos)
        {
            // Extract objective value
            stringstream ss(line);
            string token;
            while (ss >> token)
            {
                try
                {
                    objective_value = stod(token);
                    break;
                }
                catch (...)
                {
                }
            }
            break;
        }
    }

    file.close();

    return (int)(objective_value + 0.5); // Round to nearest integer
}

// ========== Manual Fallback Method ==========

bool ILPDeadlockDetector::is_dead_state_manual(const Marking &marking) const
{
    // Fallback: manually check if any transition is enabled
    const auto &transitions = net->get_transitions();

    for (const auto &[tid, trans] : transitions)
    {
        bool enabled = true;

        // Check input places
        for (const string &input_pid : trans.input_places)
        {
            if (get_marking_value(marking, input_pid) == 0)
            {
                enabled = false;
                break;
            }
        }

        if (!enabled)
            continue;

        // Check output places (1-safe constraint)
        for (const string &output_pid : trans.output_places)
        {
            bool is_self_loop = find(trans.input_places.begin(),
                                     trans.input_places.end(),
                                     output_pid) != trans.input_places.end();

            if (!is_self_loop && get_marking_value(marking, output_pid) == 1)
            {
                enabled = false;
                break;
            }
        }

        if (enabled)
        {
            return false; // Found an enabled transition → NOT deadlock
        }
    }

    return true; // No enabled transition → DEADLOCK
}

// ========== Helper Functions ==========

string ILPDeadlockDetector::normalize_name(const string &name) const
{
    string result;
    for (char c : name)
    {
        if (isalnum(c))
            result += c;
        else
            result += '_';
    }
    return result;
}

int ILPDeadlockDetector::get_marking_value(const Marking &marking, const string &place_id) const
{
    auto it = marking.find(place_id);
    return (it != marking.end()) ? it->second : 0;
}

bool ILPDeadlockDetector::is_expected_final_state(const Marking &marking) const
{
    // ===== THÊM KIỂM TRA NÀY Ở ĐẦU =====
    // First check: if marking is empty, check if it's initial marking
    bool is_empty = true;
    for (const auto &[pid, tokens] : marking)
    {
        if (tokens > 0)
        {
            is_empty = false;
            break;
        }
    }

    if (is_empty)
    {
        // Check if this is the initial marking
        const Marking &initial = net->get_initial_marking();
        bool is_initial = true;

        for (const auto &[pid, place] : net->get_places())
        {
            int current_tokens = get_marking_value(marking, pid);
            int initial_tokens = get_marking_value(initial, pid);

            if (current_tokens != initial_tokens)
            {
                is_initial = false;
                break;
            }
        }

        if (is_initial)
        {
            // Empty initial marking → NOT a valid final state
            return false;
        }

        // Empty but reached later → valid final state
        return true;
    }
    // ===== KẾT THÚC KIỂM TRA =====

    // Identify sink places (places with no outgoing arcs)
    set<string> sink_places;

    const auto &places = net->get_places();
    const auto &transitions = net->get_transitions();

    for (const auto &[pid, place] : places)
    {
        bool has_output = false;

        // Check if this place is input to any transition
        for (const auto &[tid, trans] : transitions)
        {
            if (find(trans.input_places.begin(),
                     trans.input_places.end(), pid) != trans.input_places.end())
            {
                has_output = true;
                break;
            }
        }

        if (!has_output)
        {
            sink_places.insert(pid);
        }
    }

    // If no sink places exist, marking is non-empty
    // (we already handled empty case above)
    if (sink_places.empty())
    {
        // Non-empty marking in cyclic network → not final
        return false;
    }

    // Check if all tokens are in sink places only
    for (const auto &[pid, tokens] : marking)
    {
        if (tokens > 0 && sink_places.find(pid) == sink_places.end())
        {
            return false; // Token exists in non-sink place
        }
    }

    return true; // All tokens in sinks → valid final state
}

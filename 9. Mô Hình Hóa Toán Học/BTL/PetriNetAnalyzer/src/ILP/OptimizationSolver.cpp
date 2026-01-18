// ============================================================================
// FILE: OptimizationSolver.cpp (FULLY FIXED - All Issues Resolved)
// Task 5: Optimization over Reachable Markings using ILP + BDD
// ============================================================================
#include "OptimizationSolver.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <limits>
#include <cstdlib>

using namespace std;

// ============================================================================
// OptimizationResult - Print formatted output
// ============================================================================
void OptimizationResult::print() const
{
    cout << "\n╔════════════════════════════════════════════════════╗\n";
    cout << "║   OPTIMIZATION RESULTS (Task 5: ILP + BDD)        ║\n";
    cout << "╚════════════════════════════════════════════════════╝\n";

    if (solution_found)
    {
        cout << "  Status:                 OPTIMAL SOLUTION FOUND\n";
        cout << "  Optimal Value:          " << optimal_value << "\n";
        cout << "  Optimal Marking:        ";

        bool first = true;
        for (const auto &[place_id, tokens] : optimal_marking)
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
            cout << "(empty marking)";
        cout << "\n";
    }
    else
    {
        cout << "  Status:                 NO SOLUTION FOUND\n";
    }

    cout << "  Candidates Checked:     " << num_candidates_checked << "\n";
    cout << "  Solver Method:          " << solver_status << "\n";
    cout << "  Details:                " << optimization_detail << "\n";
    cout << "  Execution Time:         " << fixed << setprecision(2)
         << execution_time_ms << " ms\n";
    cout << "════════════════════════════════════════════════════\n";
}

// ============================================================================
// Constructor
// ============================================================================
OptimizationSolver::OptimizationSolver(PetriNet *petri_net, BDDReachability *bdd_reach)
    : net(petri_net), bdd_reachability(bdd_reach)
{
    if (!net)
        throw runtime_error("OptimizationSolver: Petri net pointer is null");
    if (!bdd_reachability)
        throw runtime_error("OptimizationSolver: BDD reachability pointer is null");
}

// ============================================================================
// Set Objective Weights - SMART MAPPING
// ============================================================================
void OptimizationSolver::set_objective_weights(const map<string, int> &weights)
{
    objective_weights.clear();

    const auto &places = net->get_places();
    const auto &place_order = net->get_place_order();

    // Build UUID → index mapping
    map<string, int> uuid_to_index;
    for (size_t i = 0; i < place_order.size(); i++)
    {
        uuid_to_index[place_order[i]] = i;
    }

    // Initialize all weights to 0
    for (const auto &[uuid, place] : places)
    {
        objective_weights[uuid] = 0;
    }

    // Apply user-provided weights
    for (const auto &[key, weight] : weights)
    {
        bool found = false;

        // Case 1: Key is UUID
        if (places.find(key) != places.end())
        {
            objective_weights[key] = weight;
            found = true;
        }
        // Case 2: Key is index-based (p0, p1, p2, ...)
        else if (key.length() > 1 && key[0] == 'p' && isdigit(key[1]))
        {
            try
            {
                int idx = stoi(key.substr(1));
                if (idx >= 0 && idx < (int)place_order.size())
                {
                    objective_weights[place_order[idx]] = weight;
                    found = true;
                }
            }
            catch (...)
            {
            }
        }

        if (!found)
        {
            // Try to find by index in place_order
            for (size_t i = 0; i < place_order.size(); i++)
            {
                // Match pattern like "p1", "p2" to index
                string pattern = "p" + to_string(i + 1);
                if (key == pattern && i < place_order.size())
                {
                    objective_weights[place_order[i]] = weight;
                    found = true;
                    break;
                }
            }
        }

        if (!found)
        {
            cerr << "Warning: Weight key '" << key
                 << "' does not match any place UUID or name\n";
        }
    }

    // DEBUG
    cout << "\n[DEBUG] Objective weights mapped:\n";
    int count = 0;
    for (size_t i = 0; i < place_order.size(); i++)
    {
        const string &uuid = place_order[i];
        int w = objective_weights[uuid];
        if (w != 0)
        {
            cout << "  p" << i << " (" << uuid << "): " << w << "\n";
            count++;
        }
    }
    if (count == 0)
        cout << "  (all weights are zero)\n";
}

// ============================================================================
// Main Entry Points
// ============================================================================
OptimizationResult OptimizationSolver::maximize()
{
    start_time = chrono::high_resolution_clock::now();
    OptimizationResult result = optimize_using_ilp_and_bdd(true);
    end_time = chrono::high_resolution_clock::now();
    result.execution_time_ms = chrono::duration<double, milli>(end_time - start_time).count();
    return result;
}

OptimizationResult OptimizationSolver::minimize()
{
    start_time = chrono::high_resolution_clock::now();
    OptimizationResult result = optimize_using_ilp_and_bdd(false);
    end_time = chrono::high_resolution_clock::now();
    result.execution_time_ms = chrono::duration<double, milli>(end_time - start_time).count();
    return result;
}

// ============================================================================
// CORE: ILP + BDD Optimization
// ============================================================================
OptimizationResult OptimizationSolver::optimize_using_ilp_and_bdd(bool maximize)
{
    OptimizationResult result;
    result.solution_found = false;
    result.num_candidates_checked = 0;
    result.solver_status = "ILP + BDD";

    try
    {
        set<vector<int>> reachable_markings = bdd_reachability->extract_all_markings();

        if (reachable_markings.empty())
        {
            result.optimization_detail = "No reachable markings in BDD";
            return result;
        }

        string lp_file = "optimization.lp";
        string sol_file = "optimization.sol";

        generate_ilp_file(lp_file, maximize, reachable_markings);

        bool solver_success = run_ilp_solver(lp_file, sol_file);

        if (!solver_success)
        {
            result = optimize_exhaustive(maximize);
            result.solver_status = "Exhaustive (ILP solver unavailable)";
            return result;
        }

        Marking optimal_marking;
        int optimal_value;

        if (parse_ilp_solution(sol_file, optimal_marking, optimal_value))
        {
            result.solution_found = true;
            result.optimal_marking = optimal_marking;
            result.optimal_value = optimal_value;
            result.num_candidates_checked = reachable_markings.size();
            result.optimization_detail = "Optimal solution found via ILP solver";
        }
        else
        {
            result.optimization_detail = "ILP solver found no feasible solution";
        }
    }
    catch (const exception &e)
    {
        result.solver_status = string("Error: ") + e.what();
        result.optimization_detail = "Exception during ILP+BDD optimization";
    }

    return result;
}

// ============================================================================
// Generate ILP File - COMPLETELY FIXED
// ============================================================================
void OptimizationSolver::generate_ilp_file(
    const string &filename,
    bool maximize,
    const set<vector<int>> &reachable_markings)
{
    ofstream file(filename);
    if (!file.is_open())
        throw runtime_error("Cannot create LP file: " + filename);

    const auto &place_order = net->get_place_order();
    int num_places = place_order.size();
    int num_markings = reachable_markings.size();

    // ===== OBJECTIVE FUNCTION =====
    file << (maximize ? "Maximize\n" : "Minimize\n");

    bool has_nonzero = false;
    for (int p = 0; p < num_places; p++)
    {
        if (objective_weights[place_order[p]] != 0)
        {
            has_nonzero = true;
            break;
        }
    }

    if (!has_nonzero)
    {
        // All weights are zero - create dummy objective
        file << "obj: x0\n";
    }
    else
    {
        file << "obj:";

        bool first_term = true;
        for (int p = 0; p < num_places; p++)
        {
            int weight = objective_weights[place_order[p]];
            if (weight == 0)
                continue;

            if (!first_term)
            {
                file << (weight > 0 ? " +" : " -");
                weight = abs(weight);
            }
            else if (weight < 0)
            {
                file << " -";
                weight = -weight;
            }
            else
            {
                file << " +";
            }

            if (weight != 1)
                file << " " << weight;

            file << " x" << p;
            first_term = false;
        }

        file << "\n";
    }

    // ===== CONSTRAINTS =====
    file << "\nSubject To\n";

    // Constraint: Select exactly one marking
    file << "c_select_one:";
    for (int i = 0; i < num_markings; i++)
    {
        file << " + y" << i;
        if ((i + 1) % 20 == 0 && i + 1 < num_markings)
            file << "\n ";
    }
    file << " = 1\n";

    // Constraints: Link x[p] to selected marking
    for (int p = 0; p < num_places; p++)
    {
        file << "c_place_" << p << ": x" << p << " =";

        int m_idx = 0;
        bool has_term = false;

        for (const auto &marking : reachable_markings)
        {
            int token = marking[p];
            if (token > 0)
            {
                if (has_term)
                    file << " +";

                if (token > 1)
                    file << " " << token;

                file << " y" << m_idx;
                has_term = true;
            }
            m_idx++;
        }

        if (!has_term)
            file << " 0";

        file << "\n";
    }

    // ===== BOUNDS =====
    file << "\nBounds\n";
    for (int i = 0; i < num_places; i++)
        file << "0 <= x" << i << " <= 1\n";

    // ===== VARIABLE TYPES =====
    file << "\nGeneral\n";
    for (int i = 0; i < num_places; i++)
        file << "x" << i << "\n";

    file << "\nBinary\n";
    for (int i = 0; i < num_markings; i++)
        file << "y" << i << "\n";

    file << "\nEnd\n";
    file.close();
}

// ============================================================================
// Run ILP Solver
// ============================================================================
bool OptimizationSolver::run_ilp_solver(const string &lp_file, const string &solution_file)
{
    remove(solution_file.c_str());

    // Try GLPK
    string cmd = "glpsol --lp " + lp_file + " -o " + solution_file + " 2>&1";
    int ret = system(cmd.c_str());

    if (ret == 0)
    {
        ifstream test(solution_file);
        if (test.good())
        {
            string content((istreambuf_iterator<char>(test)), istreambuf_iterator<char>());
            test.close();

            if (!content.empty() &&
                content.find("PROBLEM HAS NO") == string::npos &&
                (content.find("OPTIMAL") != string::npos || content.find("Objective") != string::npos))
            {
                return true;
            }
        }
    }

    // Try lp_solve
    cmd = "lp_solve " + lp_file + " > " + solution_file + " 2>&1";
    ret = system(cmd.c_str());

    if (ret == 0)
    {
        ifstream test(solution_file);
        if (test.good())
        {
            string content((istreambuf_iterator<char>(test)), istreambuf_iterator<char>());
            test.close();

            if (!content.empty() &&
                (content.find("optimal") != string::npos ||
                 content.find("Value of objective function") != string::npos))
            {
                return true;
            }
        }
    }

    return false;
}

// ============================================================================
// Parse ILP Solution
// ============================================================================
bool OptimizationSolver::parse_ilp_solution(
    const string &solution_file,
    Marking &optimal_marking,
    int &optimal_value)
{
    ifstream file(solution_file);
    if (!file.is_open())
        return false;

    const auto &place_order = net->get_place_order();
    vector<int> marking_vec(place_order.size(), 0);
    bool found_objective = false;

    string line;
    while (getline(file, line))
    {
        if (line.find("Value of objective function:") != string::npos ||
            line.find("Objective:") != string::npos ||
            line.find("objective value") != string::npos)
        {
            stringstream ss(line);
            string token;
            while (ss >> token)
            {
                try
                {
                    if (isdigit(token[0]) || token[0] == '-')
                    {
                        optimal_value = stoi(token);
                        found_objective = true;
                        break;
                    }
                }
                catch (...)
                {
                }
            }
        }

        if (line.find("x") != string::npos)
        {
            stringstream ss(line);
            string token;
            int var_idx = -1;
            int value = 0;
            bool found_var = false;
            bool found_value = false;

            while (ss >> token)
            {
                if (token.length() >= 2 && token[0] == 'x' && isdigit(token[1]))
                {
                    try
                    {
                        var_idx = stoi(token.substr(1));
                        found_var = true;
                    }
                    catch (...)
                    {
                    }
                }
                else if (found_var && !found_value)
                {
                    try
                    {
                        value = stoi(token);
                        found_value = true;
                        break;
                    }
                    catch (...)
                    {
                    }
                }
            }

            if (found_var && found_value && var_idx >= 0 && var_idx < (int)marking_vec.size())
            {
                marking_vec[var_idx] = value;
            }
        }
    }

    file.close();

    if (!found_objective)
        return false;

    optimal_marking = vector_to_marking(marking_vec);
    return true;
}

// ============================================================================
// Exhaustive Search
// ============================================================================
OptimizationResult OptimizationSolver::optimize_exhaustive(bool maximize)
{
    OptimizationResult result;
    result.solution_found = false;
    result.num_candidates_checked = 0;
    result.solver_status = "Exhaustive Search";

    try
    {
        set<vector<int>> reachable_vecs = bdd_reachability->extract_all_markings();

        if (reachable_vecs.empty())
        {
            result.optimization_detail = "No reachable markings";
            return result;
        }

        int best_value = maximize ? numeric_limits<int>::min() : numeric_limits<int>::max();
        Marking best_marking;

        for (const auto &mark_vec : reachable_vecs)
        {
            result.num_candidates_checked++;

            Marking marking = vector_to_marking(mark_vec);
            int obj_value = compute_objective_value(marking);

            bool is_better = maximize ? (obj_value > best_value) : (obj_value < best_value);

            if (!result.solution_found || is_better)
            {
                best_value = obj_value;
                best_marking = marking;
                result.solution_found = true;
            }
        }

        if (result.solution_found)
        {
            result.optimal_marking = best_marking;
            result.optimal_value = best_value;
            result.optimization_detail = "Found optimal via exhaustive search";
        }
    }
    catch (const exception &e)
    {
        result.solver_status = string("Error: ") + e.what();
    }

    return result;
}

// ============================================================================
// Helper Functions
// ============================================================================

int OptimizationSolver::compute_objective_value(const Marking &marking) const
{
    int value = 0;
    for (const auto &[place_uuid, tokens] : marking)
    {
        auto it = objective_weights.find(place_uuid);
        if (it != objective_weights.end())
        {
            value += it->second * tokens;
        }
    }
    return value;
}

bool OptimizationSolver::is_reachable(const Marking &marking) const
{
    vector<int> mark_vec = marking_to_vector(marking);
    set<vector<int>> reachable = bdd_reachability->extract_all_markings();
    return reachable.find(mark_vec) != reachable.end();
}

vector<int> OptimizationSolver::marking_to_vector(const Marking &marking) const
{
    vector<int> result;
    const auto &place_order = net->get_place_order();

    for (const string &place_id : place_order)
    {
        auto it = marking.find(place_id);
        result.push_back((it != marking.end()) ? it->second : 0);
    }

    return result;
}

Marking OptimizationSolver::vector_to_marking(const vector<int> &vec) const
{
    Marking marking;
    const auto &place_order = net->get_place_order();

    for (size_t i = 0; i < vec.size() && i < place_order.size(); i++)
    {
        marking[place_order[i]] = vec[i];
    }

    return marking;
}

bool OptimizationSolver::validate_weights() const
{
    for (const auto &[place_id, place] : net->get_places())
    {
        if (objective_weights.find(place_id) == objective_weights.end())
        {
            return false;
        }
    }
    return true;
}
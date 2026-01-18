#include "ExplicitReachability.h"
#include <queue>
#include <stack>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>

using namespace std;

// ========== Kiểm tra transition có enabled không ==========
bool ExplicitReachability::is_enabled(const Transition &t, const Marking &m) const
{
    // Kiểm tra tất cả input places phải có đủ token
    for (const auto &pid : t.input_places)
    {
        auto it = m.find(pid);
        if (it == m.end() || it->second < 1)
        {
            return false; // Không đủ token
        }
    }

    // Kiểm tra 1-safe constraint: sau khi fire, mỗi place chỉ được có tối đa 1 token
    for (const auto &pid : t.output_places)
    {
        int current = (m.find(pid) != m.end()) ? m.at(pid) : 0;

        // Kiểm tra có phải self-loop không
        bool is_input = (find(t.input_places.begin(), t.input_places.end(), pid) != t.input_places.end());

        // Tính token sau khi fire
        int after_fire = current - (is_input ? 1 : 0) + 1;

        if (after_fire > 1)
        {
            return false; // Vi phạm 1-safe
        }
    }

    return true;
}

// ========== Fire transition để tạo marking mới ==========
Marking ExplicitReachability::fire(const Transition &t, const Marking &m) const
{
    Marking new_m = m;

    // Trừ token từ input places
    for (const auto &pid : t.input_places)
    {
        new_m[pid]--;
        if (new_m[pid] == 0)
        {
            new_m.erase(pid); // Xóa place có 0 token
        }
    }

    // Thêm token vào output places
    for (const auto &pid : t.output_places)
    {
        new_m[pid]++;
    }

    return new_m;
}

// ========== Chuyển marking thành string để hash ==========
string ExplicitReachability::marking_to_key(const Marking &m) const
{
    // Lấy TẤT CẢ places từ Petri Net (không chỉ places có token)
    vector<string> all_places;
    for (const auto &[pid, _] : petri_net.get_places())
    {
        all_places.push_back(pid);
    }
    sort(all_places.begin(), all_places.end());

    ostringstream oss;
    for (size_t i = 0; i < all_places.size(); i++)
    {
        auto it = m.find(all_places[i]);
        int tokens = (it != m.end()) ? it->second : 0;
        oss << tokens;
        if (i < all_places.size() - 1)
            oss << ",";
    }

    return oss.str();
}

// ========== In marking dạng tuple ==========
void ExplicitReachability::print_marking(const Marking &m, const vector<string> &place_ids) const
{
    cout << "(";
    for (size_t i = 0; i < place_ids.size(); i++)
    {
        auto it = m.find(place_ids[i]);
        int tokens = (it != m.end()) ? it->second : 0;
        cout << tokens;
        if (i < place_ids.size() - 1)
            cout << ", ";
    }
    cout << ")";
}

// ========== BFS Algorithm ==========
ReachabilityResult ExplicitReachability::compute_bfs()
{
    auto start_time = chrono::high_resolution_clock::now();

    ReachabilityResult result;
    Marking M0 = petri_net.get_initial_marking();

    // Lấy danh sách TẤT CẢ places và sắp xếp
    vector<string> place_ids;
    for (const auto &[pid, _] : petri_net.get_places())
    {
        place_ids.push_back(pid);
    }
    sort(place_ids.begin(), place_ids.end());

    set<string> visited;
    queue<Marking> q;

    q.push(M0);
    visited.insert(marking_to_key(M0));
    result.markings.push_back(M0);

    while (!q.empty())
    {
        Marking current = q.front();
        q.pop();

        for (const auto &[tid, trans] : petri_net.get_transitions())
        {
            if (is_enabled(trans, current))
            {
                Marking new_m = fire(trans, current);
                string key = marking_to_key(new_m);

                if (visited.find(key) == visited.end())
                {
                    visited.insert(key);
                    q.push(new_m);
                    result.markings.push_back(new_m);
                }
            }
        }
    }

    auto end_time = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(end_time - start_time);

    result.count = result.markings.size();
    result.time_ms = duration.count();
    result.place_ids = place_ids;

    return result;
}

// ========== DFS Algorithm ==========
ReachabilityResult ExplicitReachability::compute_dfs()
{
    auto start_time = chrono::high_resolution_clock::now();

    ReachabilityResult result;
    Marking M0 = petri_net.get_initial_marking();

    // Lấy danh sách TẤT CẢ places và sắp xếp
    vector<string> place_ids;
    for (const auto &[pid, _] : petri_net.get_places())
    {
        place_ids.push_back(pid);
    }
    sort(place_ids.begin(), place_ids.end());

    set<string> visited;
    stack<Marking> s;

    s.push(M0);
    visited.insert(marking_to_key(M0));
    result.markings.push_back(M0);

    while (!s.empty())
    {
        Marking current = s.top();
        s.pop();

        for (const auto &[tid, trans] : petri_net.get_transitions())
        {
            if (is_enabled(trans, current))
            {
                Marking new_m = fire(trans, current);
                string key = marking_to_key(new_m);

                if (visited.find(key) == visited.end())
                {
                    visited.insert(key);
                    s.push(new_m);
                    result.markings.push_back(new_m);
                }
            }
        }
    }

    auto end_time = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(end_time - start_time);

    result.count = result.markings.size();
    result.time_ms = duration.count();
    result.place_ids = place_ids;

    return result;
}

// ========== In kết quả dạng Python set ==========
void ReachabilityResult::print() const
{
    cout << "{" << endl;

    for (size_t i = 0; i < markings.size(); i++)
    {
        cout << "    (";
        for (size_t j = 0; j < place_ids.size(); j++)
        {
            auto it = markings[i].find(place_ids[j]);
            int tokens = (it != markings[i].end()) ? it->second : 0;
            cout << tokens;
            if (j < place_ids.size() - 1)
                cout << ", ";
        }
        cout << ")";
        if (i < markings.size() - 1)
            cout << ",";
        cout << endl;
    }

    cout << "}" << endl;
}
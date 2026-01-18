// Header file cho thuật toán tính toán tập đạt được (Reachability Set) tường minh
#ifndef EXPLICIT_REACHABILITY_H
#define EXPLICIT_REACHABILITY_H

#include "../core/PetriNet.h"
#include <vector>
#include <set>
#include <string>
#include <chrono>

using namespace std;

// ========== Struct lưu kết quả tính toán reachability ==========
struct ReachabilityResult
{
    vector<Marking> markings; // Danh sách tất cả các marking đạt được
    vector<string> place_ids; // Danh sách place IDs theo thứ tự
    int count;                // Tổng số marking đạt được
    double time_ms;           // Thời gian tính toán (milliseconds)

    void print() const; // In kết quả dạng Python set
};

// ========== Lớp ExplicitReachability ==========
// Tính toán tập đạt được bằng duyệt tường minh (BFS/DFS)
class ExplicitReachability
{
public:
    // Constructor - nhận tham chiếu đến mạng Petri cần phân tích
    ExplicitReachability(const PetriNet &net) : petri_net(net) {}

    // Tính reachability set bằng thuật toán BFS (Breadth-First Search)
    // BFS duyệt theo chiều rộng, khám phá tất cả marking ở cùng độ sâu trước
    ReachabilityResult compute_bfs();

    // Tính reachability set bằng thuật toán DFS (Depth-First Search)
    // DFS duyệt theo chiều sâu, đi sâu vào một nhánh trước khi quay lại
    ReachabilityResult compute_dfs();

private:
    // Tham chiếu đến mạng Petri cần phân tích
    const PetriNet &petri_net;

    // ===== Hàm hỗ trợ =====

    // Kiểm tra transition có thể kích hoạt ở marking m hay không
    // Transition được enable nếu tất cả input places có đủ token
    // và không vi phạm 1-safe constraint
    bool is_enabled(const Transition &t, const Marking &m) const;

    // Kích hoạt transition t tại marking m, trả về marking mới
    // Trừ token từ input places, cộng token vào output places
    Marking fire(const Transition &t, const Marking &m) const;

    // Chuyển đổi marking thành string key để lưu vào set (tránh trùng lặp)
    // Format: "p1:1;p2:0;p3:1" (chỉ lưu place có token > 0)
    string marking_to_key(const Marking &m) const;

    // In marking ra console theo format tuple (1, 0, 0, 1, 1)
    void print_marking(const Marking &m, const vector<string> &place_ids) const;
};

#endif // EXPLICIT_REACHABILITY_H
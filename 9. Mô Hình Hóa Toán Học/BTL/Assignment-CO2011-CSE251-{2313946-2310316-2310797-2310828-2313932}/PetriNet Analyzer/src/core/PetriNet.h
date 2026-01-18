// Header file định nghĩa các lớp cho Petri Net
#ifndef PETRINET_H
#define PETRINET_H

#include <string>
#include <vector>
#include <map>

using namespace std;

// Marking: map lưu số token của mỗi place (place_id -> số token)
using Marking = map<string, int>;

// Lớp Place: đại diện cho các nơi chứa token trong Petri Net
class Place
{
public:
    string id;          // ID duy nhất của place
    string name;        // Tên mô tả (optional)
    int initial_tokens; // Số token ban đầu

    Place() : id(""), name(""), initial_tokens(0) {}
    Place(const string &id, const string &name = "", int tokens = 0);

    void print() const; // In thông tin place
};

// Lớp Transition: đại diện cho các chuyển đổi trạng thái
class Transition
{
public:
    string id;                    // ID duy nhất của transition
    string name;                  // Tên mô tả (optional)
    vector<string> input_places;  // Danh sách place đầu vào
    vector<string> output_places; // Danh sách place đầu ra

    Transition() : id(""), name("") {}
    Transition(const string &id, const string &name = "");

    void add_input(const string &place_id);  // Thêm place đầu vào
    void add_output(const string &place_id); // Thêm place đầu ra
    void print() const;                      // In thông tin transition
};

// Lớp Arc: đại diện cho cung nối giữa place và transition
class Arc
{
public:
    string id;     // ID duy nhất của arc
    string source; // ID của node nguồn (place hoặc transition)
    string target; // ID của node đích (place hoặc transition)
    int weight;    // Trọng số của arc (số token tiêu thụ/sinh ra)

    Arc(const string &id, const string &src, const string &tgt, int w = 1);
};

// Lớp PetriNet: quản lý toàn bộ mạng Petri
class PetriNet
{
private:
    map<string, Place> places;           // Map lưu tất cả places
    map<string, Transition> transitions; // Map lưu tất cả transitions
    vector<Arc> arcs;                    // Vector lưu tất cả arcs
    Marking initial_marking;             // Trạng thái ban đầu của mạng

    // Lưu thứ tự place và transition để in theo đúng thứ tự
    vector<string> place_order;
    vector<string> transition_order;

public:
    // Thêm các phần tử vào mạng
    void add_place(const Place &p);
    void add_transition(const Transition &t);
    void add_arc(const Arc &a);

    // Getter truy cập dữ liệu
    const map<string, Place> &get_places() const { return places; }
    const map<string, Transition> &get_transitions() const { return transitions; }
    const Marking &get_initial_marking() const { return initial_marking; }
    const vector<string> &get_place_order() const { return place_order; }
    const vector<string> &get_transition_order() const { return transition_order; }
    Transition *get_transition(const string &id);

    // Các hàm tiện ích
    void print() const; // In theo format ma trận I, O, M0
};

#endif
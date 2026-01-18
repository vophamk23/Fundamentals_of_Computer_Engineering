// Header file cho Parser đọc file PNML (Petri Net Markup Language)
#ifndef PNMLPARSER_H
#define PNMLPARSER_H

#include "../core/PetriNet.h"
#include <string>

using namespace std;

// Lớp PNMLParser: chuyển đổi file XML PNML thành đối tượng PetriNet
class PNMLParser
{
public:
    // Parse file PNML và trả về đối tượng Petri Net
    // Throws runtime_error nếu file không hợp lệ
    static PetriNet parse(const string &filename);

private:
    // Hàm helper: lấy text từ tag con trong XML
    // element: con trỏ XMLElement
    // tag: tên tag cần lấy (vd: "name", "initialMarking")
    // return: text bên trong tag <text>, hoặc "" nếu không tìm thấy
    static string get_text(void *element, const char *tag);

    // Hàm helper: lấy giá trị số nguyên từ tag con
    // element: con trỏ XMLElement
    // tag: tên tag cần lấy
    // default_val: giá trị mặc định nếu không tìm thấy hoặc lỗi
    // return: số nguyên đã parse hoặc default_val
    static int get_int(void *element, const char *tag, int default_val = 0);
};

#endif
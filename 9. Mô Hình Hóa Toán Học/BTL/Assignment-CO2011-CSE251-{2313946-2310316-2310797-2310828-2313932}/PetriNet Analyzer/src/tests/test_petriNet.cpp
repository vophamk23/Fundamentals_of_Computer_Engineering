// ============================================================================
// FILE: test_runner_task1.cpp
// Đọc từ expected_petrinet.txt
// ============================================================================

#include "../parser/PNMLParser.h"
#include "../core/PetriNet.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>

using namespace std;

// ANSI color codes
#define GREEN "\033[32m"
#define RED "\033[31m"
#define YELLOW "\033[33m"
#define CYAN "\033[36m"
#define RESET "\033[0m"

string read_file(const string &filename)
{
    ifstream file(filename);
    if (!file.is_open())
    {
        cerr << "ERROR: Cannot open file: " << filename << endl;
        return "";
    }
    stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

string trim(const string &str)
{
    size_t first = str.find_first_not_of(" \n\r\t");
    if (first == string::npos)
        return "";
    size_t last = str.find_last_not_of(" \n\r\t");
    return str.substr(first, (last - first + 1));
}

string normalize_whitespace(const string &str)
{
    string result;
    bool in_space = false;

    for (char c : str)
    {
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r')
        {
            if (!in_space && !result.empty())
            {
                result += ' ';
                in_space = true;
            }
        }
        else
        {
            result += c;
            in_space = false;
        }
    }

    return trim(result);
}

bool compare_outputs(const string &expected, const string &actual)
{
    return normalize_whitespace(expected) == normalize_whitespace(actual);
}

bool run_test(int test_num, bool verbose = false, bool save_output = false)
{
    string test_dir = "tests/test_" + to_string(test_num);
    string pnml_file = test_dir + "/example.pnml";
    string expected_file = test_dir + "/expected_petrinet.txt";

    try
    {
        // Parse PNML file
        PetriNet pn = PNMLParser::parse(pnml_file);

        // Capture output
        stringstream actual_output;
        streambuf *old_cout = cout.rdbuf(actual_output.rdbuf());
        pn.print();
        cout.rdbuf(old_cout);

        // Đọc expected output
        string expected = read_file(expected_file);
        if (expected.empty())
        {
            if (verbose)
            {
                cout << RED << "ERROR: Cannot read expected file: " << expected_file << RESET << "\n";
            }
            return false;
        }

        string actual = actual_output.str();

        // Lưu actual output ra file
        if (save_output)
        {
            string actual_file = test_dir + "/actual_output.txt";
            ofstream outf(actual_file);
            if (outf.is_open())
            {
                outf << actual;
                outf.close();
            }
        }

        // So sánh
        if (compare_outputs(expected, actual))
        {
            if (verbose)
            {
                cout << "\n"
                     << GREEN << "PASSED: test_" << setfill('0') << setw(3) << test_num << RESET << "\n";
                cout << "\n━━━ EXPECTED ━━━\n"
                     << expected << "\n";
                cout << "\n━━━ ACTUAL ━━━\n"
                     << actual << "\n";
            }
            return true;
        }
        else
        {
            if (verbose)
            {
                cout << "\n"
                     << RED << "FAILED: test_" << setfill('0') << setw(3) << test_num << RESET << "\n";
                cout << "\n━━━ EXPECTED ━━━\n"
                     << expected << "\n";
                cout << "\n━━━ ACTUAL ━━━\n"
                     << actual << "\n";
            }
            return false;
        }
    }
    catch (const exception &e)
    {
        if (verbose)
        {
            cout << "\n"
                 << RED << "ERROR: test_" << setfill('0') << setw(3) << test_num << RESET << "\n";
            cout << "Exception: " << e.what() << "\n";
        }
        return false;
    }
}

int main(int argc, char *argv[])
{
    vector<int> tests_to_run;
    bool verbose = false;
    bool save_output = false;
    string output_file = "";

    // Parse arguments
    for (int i = 1; i < argc; i++)
    {
        string arg = argv[i];
        if (arg == "-v" || arg == "--verbose")
        {
            verbose = true;
        }
        else if (arg == "-s" || arg == "--save")
        {
            save_output = true;
        }
        else if (arg == "-o" || arg == "--output")
        {
            if (i + 1 < argc)
            {
                output_file = argv[i + 1];
                i++;
            }
        }
        else
        {
            try
            {
                int test_num = stoi(arg);
                tests_to_run.push_back(test_num);
            }
            catch (...)
            {
                cerr << "Invalid argument: " << arg << "\n";
            }
        }
    }

    // Mặc định chạy test 1-6
    if (tests_to_run.empty())
    {
        for (int i = 1; i <= 6; i++)
        {
            tests_to_run.push_back(i);
        }
    }

    int passed = 0;
    int failed = 0;
    int total = tests_to_run.size();
    vector<int> failed_tests;

    ofstream outfile;
    if (!output_file.empty())
    {
        outfile.open(output_file);
        if (!outfile.is_open())
        {
            cerr << "ERROR: Cannot create output file: " << output_file << "\n";
            return 1;
        }
    }

    // Header giống task3
    cout << "\033[1mcollected " << total << " items\n\n\033[0m";

    // Chạy tests
    for (size_t i = 0; i < tests_to_run.size(); i++)
    {
        int test_num = tests_to_run[i];
        bool result = run_test(test_num, verbose, save_output);

        // Tính phần trăm
        int percent = ((i + 1) * 100) / total;

        // Format giống task3: test_petriNet.cpp::test_001 PASSED        [ 16%]
        cout << "test_petriNet.cpp::test_"
             << setfill('0') << setw(3) << test_num
             << setfill(' ') << " ";

        // In status với màu
        if (result)
        {
            cout << GREEN << "PASSED" << RESET;
        }
        else
        {
            cout << RED << "FAILED" << RESET;
        }

        // In progress percentage bên phải (căn phải) với màu vàng
        cout << right << setw(60) << "[ " << YELLOW << setw(3) << percent << "%" << RESET << "]" << endl;

        // Ghi vào file nếu có
        if (outfile.is_open())
        {
            outfile << "test_petriNet.cpp::test_" << setfill('0') << setw(3) << test_num
                    << " " << (result ? "PASSED" : "FAILED") << " [" << percent << "%]\n";
        }

        if (result)
        {
            passed++;
        }
        else
        {
            failed++;
            failed_tests.push_back(test_num);
        }
    }

    if (outfile.is_open())
    {
        outfile.close();
        cout << "\n📄 Results saved to: " << output_file << "\n";
    }

    if (save_output)
    {
        cout << "📁 Actual outputs saved to: tests/test_XXX/actual_output.txt\n";
    }

    // Tổng kết giống task3 với dấu = màu cyan
    cout << "\n";

    for (int i = 0; i < 40; i++)
        cout << "\033[1m" << CYAN "=" << "\033[0m";
    cout << RESET;

    if (passed == total)
    {
        // passed in bold bright green, "in 0.14s" bright green, no bold
        cout << " " << "\033[1;92m" << passed << " passed" // bold + bright green
             << "\033[0;92m in 0.14s " << "\033[0m";       // reset bold, keep bright green
    }
    else
    {
        // passed in bold bright green
        cout << " " << "\033[1;92m" << passed << " passed"
             << "\033[0;92m" << ", "                 // reset bold, keep green
             << "\033[1;91m" << failed << " failed"  // failed in bold bright red
             << "\033[0;92m in 0.14s " << "\033[0m"; // reset bold, keep green for time
    }

    for (int i = 0; i < 40; i++)
        cout << "\033[1m" << CYAN "=" << "\033[0m";
    cout << RESET;
    cout << "\n\n";

    // In danh sách failed tests nếu có
    if (!failed_tests.empty())
    {
        cout << RED << "Failed tests: " << RESET;
        for (size_t i = 0; i < failed_tests.size(); i++)
        {
            cout << failed_tests[i];
            if (i < failed_tests.size() - 1)
                cout << ", ";
        }
        cout << "\n\n";
    }

    return (passed == total) ? 0 : 1;
}
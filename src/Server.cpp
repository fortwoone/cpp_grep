#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "matcher.hpp"

using std::cerr;
using std::cin;
using std::cout;
using std::endl;
using std::getline;
using std::runtime_error;
using std::string;
using std::unitbuf;
using std::vector;

int main(int argc, char* argv[]) {
    // Flush after every std::cout / std::cerr
    cout << unitbuf;
    cerr << unitbuf;

    // You can use print statements as follows for debugging, they'll be visible when running tests.
    cerr << "Logs from your program will appear here" << endl;

    if (argc < 3) {
        cerr << "Expected at least two arguments" << endl;
        return 1;
    }

    string flag = argv[1];

    if (argc == 5 && flag == "-r"){
        string second_flag = argv[2];
        if (second_flag != "-E"){
            cerr << "Expected second argument to be '-E' because '-r' was the first argument\n";
            return 1;
        }
        string pattern = argv[3];
        string directory = argv[4];

        try{
            if (cpp_grep::match_in_directory_recursive(directory, pattern)){
                return 0;
            }
            else{
                return 1;
            }
        }
        catch (const runtime_error& e){
            cerr << e.what() << endl;
            return 1;
        }
    }

    string pattern = argv[2];

    if (flag != "-E") {
        cerr << "Expected first argument to be '-r' or '-E'" << endl;
        return 1;
    }

    if (argc > 4){
        vector<string> file_paths;
        file_paths.reserve(argc - 3);
        for (int i = 3; i < argc; ++i){
            file_paths.emplace_back(argv[i]);
        }
        try{
            if (cpp_grep::match_in_files(file_paths, pattern)){
                return 0;
            }
            else{
                return 1;
            }
        }
        catch (const runtime_error& e){
            cerr << e.what() << endl;
            return 1;
        }
    }

    if (argc == 4){
        string file = argv[3];
        try{
            if (cpp_grep::match_in_file(file, pattern)){
                return 0;
            }
            else{
                return 1;
            }
        }
        catch (const runtime_error& e){
            cerr << e.what() << endl;
            return 1;
        }
    }

     string input_line;
     getline(cin, input_line);

     try {
         if (cpp_grep::match_pattern(input_line, pattern)) {
             return 0;
         } else {
             return 1;
         }
     } catch (const runtime_error& e) {
         cerr << e.what() << endl;
         return 1;
     }
}

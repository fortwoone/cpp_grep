#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>

#include "chr_class_handlers.hpp"
#include "chr_classes.hpp"

using std::cerr;
using std::cin;
using std::cout;
using std::endl;
using std::getline;
using std::runtime_error;
using std::string;
using std::unitbuf;
using std::unreachable;

namespace cpp_grep{

    bool match_char(char input, const vector<RegexPatternPortion>& portions, uint& pattern_index){
        if (pattern_index >= portions.size()){
            return false;
        }
        const auto& portion = portions.at(pattern_index);

        using enum ECharClass;
        switch (portion.get_char_cls()){
            case LITERAL:
                pattern_index++;
                return input == portion.get_literal();
            case DIGIT:
                pattern_index++;
                return priv::is_digit(input);
            case WORD:
                pattern_index++;
                return priv::is_word(input);
            case CHAR_GROUP:
                pattern_index++;
                if (portion.is_positive_grp()){
                    return portion.get_char_grp().contains(input);
                }
                else {
                    return !portion.get_char_grp().contains(input);
                }
            default:
                unreachable();
        }
    }

    bool match_here(const string& input_line, const vector<RegexPatternPortion>& portions, uint input_index, uint pattern_index){
        if (pattern_index >= portions.size()){
            return true;
        }

        auto portion = portions.at(pattern_index);

        if (input_index >= input_line.size()){
            return portion.get_char_cls() == ECharClass::END_ANCHOR;
        }

        if (portion.get_char_cls() == ECharClass::START_ANCHOR){
            if (input_index > 0){
                return false;
            }
            return match_here(
                input_line,
                portions,
                input_index,
                pattern_index + 1
            );
        }

        uint check_pattern_idx = pattern_index;

        if (!match_char(input_line[input_index], portions, check_pattern_idx)){
            return false;
        }

        return match_here(
            input_line,
            portions,
            input_index + 1,
            check_pattern_idx
        );
    }

    bool match_pattern(const string& input_line, const string& pattern){
        if (pattern.length() == 1) {
            return input_line.find(pattern) != string::npos;
        }
        else if (pattern == priv::DIGIT_CLS_PATTERN){
            // Handle digit class.
            return match_digit_pattern(input_line);
        }
        else if (pattern == priv::WORD_CLS_PATTERN){
            // Handle word class.
            return match_word_pattern(input_line);
        }
        else if (pattern.starts_with('[') && pattern.ends_with(']')){
            // Character group
            // Don't count the brackets while searching, they simply delimit the group itself.
            auto stripped_pattern = pattern.substr(1, pattern.size() - 2);
            if (stripped_pattern.starts_with('^')){
                // Negative character group
                return match_negative_character_grp(input_line, stripped_pattern.substr(1));
            }
            // Positive character group otherwise.
            return match_positive_character_grp(input_line, stripped_pattern);
        }
        else if (pattern.length() > 1){
            vector<RegexPatternPortion> portions = extract_patterns(pattern);
            for (size_t start = 0; start <= input_line.size(); ++start){
                if (match_here(input_line, portions, start, 0)){
                    return true;
                }
            }
            return false;
        }
        else {
            throw runtime_error("Unhandled pattern " + pattern);
        }
    }
}

int main(int argc, char* argv[]) {
    // Flush after every std::cout / std::cerr
    cout << unitbuf;
    cerr << unitbuf;

    // You can use print statements as follows for debugging, they'll be visible when running tests.
    cerr << "Logs from your program will appear here" << endl;

    if (argc != 3) {
        cerr << "Expected two arguments" << endl;
        return 1;
    }

    string flag = argv[1];
    string pattern = argv[2];

    if (flag != "-E") {
        cerr << "Expected first argument to be '-E'" << endl;
        return 1;
    }

     // Uncomment this block to pass the first stage

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

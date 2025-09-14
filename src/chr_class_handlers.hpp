//
// Created by fortwoone on 10/09/2025.
//

#pragma once

#include <algorithm>
#include <string>

namespace cpp_grep{
    using std::any_of;
    using std::all_of;
    using std::string;

    namespace priv{
        bool is_digit(char chr);
        bool is_word(char chr);
    }

    bool match_digit_pattern(const string& input_line);
    bool match_word_pattern(const string& input_line);
    bool match_positive_character_grp(const string& input_line, const string& chr_grp);
    bool match_negative_character_grp(const string& input_line, const string& chr_grp);
}

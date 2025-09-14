//
// Created by fortwoone on 14/09/2025.
//

#pragma once

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "backref_mgr.hpp"
#include "chr_class_handlers.hpp"
#include "chr_classes.hpp"

namespace cpp_grep{
    namespace fs = std::filesystem;

    using std::cerr;
    using std::cin;
    using std::cout;
    using std::endl;
    using std::find_if;
    using std::getline;
    using std::ifstream;
    using std::out_of_range;
    using std::runtime_error;
    using std::string;
    using std::unreachable;
    using std::vector;

    bool match_char(char input, const vector<RegexPatternPortion>& portions, uint& pattern_index);
    bool match_here(
        const string& input_line,
        const vector<RegexPatternPortion>& portions,
        uint input_index,
        uint pattern_index,
        BackRefManager& backref_texts,
        RegexPatternPortion* next_outside_portion = nullptr,
        uint* processed = nullptr
    );
    bool match_pattern(const string& input_line, const string& pattern);
    bool match_in_file(const string& file, const string& pattern);
    bool match_in_files(const vector<string>& files, const string& pattern);
    bool match_in_directory_recursive(const string& directory, const string& pattern);
}

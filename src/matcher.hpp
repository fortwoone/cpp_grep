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

    /**
     * @brief Matches a single character to the given pattern.
     * This function only acts on simple character classes.
     * More complex variants are handled directly in match_here instead.
     * @param input The input character to match.
     * @param portions The pattern match checks are performed on.
     * @param pattern_index The pattern portion index used.
     * @return true if a match was found, false otherwise.
     */
    bool match_char(char input, const vector<RegexPatternPortion>& portions, uint& pattern_index);

    /**
     * @brief Main matching function. This is where the bulk of the work is done.
     * @param input_line The input line a match is to be checked on.
     * @param portions The pattern used for the match check.
     * @param input_index The start index for the match.
     * @param pattern_index The index in the portion list to use.
     * @param backref_texts A reference to a backreference text manager object (see BackRefManager in backref_mgr.cpp/hpp).
     * @param next_outside_portion A pointer to the next pattern portion in the enclosing nesting level, or nullptr if there isn't one for whatever reason.
     * @param processed A pointer to an uint32_t which holds how many characters were processed during the match check.
     * @return
     */
    bool match_here(
        const string& input_line,
        const vector<RegexPatternPortion>& portions,
        uint input_index,
        uint pattern_index,
        BackRefManager& backref_texts,
        RegexPatternPortion* next_outside_portion = nullptr,
        uint* processed = nullptr
    );

    /**
     * @brief Match a pattern on a single line.
     * @param input_line The input line the pattern will be matched against.
     * @param pattern The pattern in question.
     * @return true if the pattern was matched anywhere in the line, false otherwise.
     */
    bool match_pattern(const string& input_line, const string& pattern);

    /**
     * @brief Match a pattern on a file.
     *
     * The file can be multiline.
     * All found occurrences will be printed into stdout.
     * @param file The path to the file to check.
     * @param pattern The pattern to match against.
     * @return true if a match was found at any point in the file, false otherwise.
     */
    bool match_in_file(const string& file, const string& pattern);

    /**
     * @brief Match a pattern in multiple different files.
     *
     * Any found occurrence will be printed in stdout with the file name shown before the line in question.
     * @param files A sequence containing file paths. The pattern will be checked for in all provided files.
     * @param pattern The pattern to match against.
     * @return true if a match was found at any point in any of the given files, false otherwise.
     */
    bool match_in_files(const vector<string>& files, const string& pattern);

    /**
     * @brief Match a pattern in all the files in a directory.
     *
     * The search is performed recursively.
     * Any found occurrence will be printed in stdout with the file path shown before the line in question.
     * @param directory The directory the check will be performed in.
     * @param pattern The pattern to match against.
     * @return true if a match was found at any point in any of the files present in the directory and its subdirectories, false otherwise.
     */
    bool match_in_directory_recursive(const string& directory, const string& pattern);
}

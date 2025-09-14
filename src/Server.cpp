#include <algorithm>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "backref_mgr.hpp"
#include "chr_class_handlers.hpp"
#include "chr_classes.hpp"

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
using std::unitbuf;
using std::unreachable;
using std::vector;

namespace cpp_grep{
    namespace priv{
        unordered_set<ECharClass> END_SEARCH_IF_EMPTY_AND_LAST_PAT = {
            ECharClass::ZERO_OR_ONE,
            ECharClass::PATTERN_MOST_ONE,
            ECharClass::ANY_MOST_ONE,
            ECharClass::END_ANCHOR
        };
    }

    bool match_char(char input, const vector<RegexPatternPortion>& portions, uint& pattern_index){
        if (pattern_index >= portions.size()){
            return false;
        }
        const auto& portion = portions.at(pattern_index);

        using enum ECharClass;
        switch (portion.get_char_cls()){
            case ANY:
                pattern_index++;
                return true;
            case LITERAL:
                cerr << "Portion literal: " << portion.get_literal() << "\n";
                cerr << "Input character: " << input << "\n";
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
            case END_ANCHOR:
                return false;
            default:
                unreachable();
        }
    }

    bool match_here(  // NOLINT
        const string& input_line,
        const vector<RegexPatternPortion>& portions,
        uint input_index,
        uint pattern_index,
        BackRefManager& backref_texts,
        RegexPatternPortion* next_outside_portion = nullptr,
        uint* processed = nullptr
    ){
        if (pattern_index >= portions.size()){
            return true;
        }

        const auto& portion = portions.at(pattern_index);

        if (input_index >= input_line.size()){
            return priv::END_SEARCH_IF_EMPTY_AND_LAST_PAT.contains(portion.get_char_cls());
        }

        if (portion.get_char_cls() == ECharClass::START_ANCHOR){
            if (input_index > 0){
                return false;
            }
            if (processed != nullptr){
                (*processed)++;
            }
            return match_here(
                input_line,
                portions,
                input_index,
                pattern_index + 1,
                backref_texts
            );
        }

        uint check_pattern_idx = pattern_index;

        switch (portion.get_char_cls()){
            case ECharClass::ONE_OR_MORE:
            {
                // Handle matches separately.
                uint count = 0;
                while (input_line[input_index + count] == portion.get_literal()){
                    count++;
                }
                if (!count) {
                    // Fail if none were found.
                    return false;
                }
                check_pattern_idx++;
                if (portions.size() <= check_pattern_idx){
                    return true;
                }
                else if (
                    portions.at(check_pattern_idx).get_char_cls() == ECharClass::LITERAL
                    && portions.at(check_pattern_idx).get_literal() == portion.get_literal()
                    || next_outside_portion != nullptr
                    && pattern_index == portions.size() - 1
                    && next_outside_portion->get_char_cls() == ECharClass::LITERAL
                    && next_outside_portion->get_literal() == portion.get_literal()
                ){
                    // Edge case: if a literal check is added with an identical literal
                    // to the one used in this pattern portion, move the subindex backwards by one
                    // so the literal check can still count as true.
                    // Expanded to work if this is the last literal in a pattern so constant matches are interrupted.
                    count--;
                }

                if (processed != nullptr){
                    (*processed) += count;
                }

                return match_here(
                    input_line,
                    portions,
                    input_index + count,
                    check_pattern_idx,
                    backref_texts,
                    next_outside_portion,
                    processed
                );
            }
            case ECharClass::ZERO_OR_ONE:
            {
                // Handle matches separately.
                uint count = 0;
                while (input_line[input_index + count] == portion.get_literal()){
                    count++;
                    if (count > 1){
                        // More than one occurrence? No match.
                        return false;
                    }
                }
                if (processed != nullptr){
                    (*processed) += count;
                }
                check_pattern_idx++;
                if (portions.size() <= check_pattern_idx){
                    return true;
                }
                return match_here(
                    input_line,
                    portions,
                    input_index + count,
                    check_pattern_idx,
                    backref_texts,
                    next_outside_portion,
                    processed
                );
            }
            case ECharClass::DIGIT_MOST_ONE:
            {
                uint count = 0;
                while (priv::is_digit(input_line[input_index + count])){
                    count++;
                    if (count > 1){
                        return false;
                    }
                }
                
                if (processed != nullptr){
                    (*processed) += count;
                }
                
                check_pattern_idx++;
                return match_here(
                    input_line,
                    portions,
                    input_index + count,
                    check_pattern_idx,
                    backref_texts,
                    next_outside_portion,
                    processed
                );
            }
            case ECharClass::DIGIT_LEAST_ONE:
            {
                uint count = 0;
                while (priv::is_digit(input_line[input_index + count])){
                    count++;
                }
                
                if (!count){
                    return false;
                }

                if (processed != nullptr){
                    (*processed) += count;
                }

                check_pattern_idx++;
                return match_here(
                    input_line,
                    portions,
                    input_index + count,
                    check_pattern_idx,
                    backref_texts,
                    next_outside_portion,
                    processed
                );
            }
            case ECharClass::WORD_MOST_ONE:
            {
                uint count = 0;
                while (priv::is_word(input_line[input_index + count])){
                    count++;
                    if (count > 1){
                        return false;
                    }
                }

                if (processed != nullptr){
                    (*processed) += count;
                }

                check_pattern_idx++;
                return match_here(
                    input_line,
                    portions,
                    input_index + count,
                    check_pattern_idx,
                    backref_texts,
                    next_outside_portion,
                    processed
                );
            }
            case ECharClass::WORD_LEAST_ONE:
            {
                uint count = 0;
                while (priv::is_word(input_line[input_index + count])){
                    count++;
                }

                if (!count){
                    return false;
                }

                if (processed != nullptr){
                    (*processed) += count;
                }

                check_pattern_idx++;
                return match_here(
                    input_line,
                    portions,
                    input_index + count,
                    check_pattern_idx,
                    backref_texts,
                    next_outside_portion,
                    processed
                );
            }
            case ECharClass::CHAR_GROUP_MOST_ONE:
            {
                uint i = 0;
                bool is_positive = portion.is_positive_grp();
                const auto& char_grp = portion.get_char_grp();
                if (is_positive){
                    while ((input_index + i) < input_line.size() && char_grp.contains(input_line[input_index + i])){
                        i++;
                        if (i > 1){
                            return false;
                        }
                    }
                }
                else{
                    while ((input_index + i) < input_line.size() && !char_grp.contains(input_line[input_index + i])){
                        i++;
                        if (i > 1){
                            return false;
                        }
                    }
                }

                if (processed != nullptr){
                    (*processed) += i;
                }
                check_pattern_idx++;
                return match_here(
                    input_line,
                    portions,
                    input_index + i,
                    check_pattern_idx,
                    backref_texts,
                    next_outside_portion,
                    processed
                );
            }
            case ECharClass::CHAR_GROUP_LEAST_ONE:
            {
                uint i = 0;

                bool is_positive = portion.is_positive_grp();
                const auto& char_grp = portion.get_char_grp();

                if (is_positive){
                    while ((input_index + i) < input_line.size() && char_grp.contains(input_line[input_index + i])){
                        i++;

                    }
                }
                else{
                    if (
                        next_outside_portion != nullptr
                        && pattern_index == portions.size() - 1
                        && next_outside_portion->get_char_cls() == ECharClass::LITERAL
                    ){
                        char next_chr = next_outside_portion->get_literal();
                        while (
                            (input_index + i) < input_line.size()
                            && !char_grp.contains(input_line[input_index + i])
                            && input_line[input_index + i] != next_chr
                        ){
                            i++;
                        }
                    }
                    else {
                        while (
                            (input_index + i) < input_line.size()
                            && !char_grp.contains(input_line[input_index + i])
                        ){
                            i++;
                        }
                    }
                }

                if (i < 1){
                    return false;
                }

                if (processed != nullptr){
                    (*processed) += i;
                }
                check_pattern_idx++;
                return match_here(
                    input_line,
                    portions,
                    input_index + i,
                    check_pattern_idx,
                    backref_texts,
                    next_outside_portion,
                    processed
                );
            }
            case ECharClass::ANY_LEAST_ONE:
            {
                if (pattern_index + 1 >= portions.size()){
                    return true;
                }
                else if (
                    portions.at(pattern_index + 1).get_char_cls() == ECharClass::LITERAL
                ){
                    char next_literal = portions.at(pattern_index + 1).get_literal();
                    uint count = 0;
                    while (input_line[input_index + count] != next_literal && (input_index + count) < input_line.size()){
                        count++;
                    }
                    if (!count){
                        return false;
                    }
                    check_pattern_idx++;
                    return match_here(
                        input_line,
                        portions,
                        input_index + count,
                        check_pattern_idx,
                        backref_texts,
                        next_outside_portion,
                        processed
                    );
                }
                else{
                    uint count = 0;
                    while (!match_here(input_line, portions, input_index + count + 1, pattern_index + 1, backref_texts, next_outside_portion, processed)){
                        count++;
                        if (input_index + count + 1 >= input_line.size()){
                            break;
                        }
                    }
                    check_pattern_idx++;
                    return match_here(
                        input_line,
                        portions,
                        input_index + count + 1,
                        check_pattern_idx,
                        backref_texts,
                        next_outside_portion,
                        processed
                    );
                }
            }
            case ECharClass::OR:
            {
                const auto& pattern1 = portion.get_subpattern1();
                const auto& pattern2 = portion.get_subpattern2();
                uint count_a = 0;
                uint count_b = 0;
                if (match_here(input_line, pattern1, input_index, 0, backref_texts, nullptr, &count_a)){
                    if (processed != nullptr){
                        (*processed) += count_a;
                    }
                    return true;
                }
                if (match_here(input_line, pattern2, input_index, 0, backref_texts, nullptr, &count_b)){
                    if (processed != nullptr){
                        (*processed) += count_b;
                    }
                    return true;
                }
                if (processed != nullptr){
                    (*processed)++;
                }
                return false;
            }
            case ECharClass::PATTERN:
            {
                uint count = 0;
                ubyte reserved_slot = backref_texts.reserve_first_free_slot();
                RegexPatternPortion* next_outside = nullptr;
                if (pattern_index + 1 < portions.size()){
                    next_outside = const_cast<RegexPatternPortion*>(portions.data()) + pattern_index + 1;
                }
                if (!match_here(input_line, portion.get_subpattern(), input_index, 0, backref_texts, next_outside, &count)){
                    return false;
                }
                next_outside = nullptr;
                check_pattern_idx++;
                if (processed != nullptr){
                    (*processed) += count;
                }

                // Attempt to save the matched text into the backreference list for later use.
                // Find the first empty string inside, and replace it with the text if there is indeed an empty string.
                backref_texts.set_text_at(reserved_slot, input_line.substr(input_index, count));

                input_index += count - 1;
                return match_here(
                    input_line,
                    portions,
                    input_index + 1,
                    check_pattern_idx,
                    backref_texts,
                    next_outside_portion,
                    processed
                );
            }
            case ECharClass::PATTERN_MOST_ONE:
            {
                uint count = 0;
                uint processed_chrs = 0;
                uint processed_tot = 0;
                ubyte reserved_slot = backref_texts.reserve_first_free_slot();
                while (match_here(input_line, portion.get_subpattern(), input_index + processed_tot, 0, backref_texts, next_outside_portion, &processed_chrs)){
                    count++;
                    processed_tot += processed_chrs;
                    processed_chrs = 0;
                }
                if (count > 1){
                    return false;
                }
                if (processed_chrs > 0 && processed_tot == 0){
                    processed_tot += processed_chrs;
                }
                if (processed != nullptr){
                    (*processed) += processed_tot;
                }

                // Attempt to save the matched text into the backreference list for later use.
                // Find the first empty string inside, and replace it with the text if there is indeed an empty string.
                backref_texts.set_text_at(reserved_slot, input_line.substr(input_index, processed_tot));

                check_pattern_idx++;
                return match_here(
                    input_line,
                    portions,
                    input_index + processed_tot,
                    check_pattern_idx,
                    backref_texts,
                    next_outside_portion,
                    processed
                );
            }
            case ECharClass::PATTERN_LEAST_ONE:
            {
                uint count = 0;
                uint processed_chrs = 0;
                uint processed_tot = 0;
                ubyte reserved_slot = backref_texts.reserve_first_free_slot();
                while (match_here(input_line.substr(input_index + processed_tot), portion.get_subpattern(), 0, 0, backref_texts, next_outside_portion, &processed_chrs)){
                    count++;
                    processed_tot += processed_chrs;
                    processed_chrs = 0;
                }
                if (count < 1){
                    return false;
                }
                if (processed_chrs > 0 && processed_tot == 0){
                    processed_tot += processed_chrs;
                }
                if (processed != nullptr){
                    (*processed) += processed_tot;
                }

                if (processed_tot > 0){
                    // Attempt to save the matched text into the backreference list for later use.
                    // Find the first empty string inside, and replace it with the text if there is indeed an empty string.
                    backref_texts.set_text_at(reserved_slot, input_line.substr(input_index, processed_tot));
                }
                else{
                    backref_texts.free_at(reserved_slot);
                }

                check_pattern_idx++;
                return match_here(
                    input_line,
                    portions,
                    input_index + 1 + processed_tot,
                    check_pattern_idx,
                    backref_texts,
                    next_outside_portion,
                    processed
                );
            }
            case ECharClass::BACKREFERENCE:
            {
                ubyte backref_index = portion.get_backref_index();
                const auto& txt = backref_texts.get_text_at(backref_index);

                if (input_line.substr(input_index, txt.size()) != txt){
                    return false;
                }
                if (processed != nullptr){
                    (*processed) += txt.size();
                }

                check_pattern_idx++;
                return match_here(
                    input_line,
                    portions,
                    input_index + txt.size(),
                    check_pattern_idx,
                    backref_texts,
                    next_outside_portion,
                    processed
                );
            }
            case ECharClass::BACKREF_LEAST_ONE:
            {
                ubyte backref_index = portion.get_backref_index() - 1;
                const auto& txt = backref_texts.get_text_at(backref_index);
                size_t txt_size = txt.size();
                uint count = 0;

                while (input_line.substr(input_index + count * txt_size, txt_size) == txt){
                    count++;
                }
                if (!count){
                    return false;
                }

                if (processed != nullptr){
                    (*processed) += count * txt_size;
                }

                check_pattern_idx++;
                return match_here(
                    input_line,
                    portions,
                    input_index + count * txt_size,
                    check_pattern_idx,
                    backref_texts,
                    next_outside_portion,
                    processed
                );
            }
            case ECharClass::BACKREF_MOST_ONE:
            {
                ubyte backref_index = portion.get_backref_index() - 1;
                const auto& txt = backref_texts.get_text_at(backref_index);
                size_t txt_size = txt.size();
                uint count = 0;

                while (input_line.substr(input_index + count * txt_size, txt_size) == txt){
                    count++;
                    if (count > 1){
                        return false;
                    }
                }

                if (processed != nullptr){
                    (*processed) += count * txt_size;
                }

                check_pattern_idx++;
                return match_here(
                    input_line,
                    portions,
                    input_index + count * txt_size,
                    check_pattern_idx,
                    backref_texts,
                    next_outside_portion,
                    processed
                );
            }
            default:
                break;
        }

        if (!match_char(input_line[input_index], portions, check_pattern_idx)){
            return false;
        }

        if (processed != nullptr){
            (*processed)++;
        }

        return match_here(
            input_line,
            portions,
            input_index + 1,
            check_pattern_idx,
            backref_texts,
            next_outside_portion,
            processed
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
            uint caught_grp_count = 0;
            vector<RegexPatternPortion> portions = extract_patterns(pattern, caught_grp_count);
            BackRefManager backref_texts(caught_grp_count);
            for (size_t start = 0; start <= input_line.size(); ++start){
                if (match_here(input_line, portions, start, 0, backref_texts)){
                    return true;
                }
                backref_texts.reset();
            }
            return false;
        }
        else {
            throw runtime_error("Unhandled pattern " + pattern);
        }
    }

    bool match_in_file(const string& file, const string& pattern){
        ifstream file_obj(file);
        string input_line;
        bool success = false;
        if (pattern.length() == 1) {
            while (getline(file_obj, input_line)){
                if (input_line.contains(pattern)){
                    success = true;
                    cout << input_line << "\n";
                }
            }
            return success;
        }
        else if (pattern == priv::DIGIT_CLS_PATTERN){
            // Handle digit class.
            while (getline(file_obj, input_line)){
                if (match_digit_pattern(input_line)){
                    success = true;
                    cout << input_line << "\n";
                }
            }
            return success;
        }
        else if (pattern == priv::WORD_CLS_PATTERN){
            // Handle word class.
            while (getline(file_obj, input_line)){
                if (match_word_pattern(input_line)){
                    success = true;
                    cout << input_line << "\n";
                }
            }
            return success;
        }
        else if (pattern.starts_with('[') && pattern.ends_with(']')){
            // Character group
            // Don't count the brackets while searching, they simply delimit the group itself.
            auto stripped_pattern = pattern.substr(1, pattern.size() - 2);
            if (stripped_pattern.starts_with('^')){
                // Negative character group
                while (getline(file_obj, input_line)){
                    if (match_negative_character_grp(input_line, stripped_pattern.substr(1))){
                        success = true;
                        cout << input_line << "\n";
                    }
                }
                return success;
            }
            // Positive character group otherwise.
            while (getline(file_obj, input_line)){
                if (match_positive_character_grp(input_line, stripped_pattern)){
                    success = true;
                    cout << input_line << "\n";
                }
            }
            return success;
        }
        else if (pattern.length() > 1){
            uint caught_grp_count = 0;
            vector<RegexPatternPortion> portions = extract_patterns(pattern, caught_grp_count);
            BackRefManager backref_texts(caught_grp_count);
            while (getline(file_obj, input_line)){
                for (size_t start = 0; start <= input_line.size(); ++start){
                    if (match_here(input_line, portions, start, 0, backref_texts)){
                        success = true;
                        cout << input_line << "\n";
                        backref_texts.reset();
                        break;
                    }
                    backref_texts.reset();
                }
            }
            return success;
        }
        else {
            throw runtime_error("Unhandled pattern " + pattern);
        }
    }

    bool match_in_files(const vector<string>& files, const string& pattern){
        bool success = false;
        string input_line;
        if (pattern.length() == 1){
            for (const auto& path: files){
                ifstream file_obj(path);

                while (getline(file_obj, input_line)){
                    if (input_line.contains(pattern)){
                        success = true;
                        cout << path << ":" << input_line << "\n";
                    }
                }

                file_obj.close();
            }
            return success;
        }
        else if (pattern == priv::DIGIT_CLS_PATTERN){
            // Handle digit class.
            for (const auto& path: files){
                ifstream file_obj(path);

                while (getline(file_obj, input_line)){
                    if (match_digit_pattern(input_line)){
                        success = true;
                        cout << path << ":" << input_line << "\n";
                    }
                }

                file_obj.close();
            }
            return success;
        }
        else if (pattern == priv::WORD_CLS_PATTERN){
            // Handle word class.
            for (const auto& path: files){
                ifstream file_obj(path);

                while (getline(file_obj, input_line)){
                    if (match_word_pattern(input_line)){
                        success = true;
                        cout << path << ":" << input_line << "\n";
                    }
                }

                file_obj.close();
            }
            return success;
        }
        else if (pattern.starts_with('[') && pattern.ends_with(']')){
            // Character group
            // Don't count the brackets while searching, they simply delimit the group itself.
            auto stripped_pattern = pattern.substr(1, pattern.size() - 2);
            if (stripped_pattern.starts_with('^')){
                // Negative character group
                for (const auto& path: files){
                    ifstream file_obj(path);

                    while (getline(file_obj, input_line)) {
                        if (match_negative_character_grp(input_line, stripped_pattern.substr(1))) {
                            success = true;
                            cout << path << ":" << input_line << "\n";
                        }
                    }

                    file_obj.close();
                }
                return success;
            }
            // Positive character group otherwise.
            for (const auto& path: files){
                ifstream file_obj(path);
                while (getline(file_obj, input_line)) {
                    if (match_positive_character_grp(input_line, stripped_pattern)) {
                        success = true;
                        cout << path << ":" << input_line << "\n";
                    }
                }
                file_obj.close();
            }
            return success;
        }
        else if (pattern.length() > 1){
            uint caught_grp_count = 0;
            vector<RegexPatternPortion> portions = extract_patterns(pattern, caught_grp_count);
            BackRefManager backref_texts(caught_grp_count);
            for (const auto& path: files){
                ifstream file_obj(path);
                while (getline(file_obj, input_line)) {
                    cerr << "Current line (" << path << "): " << input_line << "\n";
                    for (size_t start = 0; start <= input_line.size(); ++start) {
                        if (match_here(input_line, portions, start, 0, backref_texts)) {
                            success = true;
                            cout << path << ":" << input_line << "\n";
                            backref_texts.reset();
                            break;
                        }
                        backref_texts.reset();
                    }
                }

                file_obj.close();
                backref_texts.reset();
            }
            return success;
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

    if (argc < 3) {
        cerr << "Expected at least two arguments" << endl;
        return 1;
    }

    string flag = argv[1];
    string pattern = argv[2];

    if (flag != "-E") {
        cerr << "Expected first argument to be '-E'" << endl;
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

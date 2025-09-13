//
// Created by fortwoone on 10/09/2025.
//

#include "chr_classes.hpp"
#include <iostream>

using std::cerr;

namespace cpp_grep{
    namespace priv{
        const unordered_set<ECharClass> NONSTRUCT_CHRCLASSES = {
            ECharClass::DIGIT,
            ECharClass::WORD,
            ECharClass::START_ANCHOR,
            ECharClass::END_ANCHOR,
        };

        constexpr ubyte FLG_ONE_OR_MORE = 1;
        constexpr ubyte FLG_ZERO_OR_ONE = 2;

        bool is_nonstruct_chr_class(ECharClass cls){
            return NONSTRUCT_CHRCLASSES.contains(cls);
        }
    }

    OrCharClass::OrCharClass(const vector<RegexPatternPortion>& subpattern1, const vector<RegexPatternPortion>& subpattern2)
    : subpattern1(subpattern1), subpattern2(subpattern2){}

    PatternCharClass::PatternCharClass(const vector<RegexPatternPortion>& subpattern): subpattern(subpattern){}

    // region RegexPatternPortion: Ctors

    /**
     * Initialise a pattern portion object.
     * When matching a pattern, this will match any character.
     */
    RegexPatternPortion::RegexPatternPortion(){
        char_cls = ECharClass::ANY;
        start = 0;
        end = 1;
        cls_info = nullptr;
    }

    /**
     * Initialise a literal regex pattern portion object.
     * The span's start will be set to 0 and its end to 1.
     * @param literal The literal character to check for.
     */
    RegexPatternPortion::RegexPatternPortion(char literal){
        char_cls = ECharClass::LITERAL;
        start = 0;
        end = 1;
        cls_info = make_shared<LiteralCharClass>(literal);
    }

    /**
     * Initialise a literal (or one-or-more) regex pattern portion object.
     * The span's start will be set to 0 and its end to 1.
     * @param literal The literal character to check for.
     * @param one_or_more Set the object to "one-or-more" if this is true.
     */
    RegexPatternPortion::RegexPatternPortion(char literal, ubyte one_or_more){
        switch (one_or_more){
            case priv::FLG_ONE_OR_MORE:
                if (literal == '.'){
                    char_cls = ECharClass::ANY_LEAST_ONE;
                    break;
                }
                char_cls = ECharClass::ONE_OR_MORE;
                break;
            case priv::FLG_ZERO_OR_ONE:
                if (literal == '.'){
                    char_cls = ECharClass::ANY_MOST_ONE;
                    break;
                }
                char_cls = ECharClass::ZERO_OR_ONE;
                break;
            default:
                char_cls = ECharClass::LITERAL;
                break;
        }
        start = 0;
        end = 1;
        cls_info = make_shared<LiteralCharClass>(literal);
    }

    /**
     * Initialise a literal regex pattern portion object.
     * The span's start will be set to idx and its end to idx + 1.
     * @param literal The literal character to check for.
     * @param idx The literal's position.
     */
    RegexPatternPortion::RegexPatternPortion(char literal, uint idx){
        char_cls = ECharClass::LITERAL;
        start = idx;
        end = idx + 1;
        cls_info = make_shared<LiteralCharClass>(literal);
    }

    /**
     * Initialise a non-detailed regex pattern portion object (digit, word or start/end anchor).
     * The span's start will be set to 0 and its end to 1.
     * @param char_cls The character class to select.
     * @throw invalid_argument if the character class is not one of (digit, word, start/end anchor).
     */
    RegexPatternPortion::RegexPatternPortion(ECharClass char_cls){
        if (priv::is_nonstruct_chr_class(char_cls)){
            this->char_cls = char_cls;
            start = 0;
            end = 1;
            cls_info = nullptr;
            return;
        }
        throw invalid_argument("Cannot set char. class to this value: additional information is required.");
    }

    /**
     * Initialise a non-detailed regex pattern portion object with no special position (digit or word).
     * Match behaviour changes based on flg.
     * @param char_cls The character class to select.
     * @param flg 0 for none, 1 for "one or more", 2 for "zero or one".
     */
    RegexPatternPortion::RegexPatternPortion(ECharClass char_cls, ubyte flg){
        if (char_cls != ECharClass::DIGIT && char_cls != ECharClass::WORD){
            throw invalid_argument("Cannot set char. class to this value using this constructor: additional info is required.");
        }

        if (char_cls == ECharClass::DIGIT){
            switch (flg){
                case priv::FLG_ONE_OR_MORE:
                    this->char_cls = ECharClass::DIGIT_LEAST_ONE;
                    break;
                case priv::FLG_ZERO_OR_ONE:
                    this->char_cls = ECharClass::DIGIT_MOST_ONE;
                    break;
                default:
                    this->char_cls = char_cls;
                    break;
            }
        }
        else{
            switch (flg) {
                case priv::FLG_ONE_OR_MORE:
                    this->char_cls = ECharClass::WORD_LEAST_ONE;
                    break;
                case priv::FLG_ZERO_OR_ONE:
                    this->char_cls = ECharClass::WORD_MOST_ONE;
                    break;
                default:
                    this->char_cls = char_cls;
                    break;
            }
        }

        start = 0;
        end = 1;
        cls_info = nullptr;
    }

    /**
     * Initialise a non-detailed regex pattern portion object (digit, word or start/end anchor).
     * The span's start will be set to start and its end to start + 1.
     * @param char_cls The character class to select.
     * @param start The portion's start.
     * @throw invalid_argument if the character class is not one of (digit, word, start/end anchor).
     */
    RegexPatternPortion::RegexPatternPortion(ECharClass char_cls, uint start){
        if (priv::is_nonstruct_chr_class(char_cls)){
            this->char_cls = char_cls;
            this->start = start;
            end = start + 1;
            cls_info = nullptr;
            return;
        }
        throw invalid_argument("Cannot set char. class to this value: additional information is required.");
    }

    /**
     * Initialise a non-detailed regex pattern portion object (digit, word or start/end anchor).
     * @param char_cls The character class to select.
     * @param start The span's start.
     * @param end The span's end. Must be bigger than start.
     * @throw invalid_argument if end <= start or the character class is not one of (digit, word, start/end anchor).
     */
    RegexPatternPortion::RegexPatternPortion(ECharClass char_cls, uint start, uint end){
        if (end <= start){
            throw invalid_argument("The end cannot be smaller than the start");
        }
        if (priv::is_nonstruct_chr_class(char_cls)){
            this->char_cls = char_cls;
            this->start = start;
            this->end = end;
            cls_info = nullptr;
            return;
        }
        throw invalid_argument("Cannot set char. class to this value: additional information is required.");
    }

    /**
     * Initialise a character group regex pattern portion object.
     * The span's start will be set to 0 and its end to 1.
     * @param char_grp The character group.
     * @param positive_check Whether this group is a positive group or a negative group.
     * @throw invalid_argument if the character group is empty.
     */
    RegexPatternPortion::RegexPatternPortion(const string& char_grp, bool positive_check){
        if (char_grp.empty()){
            throw invalid_argument("Cannot provide an empty character group");
        }

        char_cls = ECharClass::CHAR_GROUP;
        start = 0;
        end = 1;
        cls_info = make_shared<GroupCharClass>(char_grp, positive_check);
    }

    /**
     * Initialise a character group regex pattern portion object.
     * The span's start will be set to 0 and its end to 1.
     * @param char_grp The character group.
     * @param positive_check Whether this group is a positive group or a negative group.
     * @param flg The modifier.
     * @throw invalid_argument if the character group is empty.
     */
    RegexPatternPortion::RegexPatternPortion(const string& char_grp, bool positive_check, ubyte flg){
        if (char_grp.empty()){
            throw invalid_argument("Cannot provide an empty character group");
        }

        switch (flg){
            case priv::FLG_ZERO_OR_ONE:
                char_cls = ECharClass::CHAR_GROUP_MOST_ONE;
                break;
            case priv::FLG_ONE_OR_MORE:
                char_cls = ECharClass::CHAR_GROUP_LEAST_ONE;
                break;
            default:
                char_cls = ECharClass::CHAR_GROUP;
                break;
        }

        start = 0;
        end = 1;
        cls_info = make_shared<GroupCharClass>(char_grp, positive_check);
    }

    /**
     * Initialise a character group regex pattern portion object.
     * The span's start will be set to start and its end to start + 1.
     * @param char_grp The character group.
     * @param positive_check Whether this group is a positive group or a negative group.
     * @param start The span's start.
     * @throw invalid_argument if the character group is empty.
     */
    RegexPatternPortion::RegexPatternPortion(const string& char_grp, bool positive_check, uint start){
        if (char_grp.empty()){
            throw invalid_argument("Cannot provide an empty character group");
        }

        char_cls = ECharClass::CHAR_GROUP;
        this->start = start;
        end = start + 1;
        cls_info = make_shared<GroupCharClass>(char_grp, positive_check);
    }

    /**
     * Initialise a character group regex pattern portion object.
     * @param char_grp The character group.
     * @param positive_check Whether this group is a positive group or a negative group.
     * @param start The span's start.
     * @param end The span's end. Must be bigger than start.
     * @throw invalid_argument if the character group is empty, or end <= start.
     */
    RegexPatternPortion::RegexPatternPortion(const string& char_grp, bool positive_check, uint start, uint end){
        if (end <= start){
            throw invalid_argument("The end cannot be smaller than the start");
        }
        if (char_grp.empty()){
            throw invalid_argument("Cannot provide an empty character group");
        }
        char_cls = ECharClass::CHAR_GROUP;
        this->start = start;
        this->end = end;
        cls_info = make_shared<GroupCharClass>(char_grp, positive_check);
    }

    /**
     * Initialise a regex pattern portion object with two subpatterns.
     * @param subpattern1 The first subpattern.
     * @param subpattern2 The second subpattern.
     * @throw invalid_argument if either subpattern is empty.
     */
    RegexPatternPortion::RegexPatternPortion(const vector<RegexPatternPortion>& subpattern1, const vector<RegexPatternPortion>& subpattern2){
        if (subpattern1.empty()){
            throw invalid_argument("Subpattern 1 cannot be empty");
        }
        if (subpattern2.empty()){
            throw invalid_argument("Subpattern 2 cannot be empty");
        }
        char_cls = ECharClass::OR;
        start = 0;
        end = 1;
        cls_info = make_shared<OrCharClass>(subpattern1, subpattern2);
    }

    /**
     * Initialise a pattern regex portion object.
     * @param subpattern The corresponding subpattern.
     * @throw invalid_argument if the subpattern is empty.
     */
    RegexPatternPortion::RegexPatternPortion(const vector<RegexPatternPortion>& subpattern){
        if (subpattern.empty()){
            throw invalid_argument("The subpattern cannot be empty");
        }

        char_cls = ECharClass::PATTERN;
        start = 0;
        end = 1;
        cls_info = make_shared<PatternCharClass>(subpattern);
    }

    RegexPatternPortion::RegexPatternPortion(const vector<RegexPatternPortion>& subpattern, ubyte flg){
        if (subpattern.empty()){
            throw invalid_argument("The subpattern cannot be empty");
        }

        switch (flg){
            case priv::FLG_ZERO_OR_ONE:
                char_cls = ECharClass::PATTERN_MOST_ONE;
                break;
            case priv::FLG_ONE_OR_MORE:
                char_cls = ECharClass::PATTERN_LEAST_ONE;
                break;
            default:
                char_cls = ECharClass::PATTERN;
                break;
        }
        start = 0;
        end = 1;
        cls_info = make_shared<PatternCharClass>(subpattern);
    }

    /**
     * Copy constructor for RegexPatternPortion.
     * @param val The original match object.
     */
    RegexPatternPortion::RegexPatternPortion(const RegexPatternPortion& val){
        char_cls = val.char_cls;
        start = val.start;
        end = val.end;
        cls_info = val.cls_info;
    }
    // endregion

    // region RegexPatternPortion : Getters
    /**
     * Get the start of the range affected by this portion object.
     * @return The start of the range affected by this portion object.
     */
    uint RegexPatternPortion::get_start() const{
        return start;
    }

    /**
     * Get the end of the range affected by this portion object.
     * The range is exclusive.
     * @return The end of the range affected by this portion object.
     */
    uint RegexPatternPortion::get_end() const{
        return end;
    }

    ECharClass RegexPatternPortion::get_char_cls() const{
        return char_cls;
    }
    // endregion

    // region RegexPatternPortion: Getters (literal char. class)
    char RegexPatternPortion::get_literal() const{
        if (char_cls != ECharClass::LITERAL && char_cls != ECharClass::ONE_OR_MORE && char_cls != ECharClass::ZERO_OR_ONE){
            // This should NEVER happen, but in case it does...
            throw logic_error("Cannot retrieve a literal from a non-literal pattern portion object");
        }
        return ((LiteralCharClass*)cls_info.get())->literal;
    }
    // endregion

    // region RegexPatternPortion : Getters (group char. class)
    string RegexPatternPortion::get_char_grp() const{
        if (char_cls != ECharClass::CHAR_GROUP && char_cls != ECharClass::CHAR_GROUP_MOST_ONE && char_cls != ECharClass::CHAR_GROUP_LEAST_ONE){
            throw logic_error("Cannot retrieve a char group string from a non-char. group pattern portion object");
        }
        return ((GroupCharClass*)cls_info.get())->char_group;
    }

    bool RegexPatternPortion::is_positive_grp() const{
        if (char_cls != ECharClass::CHAR_GROUP && char_cls != ECharClass::CHAR_GROUP_MOST_ONE && char_cls != ECharClass::CHAR_GROUP_LEAST_ONE){
            throw logic_error("Cannot retrieve a char group string from a non-char. group pattern portion object");
        }
        return ((GroupCharClass*)cls_info.get())->positive_match;
    }
    // endregion

    // region RegexPatternPortion: Getters (or char. class)
    vector<RegexPatternPortion> RegexPatternPortion::get_subpattern1() const{
        if (char_cls != ECharClass::OR){
            throw logic_error("Cannot retrieve a subpattern from a non-or pattern portion object");
        }
        return ((OrCharClass*)cls_info.get())->subpattern1;
    }

    vector<RegexPatternPortion> RegexPatternPortion::get_subpattern2() const{
        if (char_cls != ECharClass::OR){
            throw logic_error("Cannot retrieve a subpattern from a non-or pattern portion object");
        }
        return ((OrCharClass*)cls_info.get())->subpattern2;
    }
    // endregion

    // region RegexPatternPortion: Getters (pattern char. class)
    vector<RegexPatternPortion> RegexPatternPortion::get_subpattern() const{
        if (char_cls != ECharClass::PATTERN && char_cls != ECharClass::PATTERN_LEAST_ONE && char_cls != ECharClass::PATTERN_MOST_ONE){
            throw logic_error("Cannot retrieve a subpattern from a non-subpattern portion object");
        }
        return ((PatternCharClass*)cls_info.get())->subpattern;
    }
    // endregion

    vector<RegexPatternPortion> extract_patterns(const string& input){
        string temp = input;
        vector<RegexPatternPortion> ret;  // The return value.
        vector<RegexPatternPortion> backreferences; // Backreference list.
        uint idx = 0;
        size_t orig_size = input.size();
        while (!temp.empty()){
            cerr << temp << "\n";
            if (temp.size() == orig_size && temp[0] == '^'){
                ret.emplace_back(
                    ECharClass::START_ANCHOR
                );
                idx++;
                temp.erase(0, 1);
            }
            else if (temp.size() == 1 && temp[0] == '$'){
                ret.emplace_back(
                    ECharClass::END_ANCHOR
                );
                idx++;
                temp.erase(0, 1);
            }
            else if (temp.starts_with(priv::DIGIT_CLS_PATTERN)){
                // Digit class
                bool nothing_after_this = (temp.size() < 3);
                if (nothing_after_this){
                    ret.emplace_back(
                        ECharClass::DIGIT,
                        idx
                    );
                    temp.erase(0, priv::DIGIT_CLS_PATTERN.size());
                }
                else{
                    ubyte flg;
                    char following_chr = temp.at(2);
                    switch (following_chr){
                        case '+':
                            flg = priv::FLG_ONE_OR_MORE;
                            break;
                        case '?':
                            flg = priv::FLG_ZERO_OR_ONE;
                            break;
                        default:
                            flg = 0;
                            break;
                    }

                    ret.emplace_back(
                        ECharClass::DIGIT,
                        flg
                    );
                    temp.erase(0, priv::DIGIT_CLS_PATTERN.size() + (flg > 0 ? 1 : 0));
                }
                idx++;
            }
            else if (temp.starts_with(priv::WORD_CLS_PATTERN)){
                // Word class
                bool nothing_after_this = (temp.size() < 3);
                if (nothing_after_this){
                    ret.emplace_back(
                        ECharClass::WORD,
                        idx
                    );
                    temp.erase(0, priv::WORD_CLS_PATTERN.size());
                }
                else{
                    ubyte flg;
                    char following_chr = temp.at(2);
                    switch (following_chr){
                        case '+':
                            flg = priv::FLG_ONE_OR_MORE;
                            break;
                        case '?':
                            flg = priv::FLG_ZERO_OR_ONE;
                            break;
                        default:
                            flg = 0;
                            break;
                    }

                    ret.emplace_back(
                        ECharClass::WORD,
                        flg
                    );
                    temp.erase(0, priv::WORD_CLS_PATTERN.size() + (flg > 0 ? 1 : 0));
                }
                idx++;
            }
            else if (temp.starts_with('\\')){
                // Backreferences
                string nb_chars;
                uint count = 0;
                while (priv::is_digit(temp[count + 1])){
                    nb_chars.append(1, temp[count + 1]);
                    count++;
                }

                if (nb_chars.empty()){
                    // Regular backslash instead.
                    ret.emplace_back('\\');
                    temp.erase(0, 1);
                    idx++;
                    continue;
                }

                uint nb = stoi(nb_chars);
                if (backreferences.empty()){
                    throw out_of_range("There are no stored backreferences.");
                }

                // Check if the pattern is used in conjunction with "zero-or-one" or "one-or-more" flags.
                char following_chr;
                try{
                    following_chr = temp.at(count + 2);
                }
                catch (const out_of_range& exc){
                    following_chr = '\0';
                }

                const auto& backref_pat = backreferences.at(nb - 1);
                switch (following_chr){
                    case '+':
                        // One or more.
                        ret.emplace_back(backref_pat.get_subpattern(), priv::FLG_ONE_OR_MORE);
                        break;
                    case '?':
                        // Zero or one.
                        ret.emplace_back(backref_pat.get_subpattern(), priv::FLG_ZERO_OR_ONE);
                        break;
                    default:
                        ret.emplace_back(backref_pat);
                        break;
                }
                temp.erase(0, count + 1);
            }
            else if (temp.starts_with('[') && temp.contains(']')){
                // Character groups
                uint range_start = 0;
                uint range_end = temp.find(']');
                string stripped_char_grp = temp.substr(1, range_end - 1);
                bool positive_check = !stripped_char_grp.starts_with('^');
                if (!positive_check){
                    stripped_char_grp.erase(0, 1);
                }
                char following_chr;
                try{
                    following_chr = temp.at(range_end + 1);
                }
                catch (const out_of_range& exc){
                    following_chr = '\0';
                }

                ubyte flg;
                switch (following_chr){
                    case '+':
                        flg = priv::FLG_ONE_OR_MORE;
                        break;
                    case '?':
                        flg = priv::FLG_ZERO_OR_ONE;
                        break;
                    default:
                        flg = 0;
                        break;
                }

                ret.emplace_back(
                    stripped_char_grp,
                    positive_check,
                    flg
                );
                idx++;
                temp.erase(range_start, range_end + (flg > 0 ? 2 : 1));
            }
            else if (temp.starts_with('(') and temp.contains(')')){
                // Pattern group
                // Look for the end of the current pattern.
                ubyte depth = 1;
                uint index = 0;
                while (depth > 0){
                    index++;
                    if (index >= temp.size()){
                        throw invalid_argument("Missing right parenthesis to close the current expression group");
                    }
                    if (temp[index] == '('){
                        depth++;
                    }
                    else if (temp[index] == ')'){
                        depth--;
                    }
                }
                ubyte flg;
                if (index + 1 < temp.size()){
                    char next_chr = temp[index + 1];
                    switch (next_chr){
                        case '+':
                            flg = priv::FLG_ONE_OR_MORE;
                            break;
                        case '?':
                            flg = priv::FLG_ZERO_OR_ONE;
                            break;
                        default:
                            flg = 0;
                            break;
                    }
                }
                else{
                    flg = 0;
                }
                auto extracted_subpattern_string = temp.substr(1, index - 1);
                cerr << "Extracted subpattern: " << extracted_subpattern_string << "\n";
                auto extracted_subpattern = extract_patterns(extracted_subpattern_string);
                ret.emplace_back(extracted_subpattern, flg);
                backreferences.emplace_back(extracted_subpattern, flg);
                idx++;
                temp.erase(
                    0,
                    index + 1 + (flg > 0 ? 1 : 0)
                );
                cerr << "After erasure: " << temp << "\n";
            }
            else if (temp.contains('|') && !temp.contains('(')){
                string::size_type sep_pos = temp.find('|');
                string subpattern_a = temp.substr(0, sep_pos);
                string subpattern_b = temp.substr(sep_pos + 1);
                cerr << "Subpattern A: " << subpattern_a << "\n";
                cerr << "Subpattern B: " << subpattern_b << "\n";
                auto extracted_spa = extract_patterns(subpattern_a);
                auto extracted_spb = extract_patterns(subpattern_b);
                ret.emplace_back(extracted_spa, extracted_spb);
                idx++;
                temp.erase(0, subpattern_a.size() + subpattern_b.size() + 1);
            }
            else if (temp[1] == '+'){
                // One or more
                ret.emplace_back(
                    temp[0],
                    priv::FLG_ONE_OR_MORE
                );
                idx++;
                temp.erase(0, 2);
            }
            else if (temp[1] == '?'){
                // Zero or one
                ret.emplace_back(
                    temp[0],
                    priv::FLG_ZERO_OR_ONE
                );
                idx++;
                temp.erase(0, 2);
            }
            else if (temp[0] == '.'){
                // Wildcard
                ret.emplace_back();
                idx++;
                temp.erase(0, 1);
            }
            else{
                ret.emplace_back(
                    temp[0]
                );
                idx++;
                temp.erase(0, 1);
            }
        }

        return ret;
    }
}

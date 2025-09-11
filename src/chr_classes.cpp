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
            ECharClass::START_ANCHOR
        };

        bool is_nonstruct_chr_class(ECharClass cls){
            return NONSTRUCT_CHRCLASSES.contains(cls);
        }
    }

    // region RegexPatternPortion: Ctors

    /**
     * Initialise a literal regex pattern portion object.
     * The span's start will be set to 0 and its end to 1.
     * @param literal The literal character to check for.
     */
    RegexPatternPortion::RegexPatternPortion(char literal){
        char_cls = ECharClass::LITERAL;
        start = 0;
        end = 1;
        portion_info.literal_cls.literal = literal;
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
        portion_info.literal_cls.literal = literal;
    }

    /**
     * Initialise a non-detailed regex pattern portion object (digit, word or start anchor).
     * The span's start will be set to 0 and its end to 1.
     * @param char_cls The character class to select.
     * @throw invalid_argument if the character class is not one of (digit, word, start anchor).
     */
    RegexPatternPortion::RegexPatternPortion(ECharClass char_cls){
        if (priv::is_nonstruct_chr_class(char_cls)){
            this->char_cls = char_cls;
            start = 0;
            end = 1;
            return;
        }
        throw invalid_argument("Cannot set char. class to this value: additional information is required.");
    }

    /**
     * Initialise a non-detailed regex pattern portion object (digit, word or start anchor).
     * The span's start will be set to start and its end to start + 1.
     * @param char_cls The character class to select.
     * @param start The portion's start.
     * @throw invalid_argument if the character class is not one of (digit, word, start anchor).
     */
    RegexPatternPortion::RegexPatternPortion(ECharClass char_cls, uint start){
        if (priv::is_nonstruct_chr_class(char_cls)){
            this->char_cls = char_cls;
            this->start = start;
            end = start + 1;
            return;
        }
        throw invalid_argument("Cannot set char. class to this value: additional information is required.");
    }

    /**
     * Initialise a non-detailed regex pattern portion object (digit, word or start anchor).
     * @param char_cls The character class to select.
     * @param start The span's start.
     * @param end The span's end. Must be bigger than start.
     * @throw invalid_argument if end <= start or the character class is not one of (digit, word, start anchor).
     */
    RegexPatternPortion::RegexPatternPortion(ECharClass char_cls, uint start, uint end){
        if (end <= start){
            throw invalid_argument("The end cannot be smaller than the start");
        }
        if (priv::is_nonstruct_chr_class(char_cls)){
            this->char_cls = char_cls;
            this->start = start;
            this->end = end;
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
        portion_info.grp_char_cls.char_group = char_grp;
        portion_info.grp_char_cls.positive_match = positive_check;
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
        portion_info.grp_char_cls.char_group = char_grp;
        portion_info.grp_char_cls.positive_match = positive_check;
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
        portion_info.grp_char_cls.char_group = char_grp;
        portion_info.grp_char_cls.positive_match = positive_check;
    }

    /**
     * Copy constructor for RegexPatternPortion.
     * @param val The original match object.
     */
    RegexPatternPortion::RegexPatternPortion(const RegexPatternPortion& val){
        char_cls = val.char_cls;
        start = val.start;
        end = val.end;
        portion_info = val.portion_info;
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
        if (char_cls != ECharClass::LITERAL){
            // This should NEVER happen, but in case it does...
            throw logic_error("Cannot retrieve a literal from a non-literal pattern portion object");
        }
        return portion_info.literal_cls.literal;
    }
    // endregion

    // region RegexPatternPortion : Getters (group char. class)
    string RegexPatternPortion::get_char_grp() const{
        if (char_cls != ECharClass::CHAR_GROUP){
            throw logic_error("Cannot retrieve a char group string from a non-char. group pattern portion object");
        }
        return portion_info.grp_char_cls.char_group;
    }

    bool RegexPatternPortion::is_positive_grp() const{
        if (char_cls != ECharClass::CHAR_GROUP){
            throw logic_error("Cannot retrieve a char group string from a non-char. group pattern portion object");
        }
        return portion_info.grp_char_cls.positive_match;
    }
    // endregion

    vector<RegexPatternPortion> extract_patterns(const string& input){
        string temp = input;
        vector<RegexPatternPortion> ret;
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
            else if (temp.starts_with(priv::DIGIT_CLS_PATTERN)){
                ret.emplace_back(
                    ECharClass::DIGIT,
                    idx
                );
                idx++;
                temp.erase(0, priv::DIGIT_CLS_PATTERN.size());
            }
            else if (temp.starts_with(priv::WORD_CLS_PATTERN)){
                ret.emplace_back(
                    ECharClass::WORD,
                    idx
                );
                idx++;
                temp.erase(0, priv::WORD_CLS_PATTERN.size());
            }
            else if (temp.starts_with('[') && temp.contains(']')){
                uint range_start = 0;
                uint range_end = temp.find(']');
                string stripped_char_grp = temp.substr(1, range_end - 1);
                bool positive_check = stripped_char_grp.starts_with('^');
                if (!positive_check){
                    stripped_char_grp.erase(0, 1);
                }
                ret.emplace_back(
                    stripped_char_grp,
                    positive_check,
                    idx
                );
                idx++;
                temp.erase(range_start, range_end);
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

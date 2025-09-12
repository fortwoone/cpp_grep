//
// Created by fortwoone on 10/09/2025.
//

#pragma once

#include <cstdint>
#include <cstring>
#include <memory>
#include <new>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <vector>

#include "chr_class_handlers.hpp"

namespace cpp_grep{
    using ubyte = uint8_t;
    using uint = uint32_t;

    using std::invalid_argument;
    using std::logic_error;
    using std::make_shared;
    using std::memcpy;
    using std::string;
    using std::swap;
    using std::shared_ptr;
    using std::unordered_set;
    using std::vector;

    namespace priv{
        // Escaping the slash character itself so no complaints are issued about an unknown escape sequence.
        // Each pattern constant will have its "printed" value commented next to it for clarity.
        constexpr string DIGIT_CLS_PATTERN = "\\d";  // "\d"
        constexpr string WORD_CLS_PATTERN = "\\w";   // "\w"
    }

    // An enum representing all supported character classes.
    enum class ECharClass: ubyte{
        ANY,                // Match any character at that position.
        LITERAL,            // Any literal character (exact match).
        DIGIT,              // Any digit character at this position.
        WORD,               // Any word character at this position.
        CHAR_GROUP,         // Any character in a given group.
        START_ANCHOR,       // The string must start with the given expression afterwards.
        END_ANCHOR,         // The string must end with the given expression (specified before).
        ONE_OR_MORE,        // The string must contain one or more consecutive occurrences of the literal.
        ZERO_OR_ONE,        // The string must contain at most one occurrence of this literal at the current location.
        ANY_LEAST_ONE,      // At least one unspecified character.
        ANY_MOST_ONE,       // At most one unspecified character.
        OR,                 // Must validate either one of two patterns.
        PATTERN,            // The subpattern must be matched at the given location.
        PATTERN_LEAST_ONE,  // The given subpattern must be matched at least once consecutively.
        PATTERN_MOST_ONE,   // The given subpattern must match at most once.
    };

    namespace priv{
        bool is_nonstruct_chr_class(ECharClass cls);
    }

    // region Character class structs

    // TODO: potential optimisation by squashing multiple literal checks into one big sequence comparison
    //  (i.e. one class object). Need to experiment later.
    struct LiteralCharClass{
        char literal;  // The character to check for.
    };

    struct GroupCharClass{
        string char_group;       // The character group.
        bool positive_match{false};    // Positive/negative character group.
        GroupCharClass(){
            char_group = {};  // Initialise the field if using this constructor.
        };
    };

    // Forward declaration here because pattern objects need to be stored in these structs (vectors),
    // so RegexPatternPortion has to be declared.
    struct OrCharClass;
    struct PatternCharClass;
    // endregion

    union URegexPatternPortionInfo{
        LiteralCharClass literal_cls{};
        GroupCharClass grp_char_cls;
        shared_ptr<OrCharClass> or_char_cls;
        shared_ptr<PatternCharClass> pat_char_cls;

        // Using placement new so GroupCharClass can still be used
        // inside despite its non-trivial constructor.
        URegexPatternPortionInfo(){
            new(&grp_char_cls) GroupCharClass();
        }

        // Copy ctor and assignment operators are redefined
        // to avoid any problems with vectors.
        URegexPatternPortionInfo(const URegexPatternPortionInfo& val){
            memcpy(
                (void*)this,
                (void*)&val,
                sizeof(URegexPatternPortionInfo)
            );
        }

        URegexPatternPortionInfo& operator=(const URegexPatternPortionInfo& val){
            if (this != &val){
                memcpy(
                    (void*)this,
                    (void*)&val,
                    sizeof(URegexPatternPortionInfo)
                );
            }
            return *this;
        }

        ~URegexPatternPortionInfo(){}
    };


    class RegexPatternPortion{
        ECharClass char_cls;
        uint start;
        uint end;

        URegexPatternPortionInfo portion_info;

        public:
            // CTORS
            RegexPatternPortion();
            explicit RegexPatternPortion(char literal);
            RegexPatternPortion(char literal, ubyte one_or_more);
            RegexPatternPortion(char literal, uint idx);
            explicit RegexPatternPortion(ECharClass char_cls);
            RegexPatternPortion(ECharClass char_cls, uint start);
            RegexPatternPortion(ECharClass char_cls, uint start, uint end);
            RegexPatternPortion(const string& char_grp, bool positive_check);
            RegexPatternPortion(const string& char_grp, bool positive_check, uint start);
            RegexPatternPortion(const string& char_grp, bool positive_check, uint start, uint end);
            RegexPatternPortion(const vector<RegexPatternPortion>& subpattern1, const vector<RegexPatternPortion>& subpattern2);
            RegexPatternPortion(const vector<RegexPatternPortion>& subpattern);
            RegexPatternPortion(const vector<RegexPatternPortion>& subpattern, ubyte flg);

            RegexPatternPortion(const RegexPatternPortion& val);

            // GETTERS
            [[nodiscard]] uint get_start() const;
            [[nodiscard]] uint get_end() const;
            [[nodiscard]] ECharClass get_char_cls() const;

            // GETTERS (LITERAL CHAR. CLASS)
            [[nodiscard]] char get_literal() const;

            // GETTERS (GROUP CHAR. CLASS)
            [[nodiscard]] string get_char_grp() const;
            [[nodiscard]] bool is_positive_grp() const;

            // GETTERS (OR CHAR. CLASS)
            [[nodiscard]] vector<RegexPatternPortion> get_subpattern1() const;
            [[nodiscard]] vector<RegexPatternPortion> get_subpattern2() const;

            // GETTERS (PATTERN CHAR. CLASS)
            [[nodiscard]] vector<RegexPatternPortion> get_subpattern() const;

    };

    struct OrCharClass{
        vector<RegexPatternPortion> subpattern1{}, subpattern2{};

        OrCharClass() = default;
        OrCharClass(const vector<RegexPatternPortion>& subpattern1, const vector<RegexPatternPortion>& subpattern2);
    };

    struct PatternCharClass{
        vector<RegexPatternPortion> subpattern{};

        PatternCharClass() = default;
        explicit PatternCharClass(const vector<RegexPatternPortion>& subpattern);
    };

    vector<RegexPatternPortion> extract_patterns(const string& input);
}

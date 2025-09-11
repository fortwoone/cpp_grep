//
// Created by fortwoone on 10/09/2025.
//

#pragma once

#include <cstdint>
#include <cstring>
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
    using std::memcpy;
    using std::string;
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
        LITERAL,        // Any literal character (exact match).
        DIGIT,          // Any digit character at this position.
        WORD,           // Any word character at this position.
        CHAR_GROUP,     // Any character in a given group.
        START_ANCHOR,   // The string must start with the given expression afterwards.
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
        bool positive_match;    // Positive/negative character group.
        GroupCharClass(){
            char_group = {};  // Initialise the field if using this constructor.
        };
    };
    // endregion

    union URegexPatternPortionInfo{
        LiteralCharClass literal_cls;
        GroupCharClass grp_char_cls;

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

            RegexPatternPortion(char literal);
            RegexPatternPortion(char literal, uint idx);
            RegexPatternPortion(ECharClass char_cls);
            RegexPatternPortion(ECharClass char_cls, uint start);
            RegexPatternPortion(ECharClass char_cls, uint start, uint end);
            RegexPatternPortion(const string& char_grp, bool positive_check);
            RegexPatternPortion(const string& char_grp, bool positive_check, uint start);
            RegexPatternPortion(const string& char_grp, bool positive_check, uint start, uint end);

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


    };

    vector<RegexPatternPortion> extract_patterns(const string& input);
}

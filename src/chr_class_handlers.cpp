//
// Created by fortwoone on 10/09/2025.
//

#include "chr_class_handlers.hpp"

namespace cpp_grep{
    namespace priv{
        namespace constants{
            const string ASCII_CHRS = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
        }

        /**
         * Check if a character is a digit or not.
         * @param chr The input character which the digit check will be performed on.
         * @return true if the given character is a digit, false otherwise.
         */
        bool is_digit(char chr){
            return '0' <= chr && chr <= '9';
        }

        /**
         * Check if a character is in the word class or not.
         * The word class includes ASCII letters from A to Z in both uppercase and lowercase,
         * as well as digits and the underscore character.
         * @param chr The input character which the word class check will be performed on.
         * @return true if the given character matches the word class, false otherwise.
         */
        bool is_word(char chr){
            return is_digit(chr) || constants::ASCII_CHRS.contains(chr) || chr == '_';
        }
    }

    /**
     * Check if any character in a given string is a digit.
     * @param input_line The input string the check has to be performed on.
     * @return true if any character in the string is a digit, false otherwise.
     */
    bool match_digit_pattern(const string& input_line){
        auto end_iterator = input_line.end(); // Cache the past-the-end iterator to avoid calculating it twice.
        auto search_result = find_if(
            input_line.begin(),
            end_iterator,
            priv::is_digit
        );
        return search_result != end_iterator;
    }

    /**
     * Check if any character in a given string matches the regexp word class.
     * @param input_line The input string the check has to be performed on.
     * @return true if any character in the string matches the regexp word class, false otherwise.
     */
    bool match_word_pattern(const string& input_line){
        auto end_iterator = input_line.end();  // Cache the past-the-end iterator to avoid calculating it twice.
        auto search_result = find_if(
            input_line.begin(),
            end_iterator,
            priv::is_word
        );
        return search_result != end_iterator;
    }
}

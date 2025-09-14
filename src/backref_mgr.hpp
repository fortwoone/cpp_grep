//
// Created by fortwoone on 13/09/2025.
//

#pragma once

#include <algorithm>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

namespace cpp_grep{
    using ubyte = uint8_t;

    using std::find_if_not;
    using std::out_of_range;
    using std::string;
    using std::vector;

    /**
     * @brief An object holding text matched by a previous capture group.
     */
    class BackRefText{
        string txt;
        bool reserved{false};

        public:
            /**
             * Initialise an empty backreference text holder object.
             */
            BackRefText() = default;

            /**
             * @brief Check if this backreference text holder object is reserved.
             * @detail A BackRefText object is reserved when the manager requests
             * an available slot to save the result of a capture group during its first match.
             * @return true if the object is reserved, false otherwise.
             */
            [[nodiscard]] bool is_reserved() const;

            /**
             * Check if the string in this object is empty.
             * @return true if it is empty, false otherwise.
             */
            [[nodiscard]] bool empty() const;

            /**
             * Get the text stored in this object.
             * @return The text stored in this object.
             */
            [[nodiscard]] string get_text() const;


            /**
             * Mark this object as reserved.
             * As a result, back reference manager objects cannot pick it again for later use until it is freed.
             */
            void reserve();

            /**
             * Mark this object as free for use for later backreference saving.
             */
            void free();

            /**
             * Frees and empties this object.
             */
            void reset();

            /**
             * Set the text in this object.
             * @param new_text The new text value.
             */
            void change_text(const string& new_text);
    };

    /**
     * @brief An object managing text saved in backreferences.
     */
    class BackRefManager{
        vector<BackRefText> back_ref_texts;

        public:
            /**
             * Generate an empty backreference manager.
             */
            BackRefManager() = default;
            /**
             * Generate a backreference manager set up to store a given amount of backreferences.
             * @param size The amount of backreferences stored in this object.
             */
            explicit BackRefManager(ubyte size);

            [[nodiscard]] string get_text_at(ubyte index) const;
            [[nodiscard]] bool is_text_reserved_at(ubyte index) const;
            [[nodiscard]] ubyte size() const;

            [[nodiscard]] ubyte reserve_first_free_slot();
            void set_text_at(ubyte index, const string& new_text);
            void reserve_at(ubyte index);
            void free_at(ubyte index);
            void resize(ubyte new_size);
            void reset();
    };
}

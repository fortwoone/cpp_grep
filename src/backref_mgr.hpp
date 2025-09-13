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

    class BackRefText{
        string txt;
        bool reserved;

        public:
            BackRefText() = default;

            [[nodiscard]] bool is_reserved() const;
            [[nodiscard]] bool empty() const;
            [[nodiscard]] string get_text() const;

            void reserve();
            void free();
            void reset();
            void change_text(const string& new_text);
    };

    class BackRefManager{
        vector<BackRefText> back_ref_texts;

        public:
            BackRefManager() = default;
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

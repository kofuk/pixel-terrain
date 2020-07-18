/*
 * Copyright (c) 2020 Koki Fukuda
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef NBT_PULL_PARSER_HH
#define NBT_PULL_PARSER_HH

#include <cstdint>
#include <memory>
#include <stack>
#include <string>
#include <vector>

namespace pixel_terrain::nbt {
    static constexpr unsigned char TAG_END = 0;
    static constexpr unsigned char TAG_BYTE = 1;
    static constexpr unsigned char TAG_SHORT = 2;
    static constexpr unsigned char TAG_INT = 3;
    static constexpr unsigned char TAG_LONG = 4;
    static constexpr unsigned char TAG_FLOAT = 5;
    static constexpr unsigned char TAG_DOUBLE = 6;
    static constexpr unsigned char TAG_BYTE_ARRAY = 7;
    static constexpr unsigned char TAG_STRING = 8;
    static constexpr unsigned char TAG_LIST = 9;
    static constexpr unsigned char TAG_COMPOUND = 10;
    static constexpr unsigned char TAG_INT_ARRAY = 11;
    static constexpr unsigned char TAG_LONG_ARRAY = 12;

    enum class parser_event {
        DOCUMENT_START,
        TAG_START,
        DATA,
        TAG_END,
        DOCUMENT_END
    };

    class nbt_pull_parser {
        unsigned char *data;
        std::shared_ptr<unsigned char[]> sp_data;
        size_t length;
        size_t offset = 0;

        parser_event current_event = parser_event::DOCUMENT_START;
        bool tag_ended = true;
        bool end_emitted = true;
        std::stack<int> indices;
        std::stack<int> lengths;
        std::stack<unsigned char> types;
        std::stack<int> payload_types;
        std::stack<std::string> names;
        union tag_data_container {
            unsigned char byte_data;
            std::int16_t short_data;
            std::int32_t int_data;
            std::uint64_t long_data;
            float float_data;
            double double_data;
            std::string *string_data;

            tag_data_container() {}
            ~tag_data_container() {}
        };
        std::string last_tag_name;
        unsigned char last_tag_type;

        tag_data_container tag_data;

        parser_event parse_tag_header();
        void parse_array_header();
        void parse_list_header();
        parser_event parse_list_data();
        void handle_tag_end();

    public:
        nbt_pull_parser(std::shared_ptr<unsigned char[]> data,
                        const size_t length);
        nbt_pull_parser(unsigned char *data, const size_t length);

        parser_event next() noexcept(false);

        parser_event get_event_type() noexcept;
        std::string get_tag_name();
        unsigned char get_tag_type();

        unsigned char get_byte() const;
        std::int16_t get_short() const;
        std::int32_t get_int() const;
        std::uint64_t get_long() const;
        float get_float() const;
        double get_double() const;
        std::string get_string() const;
    };
} // namespace pixel_terrain::nbt

#endif

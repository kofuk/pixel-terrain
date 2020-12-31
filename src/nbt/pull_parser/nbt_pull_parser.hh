// SPDX-License-Identifier: MIT

#ifndef NBT_PULL_PARSER_HH
#define NBT_PULL_PARSER_HH

#include <cstdint>
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

            tag_data_container() = default;
            ~tag_data_container() = default;
        };
        std::string last_tag_name;
        unsigned char last_tag_type;

        tag_data_container tag_data;

        auto parse_tag_header() -> parser_event;
        void parse_array_header();
        void parse_list_header();
        auto parse_list_data() -> parser_event;
        void handle_tag_end();

    public:
        nbt_pull_parser(unsigned char *data, size_t length);

        auto next() noexcept(false) -> parser_event;

        [[nodiscard]] auto get_event_type() noexcept -> parser_event;
        [[nodiscard]] auto get_tag_name() -> std::string;
        [[nodiscard]] auto get_tag_type() -> unsigned char;

        [[nodiscard]] auto get_byte() const -> unsigned char;
        [[nodiscard]] auto get_short() const -> std::int16_t;
        [[nodiscard]] auto get_int() const -> std::int32_t;
        [[nodiscard]] auto get_long() const -> std::uint64_t;
        [[nodiscard]] auto get_float() const -> float;
        [[nodiscard]] auto get_double() const -> double;
        [[nodiscard]] auto get_string() const -> std::string;
    };
} // namespace pixel_terrain::nbt

#endif

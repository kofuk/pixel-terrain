// SPDX-License-Identifier: MIT

#include <optional>
#include <type_traits>
#include <vector>

#include "nbt.hh"
#include "nbt/tag.hh"

namespace pixel_terrain::nbt {
    auto nbt::parse_buffer(std::vector<std::uint8_t>::const_iterator first,
                           std::vector<std::uint8_t>::const_iterator last)
        -> bool {
        auto *root = tag_compound::parse_buffer(&first, last);
        if (root == nullptr || last < first) {
            delete root;
            return false;
        }

        root_ = root;

        return true;
    }

    auto nbt::from_iterator(std::vector<std::uint8_t>::const_iterator first,
                            std::vector<std::uint8_t>::const_iterator last)
        -> nbt * {
        auto *result = new nbt;
        if (!result->parse_buffer(first, last)) {
            delete result;
            return nullptr;
        }

        return result;
    }

    template <>
    auto cxx_to_nbt_type<tag_null_payload>::type() -> tag_type {
        return tag_type::TAG_END;
    }

    template <>
    auto cxx_to_nbt_type<tag_byte_payload>::type() -> tag_type {
        return tag_type::TAG_BYTE;
    }

    template <>
    auto cxx_to_nbt_type<tag_short_payload>::type() -> tag_type {
        return tag_type::TAG_SHORT;
    }

    template <>
    auto cxx_to_nbt_type<tag_int_payload>::type() -> tag_type {
        return tag_type::TAG_INT;
    }

    template <>
    auto cxx_to_nbt_type<tag_long_payload>::type() -> tag_type {
        return tag_type::TAG_LONG;
    }

    template <>
    auto cxx_to_nbt_type<tag_float_payload>::type() -> tag_type {
        return tag_type::TAG_FLOAT;
    }

    template <>
    auto cxx_to_nbt_type<tag_double_payload>::type() -> tag_type {
        return tag_type::TAG_DOUBLE;
    }

    template <>
    auto cxx_to_nbt_type<tag_byte_array_payload>::type() -> tag_type {
        return tag_type::TAG_BYTE_ARRAY;
    }

    template <>
    auto cxx_to_nbt_type<tag_string_payload>::type() -> tag_type {
        return tag_type::TAG_STRING;
    }

    template <>
    auto cxx_to_nbt_type<tag_list_payload>::type() -> tag_type {
        return tag_type::TAG_LIST;
    }

    template <>
    auto cxx_to_nbt_type<tag_compound_payload>::type() -> tag_type {
        return tag_type::TAG_COMPOUND;
    }

    template <>
    auto cxx_to_nbt_type<tag_int_array_payload>::type() -> tag_type {
        return tag_type::TAG_INT_ARRAY;
    }

    template <>
    auto cxx_to_nbt_type<tag_long_array_payload>::type() -> tag_type {
        return tag_type::TAG_LONG_ARRAY;
    }
} // namespace pixel_terrain::nbt

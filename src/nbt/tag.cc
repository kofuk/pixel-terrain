// SPDX-License-Identifier: MIT

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <functional>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "nbt/tag.hh"

namespace pixel_terrain::nbt {
    namespace {
        auto query_payload(nbt_path *path, tag_payload *payload)
            -> tag_payload *;
        auto query_array_type(nbt_path *path, tag_payload *payload)
            -> tag_payload *;
        auto query_list_type(nbt_path *path, tag_payload *payload)
            -> tag_payload *;

        std::array<bool, num_types> array_type_table{
            false, /* TAG_End */
            false, /* TAG_Byte */
            false, /* TAG_Short */
            false, /* TAG_Int */
            false, /* TAG_Long */
            false, /* TAG_Float */
            false, /* TAG_Double */
            true,  /* TAG_Byte_Array */
            false, /* TAG_String */
            false, /* TAG_List */
            false, /* TAG_Compound */
            true,  /* TAG_Int_Array */
            true,  /* TAG_Long_Array */
        };

        auto query_payload(nbt_path *path, tag_payload *payload)
            -> tag_payload * {
            if (!path->remain()) {
                return payload->clone();
            }

            enum nbt_path::pathspec::category cat =
                path->path().front().category();
            if (cat == nbt_path::pathspec::category::INDEX) {
                if (array_type_table[static_cast<std::size_t>(
                        payload->type())]) {
                    return query_array_type(path, payload);
                } else if (payload->type() == tag_type::TAG_LIST) {
                    return query_list_type(path, payload);
                }

                return nullptr;
            }

            if (cat == nbt_path::pathspec::category::KEY) {
                nbt_path::pathspec spec = path->get_one();
                if (payload->type() == tag_type::TAG_COMPOUND) {
                    auto *typed = static_cast<tag_compound_payload *>(payload);
                    auto *inner = (*typed)[spec.key()];
                    if (inner == nullptr) {
                        return nullptr;
                    }

                    return query_payload(path, inner->data());
                }
            }

            return nullptr;
        }

        auto query_array_type(nbt_path *path, tag_payload *payload)
            -> tag_payload * {
            if (!path->remain()) {
                return payload->clone();
            }

            nbt_path::pathspec spec = path->get_one();
            if (spec.container() != nbt_path::pathspec::container::ARRAY) {
                return nullptr;
            }

            if (payload->type() == tag_type::TAG_BYTE_ARRAY) {
                auto *typed_payload =
                    static_cast<tag_byte_array_payload *>(payload);
                if (spec.index() < (*typed_payload)->size()) {
                    return new tag_byte_payload((*typed_payload)[spec.index()]);
                }
            } else if (payload->type() == tag_type::TAG_INT_ARRAY) {
                auto *typed_payload =
                    static_cast<tag_int_array_payload *>(payload);
                if (spec.index() < (*typed_payload)->size()) {
                    return new tag_int_payload((*typed_payload)[spec.index()]);
                }
            } else if (payload->type() == tag_type::TAG_LONG_ARRAY) {
                auto *typed_payload =
                    static_cast<tag_long_array_payload *>(payload);
                if (spec.index() < (*typed_payload)->size()) {
                    return new tag_long_payload((*typed_payload)[spec.index()]);
                }
            }

            return nullptr;
        }

        auto query_list_type(nbt_path *path, tag_payload *payload)
            -> tag_payload * {
            if (!path->remain()) {
                return payload->clone();
            }

            nbt_path::pathspec spec = path->get_one();
            if (spec.container() != nbt_path::pathspec::container::LIST) {
                return nullptr;
            }

            auto *typed_payload = static_cast<tag_list_payload *>(payload);
            tag_type payload_type = typed_payload->payload_type();
            if ((*typed_payload)->size() <= spec.index()) {
                return nullptr;
            }

            if (payload_type == tag_type::TAG_LIST) {
                return query_list_type(path, (*typed_payload)[spec.index()]);
            }
            if (array_type_table[static_cast<std::size_t>(payload->type())]) {
                return query_array_type(path, (*typed_payload)[spec.index()]);
            }
            if (payload_type == tag_type::TAG_COMPOUND) {
                query_payload(path, (*typed_payload)[spec.index()]);
            }

            return (*typed_payload)[spec.index()]->clone();
        }
    } // namespace

    auto tag::query_internal(nbt_path *path) -> tag_payload * {
        if (path->remain()) {
            nbt_path::pathspec spec = path->get_one();
            if (spec.category() == nbt_path::pathspec::category::KEY) {
                if (spec.key() == name()) {
                    return query_payload(path, data());
                }
            }
        }
        return nullptr;
    }

    auto tag_payload::query_internal(nbt_path *path) -> tag_payload * {
        return query_payload(path, this);
    }

    namespace {
        template <class II, typename Tp>
        constexpr auto ordered_copy(II first, II last, Tp *result) -> Tp * {
            std::array<std::uint8_t, sizeof(Tp)> tmp;
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
            std::reverse_iterator<II> rev_first(last);
            std::reverse_iterator<II> rev_last(first);
            std::copy(rev_first, rev_last, tmp.begin());
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
            return std::copy(first, last, tmp.begin());
#else
#error Unsupported byte order.
#endif
            return static_cast<Tp *>(
                std::memcpy(result, tmp.data(), sizeof(Tp)));
        }
    } // namespace

    auto parse_type_and_name(tag::data_iterator *first,
                             tag::data_iterator const &last)
        -> std::optional<std::pair<tag_type, std::string>> {
        if (last <= *first + 3) {
            return std::nullopt;
        }

        std::uint8_t type_id = **first;
        ++(*first);

        if (12 < type_id) {
            return std::nullopt;
        }

        tag_type ty = static_cast<tag_type>(type_id);
        if (ty == tag_type::TAG_END) {
            return std::make_pair(tag_type::TAG_END, "");
        }

        std::uint16_t name_len;
        ordered_copy(*first, *first + sizeof(std::uint16_t), &name_len);
        (*first) += sizeof(std::uint16_t);

        if (last < *first + name_len) {
            return std::nullopt;
        }

        std::string name(*first, *first + name_len);
        (*first) += name_len;

        return std::make_pair(ty, name);
    }

    namespace factory {
        namespace {
            auto tag_end(tag::data_iterator *first,
                         tag::data_iterator const &last) -> tag_end * {
                auto *t = nbt::tag_end::parse_buffer(first, last);
                if (t == nullptr) {
                    delete t;
                    return nullptr;
                }

                return t;
            }

            auto tag_byte(tag::data_iterator *first,
                          tag::data_iterator const &last) -> tag_byte * {
                auto *t = nbt::tag_byte::parse_buffer(first, last);
                if (t == nullptr) {
                    delete t;
                    return nullptr;
                }

                return t;
            }

            auto tag_short(tag::data_iterator *first,
                           tag::data_iterator const &last) -> tag_short * {
                auto *t = nbt::tag_short::parse_buffer(first, last);
                if (t == nullptr) {
                    delete t;
                    return nullptr;
                }

                return t;
            }

            auto tag_int(tag::data_iterator *first,
                         tag::data_iterator const &last) -> tag_int * {
                auto *t = nbt::tag_int::parse_buffer(first, last);
                if (t == nullptr) {
                    delete t;
                    return nullptr;
                }

                return t;
            }

            auto tag_long(tag::data_iterator *first,
                          tag::data_iterator const &last) -> tag_long * {
                auto *t = nbt::tag_long::parse_buffer(first, last);
                if (t == nullptr) {
                    delete t;
                    return nullptr;
                }

                return t;
            }

            auto tag_float(tag::data_iterator *first,
                           tag::data_iterator const &last) -> tag_float * {
                auto *t = nbt::tag_float::parse_buffer(first, last);
                if (t == nullptr) {
                    delete t;
                    return nullptr;
                }

                return t;
            }

            auto tag_double(tag::data_iterator *first,
                            tag::data_iterator const &last) -> tag_double * {
                auto *t = nbt::tag_double::parse_buffer(first, last);
                if (t == nullptr) {
                    delete t;
                    return nullptr;
                }

                return t;
            }

            auto tag_byte_array(tag::data_iterator *first,
                                tag::data_iterator const &last)
                -> tag_byte_array * {
                auto *t = nbt::tag_byte_array::parse_buffer(first, last);
                if (t == nullptr) {
                    delete t;
                    return nullptr;
                }

                return t;
            }

            auto tag_string(tag::data_iterator *first,
                            tag::data_iterator const &last) -> tag_string * {
                auto *t = nbt::tag_string::parse_buffer(first, last);
                if (t == nullptr) {
                    delete t;
                    return nullptr;
                }

                return t;
            }

            auto tag_list(tag::data_iterator *first,
                          tag::data_iterator const &last) -> tag_list * {
                auto *t = nbt::tag_list::parse_buffer(first, last);
                if (t == nullptr) {
                    delete t;
                    return nullptr;
                }

                return t;
            }

            auto tag_compound(tag::data_iterator *first,
                              tag::data_iterator const &last)
                -> tag_compound * {
                auto *t = nbt::tag_compound::parse_buffer(first, last);
                if (t == nullptr) {
                    delete t;
                    return nullptr;
                }

                return t;
            }

            auto tag_int_array(tag::data_iterator *first,
                               tag::data_iterator const &last)
                -> tag_int_array * {
                auto *t = nbt::tag_int_array::parse_buffer(first, last);
                if (t == nullptr) {
                    delete t;
                    return nullptr;
                }

                return t;
            }

            auto tag_long_array(tag::data_iterator *first,
                                tag::data_iterator const &last)
                -> tag_long_array * {
                auto *t = nbt::tag_long_array::parse_buffer(first, last);
                if (t == nullptr) {
                    delete t;
                    return nullptr;
                }

                return t;
            }

            auto tag_end_payload(tag::data_iterator *first,
                                 tag::data_iterator const &last)
                -> tag_null_payload * {
                return new tag_null_payload;
            }

            auto tag_byte_payload(tag::data_iterator *first,
                                  tag::data_iterator const &last)
                -> tag_byte_payload * {
                if (*first <= last) {
                    auto *p = new nbt::tag_byte_payload;
                    **p = **first;
                    ++(*first);
                    return p;
                }
                return nullptr;
            }

            auto tag_short_payload(tag::data_iterator *first,
                                   tag::data_iterator const &last)
                -> tag_short_payload * {
                if (*first + sizeof(std::int16_t) <= last) {
                    auto *p = new nbt::tag_short_payload;
                    std::int16_t result;
                    ordered_copy(*first, *first + sizeof(std::int16_t),
                                 &result);
                    **p = result;
                    *first += sizeof(std::int16_t);
                    return p;
                }
                return nullptr;
            }

            auto tag_int_payload(tag::data_iterator *first,
                                 tag::data_iterator const &last)
                -> tag_int_payload * {
                if (*first + sizeof(std::int32_t) <= last) {
                    auto *p = new nbt::tag_int_payload;
                    std::int32_t result;
                    ordered_copy(*first, *first + sizeof(std::int32_t),
                                 &result);
                    **p = result;
                    *first += sizeof(std::int32_t);
                    return p;
                }
                return nullptr;
            }

            auto tag_long_payload(tag::data_iterator *first,
                                  tag::data_iterator const &last)
                -> tag_long_payload * {
                if (*first + sizeof(std::int32_t) <= last) {
                    auto *p = new nbt::tag_long_payload;
                    std::uint64_t result;
                    ordered_copy(*first, *first + sizeof(std::uint64_t),
                                 &result);
                    **p = result;
                    *first += sizeof(std::uint64_t);
                    return p;
                }
                return nullptr;
            }

            auto tag_float_payload(tag::data_iterator *first,
                                   tag::data_iterator const &last)
                -> tag_float_payload * {
                if (*first + sizeof(std::int32_t) <= last) {
                    auto *p = new nbt::tag_float_payload;
                    float result;
                    ordered_copy(*first, *first + sizeof(float), &result);
                    **p = result;
                    *first += sizeof(float);
                    return p;
                }
                return nullptr;
            }

            auto tag_double_payload(tag::data_iterator *first,
                                    tag::data_iterator const &last)
                -> tag_double_payload * {
                if (*first + sizeof(double) <= last) {
                    auto *p = new nbt::tag_double_payload;
                    double result;
                    ordered_copy(*first, *first + sizeof(double), &result);
                    **p = result;
                    *first += sizeof(double);
                    return p;
                }
                return nullptr;
            }

            auto tag_byte_array_payload(tag::data_iterator *first,
                                        tag::data_iterator const &last)
                -> tag_byte_array_payload * {
                if (last < *first + sizeof(std::uint32_t)) {
                    return nullptr;
                }

                std::uint32_t len;
                ordered_copy(*first, *first + sizeof(std::uint32_t), &len);
                *first += sizeof(std::uint32_t);

                if (last < *first + len) {
                    return nullptr;
                }

                auto *p = new nbt::tag_byte_array_payload;
                for (unsigned int i = 0; i < len; ++i) {
                    std::uint8_t item = **first;
                    ++(*first);
                    (*p)->push_back(item);
                }

                return p;
            }

            auto tag_string_payload(tag::data_iterator *first,
                                    tag::data_iterator const &last)
                -> tag_string_payload * {
                if (last < *first + sizeof(std::uint16_t)) {
                    return nullptr;
                }

                std::uint16_t len;
                ordered_copy(*first, *first + sizeof(std::uint16_t), &len);
                *first += sizeof(std::uint16_t);

                if (last < *first + len) {
                    return nullptr;
                }

                auto *p = new nbt::tag_string_payload;
                **p = std::string(*first, *first + len);
                *first += len;

                return p;
            }

            auto tag_list_payload(tag::data_iterator *first,
                                  tag::data_iterator const &last)
                -> tag_list_payload * {
                if (last < *first + 3) {
                    return nullptr;
                }
                std::uint8_t payload_type_id = **first;
                ++(*first);
                if (12 <= payload_type_id) {
                    return nullptr;
                }
                tag_type payload_type = static_cast<tag_type>(payload_type_id);

                std::uint32_t len;
                ordered_copy(*first, *first + sizeof(std::uint32_t), &len);
                *first += sizeof(std::uint32_t);

                auto *p = new nbt::tag_list_payload;
                p->set_payload_type(payload_type);

                for (unsigned int i = 0; i < len; ++i) {
                    tag_payload *item =
                        payload_factories[payload_type_id](first, last);
                    if (item == nullptr) {
                        delete p;
                        return nullptr;
                    }
                    (*p)->push_back(item);
                }

                return p;
            }

            auto tag_compound_payload(tag::data_iterator *first,
                                      tag::data_iterator const &last)
                -> tag_compound_payload * {
                auto *p = new nbt::tag_compound_payload;

                for (;;) {
                    if (last < *first) {
                        delete p;
                        return nullptr;
                    }

                    std::uint8_t tag_type_id = **first;
                    if (12 < tag_type_id) {
                        delete p;
                        return nullptr;
                    }
                    if (static_cast<tag_type>(tag_type_id) ==
                        tag_type::TAG_END) {
                        ++(*first);
                        break;
                    }

                    tag *item = tag_factories[tag_type_id](first, last);
                    if (item == nullptr) {
                        delete p;
                        return nullptr;
                    }

                    (*p)->push_back(item);
                }

                return p;
            }

            auto tag_int_array_payload(tag::data_iterator *first,
                                       tag::data_iterator const &last)
                -> tag_int_array_payload * {
                if (last <= *first + sizeof(std::uint32_t)) {
                    return nullptr;
                }

                std::uint32_t len;
                ordered_copy(*first, *first + sizeof(std::uint32_t), &len);
                *first += sizeof(std::uint32_t);

                if (last < *first + len * sizeof(std::int32_t)) {
                    return nullptr;
                }

                auto *p = new nbt::tag_int_array_payload;
                for (unsigned int i = 0; i < len; ++i) {
                    std::int32_t item;
                    ordered_copy(*first, *first + sizeof(std::int32_t), &item);
                    *first += sizeof(std::int32_t);
                    (*p)->push_back(item);
                }

                return p;
            }

            auto tag_long_array_payload(tag::data_iterator *first,
                                        tag::data_iterator const &last)
                -> tag_long_array_payload * {
                if (last < *first + sizeof(std::uint32_t)) {
                    return nullptr;
                }

                std::uint32_t len;
                ordered_copy(*first, *first + sizeof(std::uint32_t), &len);
                *first += sizeof(std::uint32_t);

                if (last < *first + len * sizeof(std::uint64_t)) {
                    return nullptr;
                }

                auto *p = new nbt::tag_long_array_payload;
                for (unsigned int i = 0; i < len; ++i) {
                    std::uint64_t item;
                    ordered_copy(*first, *first + sizeof(std::int64_t), &item);
                    *first += sizeof(std::uint64_t);
                    (*p)->push_back(item);
                }

                return p;
            }

            std::vector<tag_factory> make_tag_factory_list() {
                std::vector<tag_factory> result;

                result.push_back(tag_end);
                result.push_back(tag_byte);
                result.push_back(tag_short);
                result.push_back(tag_int);
                result.push_back(tag_long);
                result.push_back(tag_float);
                result.push_back(tag_double);
                result.push_back(tag_byte_array);
                result.push_back(tag_string);
                result.push_back(tag_list);
                result.push_back(tag_compound);
                result.push_back(tag_int_array);
                result.push_back(tag_long_array);

                return result;
            }

            std::vector<tag_payload_factory> make_tag_payload_factory_list() {
                std::vector<tag_payload_factory> result;

                result.push_back(tag_end_payload);
                result.push_back(tag_byte_payload);
                result.push_back(tag_short_payload);
                result.push_back(tag_int_payload);
                result.push_back(tag_long_payload);
                result.push_back(tag_float_payload);
                result.push_back(tag_double_payload);
                result.push_back(tag_byte_array_payload);
                result.push_back(tag_string_payload);
                result.push_back(tag_list_payload);
                result.push_back(tag_compound_payload);
                result.push_back(tag_int_array_payload);
                result.push_back(tag_long_array_payload);

                return result;
            }
        } // namespace

        std::vector<tag_factory> tag_factories = make_tag_factory_list();
        std::vector<tag_payload_factory> payload_factories =
            make_tag_payload_factory_list();
    } // namespace factory

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

// SPDX-License-Identifier: MIT

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <functional>
#include <optional>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "nbt/pull_parser/nbt_pull_parser.hh"
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

    auto parse_type_and_name(tag::data_iterator const &first,
                             tag::data_iterator const &last)
        -> std::pair<std::optional<std::pair<tag_type, std::string>>,
                     tag::data_iterator> {
        if (last <= first + 3) {
            return std::make_pair(std::nullopt, first);
        }

        std::uint8_t type_id = *first;
        tag::data_iterator itr = first + 1;

        if (num_types < static_cast<std::size_t>(type_id)) {
            return std::make_pair(std::nullopt, first);
        }

        tag_type ty = static_cast<tag_type>(type_id);
        if (ty == tag_type::TAG_END) {
            return std::make_pair(std::make_pair(tag_type::TAG_END, ""), itr);
        }

        std::uint16_t name_len;
        ordered_copy(itr, itr + sizeof(std::uint16_t), &name_len);
        itr += sizeof(std::uint16_t);

        if (last < itr + name_len) {
            return std::make_pair(std::nullopt, first);
        }

        std::string name(itr, itr + name_len);
        itr += name_len;

        return std::make_pair(std::make_pair(ty, name), itr);
    }

    namespace factory {
        namespace {
            template <tag_type TT>
            auto make_tag(tag::data_iterator const &first,
                          tag::data_iterator const &last)
                -> std::pair<tag *, tag::data_iterator> {
                auto [t, itr] = nbt::basic_tag<TT>::parse_buffer(first, last);
                if (t == nullptr) {
                    delete t;
                    return std::make_pair(nullptr, first);
                }

                return std::make_pair(t, itr);
            }

            auto make_tag_end_payload(tag::data_iterator const &first,
                                      tag::data_iterator const &last)
                -> std::pair<tag_payload *, tag::data_iterator> {
                return std::make_pair(new tag_null_payload, first);
            }

            auto make_tag_byte_payload(tag::data_iterator const &first,
                                       tag::data_iterator const &last)
                -> std::pair<tag_payload *, tag::data_iterator> {
                if (first <= last) {
                    auto *p = new nbt::tag_byte_payload;
                    **p = *first;
                    return std::make_pair(p, first + 1);
                }
                return std::make_pair(nullptr, first);
            }

            auto make_tag_short_payload(tag::data_iterator const &first,
                                        tag::data_iterator const &last)
                -> std::pair<tag_payload *, tag::data_iterator> {
                if (first + sizeof(std::int16_t) <= last) {
                    auto *p = new nbt::tag_short_payload;
                    std::int16_t result;
                    ordered_copy(first, first + sizeof(std::int16_t), &result);
                    **p = result;
                    return std::make_pair(p, first + sizeof(std::int16_t));
                }
                return std::make_pair(nullptr, first);
            }

            auto make_tag_int_payload(tag::data_iterator const &first,
                                      tag::data_iterator const &last)
                -> std::pair<tag_payload *, tag::data_iterator> {
                if (first + sizeof(std::int32_t) <= last) {
                    auto *p = new nbt::tag_int_payload;
                    std::int32_t result;
                    ordered_copy(first, first + sizeof(std::int32_t), &result);
                    **p = result;
                    return std::make_pair(p, first + sizeof(std::int32_t));
                }
                return std::make_pair(nullptr, first);
            }

            auto make_tag_long_payload(tag::data_iterator const &first,
                                       tag::data_iterator const &last)
                -> std::pair<tag_payload *, tag::data_iterator> {
                if (first + sizeof(std::int32_t) <= last) {
                    auto *p = new nbt::tag_long_payload;
                    std::uint64_t result;
                    ordered_copy(first, first + sizeof(std::uint64_t), &result);
                    **p = result;
                    return std::make_pair(p, first + sizeof(std::uint64_t));
                }
                return std::make_pair(nullptr, first);
            }

            auto make_tag_float_payload(tag::data_iterator const &first,
                                        tag::data_iterator const &last)
                -> std::pair<tag_payload *, tag::data_iterator> {
                if (first + sizeof(std::int32_t) <= last) {
                    auto *p = new nbt::tag_float_payload;
                    float result;
                    ordered_copy(first, first + sizeof(float), &result);
                    **p = result;
                    return std::make_pair(p, first + sizeof(float));
                }
                return std::make_pair(nullptr, first);
            }

            auto make_tag_double_payload(tag::data_iterator const &first,
                                         tag::data_iterator const &last)
                -> std::pair<tag_payload *, tag::data_iterator> {
                if (first + sizeof(double) <= last) {
                    auto *p = new nbt::tag_double_payload;
                    double result;
                    ordered_copy(first, first + sizeof(double), &result);
                    **p = result;
                    return std::make_pair(p, first + sizeof(double));
                }
                return std::make_pair(nullptr, first);
            }

            auto make_tag_byte_array_payload(tag::data_iterator const &first,
                                             tag::data_iterator const &last)
                -> std::pair<tag_payload *, tag::data_iterator> {
                if (last < first + sizeof(std::uint32_t)) {
                    return std::make_pair(nullptr, first);
                }

                std::uint32_t len;
                ordered_copy(first, first + sizeof(std::uint32_t), &len);
                tag::data_iterator itr = first + sizeof(std::uint32_t);

                if (last < itr + len) {
                    return std::make_pair(nullptr, first);
                }

                auto *p = new nbt::tag_byte_array_payload;
                for (unsigned int i = 0; i < len; ++i) {
                    std::uint8_t item = *itr;
                    ++itr;
                    (*p)->push_back(item);
                }

                return std::make_pair(p, itr);
            }

            auto make_tag_string_payload(tag::data_iterator const &first,
                                         tag::data_iterator const &last)
                -> std::pair<tag_payload *, tag::data_iterator> {
                if (last < first + sizeof(std::uint16_t)) {
                    return std::make_pair(nullptr, first);
                }

                std::uint16_t len;
                ordered_copy(first, first + sizeof(std::uint16_t), &len);
                tag::data_iterator itr = first + sizeof(std::uint16_t);

                if (last < itr + len) {
                    return std::make_pair(nullptr, first);
                }

                auto *p = new nbt::tag_string_payload;
                **p = std::string(itr, itr + len);

                return std::make_pair(p, itr + len);
            }

            auto make_tag_list_payload(tag::data_iterator const &first,
                                       tag::data_iterator const &last)
                -> std::pair<tag_payload *, tag::data_iterator> {
                if (last < first + 3) {
                    return std::make_pair(nullptr, first);
                }
                std::uint8_t payload_type_id = *first;
                tag::data_iterator itr = first + 1;
                if (12 <= payload_type_id) {
                    return std::make_pair(nullptr, first);
                }
                tag_type payload_type = static_cast<tag_type>(payload_type_id);

                std::uint32_t len;
                ordered_copy(itr, itr + sizeof(std::uint32_t), &len);
                itr += sizeof(std::uint32_t);

                auto *p = new nbt::tag_list_payload;
                p->set_payload_type(payload_type);

                for (unsigned int i = 0; i < len; ++i) {
                    auto [item, out_itr] =
                        payload_factories[payload_type_id](itr, last);
                    if (item == nullptr) {
                        delete p;
                        return std::make_pair(nullptr, first);
                    }
                    (*p)->push_back(item);
                    itr = out_itr;
                }

                return std::make_pair(p, itr);
            }

            auto make_tag_compound_payload(tag::data_iterator const &first,
                                           tag::data_iterator const &last)
                -> std::pair<tag_payload *, tag::data_iterator> {
                auto *p = new nbt::tag_compound_payload;

                tag::data_iterator itr = first;

                for (;;) {
                    if (last < itr) {
                        delete p;
                        return std::make_pair(nullptr, first);
                    }

                    std::uint8_t tag_type_id = *itr;
                    if (12 < tag_type_id) {
                        delete p;
                        return std::make_pair(nullptr, first);
                    }
                    if (static_cast<tag_type>(tag_type_id) ==
                        tag_type::TAG_END) {
                        ++itr;
                        break;
                    }

                    auto [item, out_itr] =
                        tag_factories[tag_type_id](itr, last);
                    if (item == nullptr) {
                        delete p;
                        return std::make_pair(nullptr, first);
                    }

                    (*p)->push_back(item);
                    itr = out_itr;
                }

                return std::make_pair(p, itr);
            }

            auto make_tag_int_array_payload(tag::data_iterator const &first,
                                            tag::data_iterator const &last)
                -> std::pair<tag_payload *, tag::data_iterator> {
                if (last <= first + sizeof(std::uint32_t)) {
                    return std::make_pair(nullptr, first);
                }

                std::uint32_t len;
                ordered_copy(first, first + sizeof(std::uint32_t), &len);
                tag::data_iterator itr = first + sizeof(std::uint32_t);

                if (last < first + len * sizeof(std::int32_t)) {
                    return std::make_pair(nullptr, first);
                }

                auto *p = new nbt::tag_int_array_payload;
                for (unsigned int i = 0; i < len; ++i) {
                    std::int32_t item;
                    ordered_copy(itr, itr + sizeof(std::int32_t), &item);
                    itr += sizeof(std::int32_t);
                    (*p)->push_back(item);
                }

                return std::make_pair(p, itr);
            }

            auto make_tag_long_array_payload(tag::data_iterator const &first,
                                             tag::data_iterator const &last)
                -> std::pair<tag_payload *, tag::data_iterator> {
                if (last < first + sizeof(std::uint32_t)) {
                    return std::make_pair(nullptr, first);
                }

                std::uint32_t len;
                ordered_copy(first, first + sizeof(std::uint32_t), &len);
                tag::data_iterator itr = first + sizeof(std::uint32_t);

                if (last < itr + len * sizeof(std::uint64_t)) {
                    return std::make_pair(nullptr, first);
                }

                auto *p = new nbt::tag_long_array_payload;
                for (unsigned int i = 0; i < len; ++i) {
                    std::uint64_t item;
                    ordered_copy(itr, itr + sizeof(std::int64_t), &item);
                    itr += sizeof(std::uint64_t);
                    (*p)->push_back(item);
                }

                return std::make_pair(p, itr);
            }
        } // namespace

        // NOLINTNEXTLINE
        std::pair<tag *, tag::data_iterator> (*tag_factories[])(
            tag::data_iterator const &first, tag::data_iterator const &last) = {
            &make_tag<tag_type::TAG_END>,
            &make_tag<tag_type::TAG_BYTE>,
            &make_tag<tag_type::TAG_SHORT>,
            &make_tag<tag_type::TAG_INT>,
            &make_tag<tag_type::TAG_LONG>,
            &make_tag<tag_type::TAG_FLOAT>,
            &make_tag<tag_type::TAG_DOUBLE>,
            &make_tag<tag_type::TAG_BYTE_ARRAY>,
            &make_tag<tag_type::TAG_STRING>,
            &make_tag<tag_type::TAG_LIST>,
            &make_tag<tag_type::TAG_COMPOUND>,
            &make_tag<tag_type::TAG_INT_ARRAY>,
            &make_tag<tag_type::TAG_LONG_ARRAY>,
        };

        // NOLINTNEXTLINE
        std::pair<tag_payload *, tag::data_iterator> (*payload_factories[])(
            tag::data_iterator const &first, tag::data_iterator const &last) = {
            &make_tag_end_payload,        &make_tag_byte_payload,
            &make_tag_short_payload,      &make_tag_int_payload,
            &make_tag_long_payload,       &make_tag_float_payload,
            &make_tag_double_payload,     &make_tag_byte_array_payload,
            &make_tag_string_payload,     &make_tag_list_payload,
            &make_tag_compound_payload,   &make_tag_int_array_payload,
            &make_tag_long_array_payload,
        };
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

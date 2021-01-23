// SPDX-License-Identifier: MIT

#ifndef NBT_TAG_HH
#define NBT_TAG_HH

#include "nbt/nbt-path.hh"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <iterator>
#include <optional>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

namespace pixel_terrain::nbt {
    enum class tag_type : std::uint8_t {
        TAG_END,
        TAG_BYTE,
        TAG_SHORT,
        TAG_INT,
        TAG_LONG,
        TAG_FLOAT,
        TAG_DOUBLE,
        TAG_BYTE_ARRAY,
        TAG_STRING,
        TAG_LIST,
        TAG_COMPOUND,
        TAG_INT_ARRAY,
        TAG_LONG_ARRAY,
    };

    class tag_payload {
    protected:
        tag_type type_;

    public:
        tag_payload() = default;
        virtual ~tag_payload() = default;

        tag_payload(tag_payload const &) = delete;
        tag_payload(tag_payload &&) = delete;

        auto operator=(tag_payload const &) -> tag_payload & = delete;
        auto operator=(tag_payload &&) -> tag_payload & = delete;

        auto type() const -> tag_type { return type_; }

        virtual auto is_array_type() -> bool = 0;
        virtual auto has_inner_tag() -> bool = 0;
    };

    class tag_null_payload : public tag_payload {
    public:
        tag_null_payload() { type_ = tag_type::TAG_END; }

        static constexpr auto type() -> tag_type { return tag_type::TAG_END; }
        virtual auto is_array_type() -> bool override { return false; }
        virtual auto has_inner_tag() -> bool override { return false; }
    };

    class tag {
    public:
        using data_iterator = std::vector<std::uint8_t>::const_iterator;

    protected:
        std::string name_;

    public:
        virtual ~tag() = default;

        [[nodiscard]] virtual auto data() -> tag_payload * = 0;

        [[nodiscard]] auto type() -> tag_type { return data()->type(); }

        [[nodiscard]] auto name() const -> std::string const & { return name_; }

        [[nodiscard]] auto query(nbt_path *path) -> tag_payload *;
    };

    template <class T>
    concept TagPayloadType = requires(T *obj) {
        static_cast<tag_payload *>(obj);
    };

    namespace factory {
        using tag_payload_factory = std::function<tag_payload *(
            tag::data_iterator *, tag::data_iterator const &)>;
        using tag_factory = std::function<tag *(tag::data_iterator *,
                                                tag::data_iterator const &)>;

        extern std::vector<tag_factory> tag_factories;
        extern std::vector<tag_payload_factory> payload_factories;
    } // namespace factory

    class tag_byte_payload : public tag_payload {
        std::uint8_t data_;

    public:
        tag_byte_payload() { type_ = tag_type::TAG_BYTE; }

        tag_byte_payload(std::uint8_t n) {
            type_ = tag_type::TAG_BYTE;
            data_ = n;
        }

        [[nodiscard]] auto operator*() -> std::uint8_t & { return data_; }

        static constexpr auto tag_type() -> tag_type {
            return tag_type::TAG_BYTE;
        }

        auto is_array_type() -> bool override { return false; }

        auto has_inner_tag() -> bool override { return false; }
    };

    class tag_short_payload : public tag_payload {
        std::int16_t data_;

    public:
        tag_short_payload() { type_ = tag_type::TAG_SHORT; }

        [[nodiscard]] auto operator*() -> std::int16_t & { return data_; }

        static constexpr auto tag_type() -> tag_type {
            return tag_type::TAG_SHORT;
        }

        auto is_array_type() -> bool override { return false; }

        auto has_inner_tag() -> bool override { return false; }
    };

    class tag_int_payload : public tag_payload {
        std::int32_t data_;

    public:
        tag_int_payload() { type_ = tag_type::TAG_INT; }

        tag_int_payload(std::int32_t n) {
            type_ = tag_type::TAG_INT;
            data_ = n;
        }

        [[nodiscard]] auto operator*() -> std::int32_t & { return data_; }

        static constexpr auto tag_type() -> tag_type {
            return tag_type::TAG_INT;
        }

        auto is_array_type() -> bool override { return false; }

        auto has_inner_tag() -> bool override { return false; }
    };

    class tag_long_payload : public tag_payload {
        std::uint64_t data_;

    public:
        tag_long_payload() { type_ = tag_type::TAG_LONG; }

        tag_long_payload(std::uint64_t n) {
            type_ = tag_type::TAG_LONG;
            data_ = n;
        }

        [[nodiscard]] auto operator*() -> std::uint64_t & { return data_; }

        static constexpr auto tag_type() -> tag_type {
            return tag_type::TAG_LONG;
        }

        auto is_array_type() -> bool override { return false; }

        auto has_inner_tag() -> bool override { return false; }
    };

    class tag_float_payload : public tag_payload {
        float data_;

    public:
        tag_float_payload() { type_ = tag_type::TAG_FLOAT; }

        [[nodiscard]] auto operator*() -> float & { return data_; }

        static constexpr auto tag_type() -> tag_type {
            return tag_type::TAG_FLOAT;
        }

        auto is_array_type() -> bool override { return false; }

        auto has_inner_tag() -> bool override { return false; }
    };

    class tag_double_payload : public tag_payload {
        double data_;

    public:
        tag_double_payload() { type_ = tag_type::TAG_DOUBLE; }

        [[nodiscard]] auto operator*() -> double & { return data_; }

        static constexpr auto tag_type() -> tag_type {
            return tag_type::TAG_DOUBLE;
        }

        auto is_array_type() -> bool override { return false; }

        auto has_inner_tag() -> bool override { return false; }
    };

    class tag_byte_array_payload : public tag_payload {
        std::vector<std::uint8_t> data_;

    public:
        tag_byte_array_payload() { type_ = tag_type::TAG_BYTE_ARRAY; }

        [[nodiscard]] auto operator*() -> std::vector<std::uint8_t> & {
            return data_;
        }

        auto operator->() -> std::vector<std::uint8_t> * { return &data_; }

        [[nodiscard]] auto operator[](std::size_t index) -> std::uint8_t & {
            return data_[index];
        }

        static constexpr auto tag_type() -> tag_type {
            return tag_type::TAG_BYTE_ARRAY;
        }

        auto is_array_type() -> bool override { return true; }

        auto has_inner_tag() -> bool override { return false; }
    };

    class tag_string_payload : public tag_payload {
        std::string data_;

    public:
        tag_string_payload() { type_ = tag_type::TAG_STRING; }

        [[nodiscard]] auto operator*() -> std::string & { return data_; }

        static constexpr auto tag_type() -> tag_type {
            return tag_type::TAG_STRING;
        }

        auto is_array_type() -> bool override { return false; }

        auto has_inner_tag() -> bool override { return false; }
    };

    class tag_list_payload : public tag_payload {
        std::vector<tag_payload *> data_;
        tag_type payload_type_;

    public:
        tag_list_payload() { type_ = tag_type::TAG_LIST; }
        ~tag_list_payload() override {
            for (tag_payload *t : data_) {
                delete t;
            }
        }

        [[nodiscard]] auto operator*() -> std::vector<tag_payload *> & {
            return data_;
        }

        auto operator->() -> std::vector<tag_payload *> * { return &data_; }

        template <TagPayloadType Tp>
        [[nodiscard]] auto get(std::size_t index) -> Tp * {
            return static_cast<Tp *>(data_[index]);
        }

        [[nodiscard]] auto operator[](std::size_t index) -> tag_payload * {
            return data_[index];
        }

        [[nodiscard]] auto payload_type() const -> tag_type {
            return payload_type_;
        }

        void set_payload_type(tag_type const &type) { payload_type_ = type; }

        static constexpr auto type() -> tag_type { return tag_type::TAG_LIST; }

        auto is_array_type() -> bool override { return false; }

        auto has_inner_tag() -> bool override {
            return payload_type() == tag_type::TAG_COMPOUND ||
                   payload_type() == tag_type::TAG_LIST ||
                   payload_type() == tag_type::TAG_BYTE_ARRAY ||
                   payload_type() == tag_type::TAG_INT_ARRAY ||
                   payload_type() == tag_type::TAG_LONG_ARRAY;
        }
    };

    class tag_compound_payload : public tag_payload {
        std::vector<tag *> data_;

    public:
        tag_compound_payload() { type_ = tag_type::TAG_COMPOUND; }

        ~tag_compound_payload() override {
            for (tag *t : data_) {
                delete t;
            }
        }

        [[nodiscard]] auto operator*() -> std::vector<tag *> & { return data_; }

        auto operator->() -> std::vector<tag *> * { return &data_; }

        [[nodiscard]] auto operator[](std::string const &key) -> tag * {
            auto found = std::find_if(
                data_.begin(), data_.end(),
                [&key](tag const *t) -> bool { return t->name() == key; });

            if (found == data_.end()) return nullptr;

            return *found;
        }

        [[nodiscard]] auto operator[](std::size_t index) -> tag * {
            return data_[index];
        }

        template <class Tp, class = std::is_convertible<Tp, tag>>
        auto get_typed(std::string const &key) -> Tp & {
            return *static_cast<Tp *>(operator[](key));
        }

        template <class Tp, class = std::is_convertible<Tp, tag>>
        auto get_typed(std::size_t index) -> Tp & {
            return *static_cast<Tp *>(operator[](index));
        }

        static constexpr auto tag_type() -> tag_type {
            return tag_type::TAG_COMPOUND;
        }

        auto is_array_type() -> bool override { return false; }

        auto has_inner_tag() -> bool override { return true; }
    };

    class tag_int_array_payload : public tag_payload {
        std::vector<std::int32_t> data_;

    public:
        tag_int_array_payload() { type_ = tag_type::TAG_INT_ARRAY; }

        [[nodiscard]] auto operator*() -> std::vector<std::int32_t> & {
            return data_;
        }

        auto operator->() -> std::vector<std::int32_t> * { return &data_; }

        [[nodiscard]] auto operator[](std::size_t index) -> std::int32_t & {
            return data_[index];
        }

        static constexpr auto tag_type() -> tag_type {
            return tag_type::TAG_INT_ARRAY;
        }

        auto is_array_type() -> bool override { return true; }

        auto has_inner_tag() -> bool override { return false; }
    };

    class tag_long_array_payload : public tag_payload {
        std::vector<std::uint64_t> data_;

    public:
        tag_long_array_payload() { type_ = tag_type::TAG_LONG_ARRAY; }

        [[nodiscard]] auto operator*() -> std::vector<std::uint64_t> & {
            return data_;
        }

        auto operator->() -> std::vector<std::uint64_t> * { return &data_; }

        [[nodiscard]] auto operator[](std::size_t index) -> std::uint64_t & {
            return data_[index];
        }

        static constexpr auto tag_type() -> tag_type {
            return tag_type::TAG_LONG_ARRAY;
        }

        auto is_array_type() -> bool override { return true; }

        auto has_inner_tag() -> bool override { return false; }
    };

    auto parse_type_and_name(tag::data_iterator *first,
                             tag::data_iterator const &last)
        -> std::optional<std::pair<tag_type, std::string>>;

    template <tag_type TagType>
    class basic_tag : public tag {
        tag_payload *data_ = nullptr;

        basic_tag() {}

    public:
        ~basic_tag() { delete data_; }

        static auto parse_buffer(data_iterator *first,
                                 data_iterator const &last)
            -> basic_tag<TagType> * {
            auto hdr = parse_type_and_name(first, last);
            if (!hdr) {
                return nullptr;
            }

            auto [type, name] = *hdr;
            if (type != TagType) {
                return nullptr;
            }

            auto *result = new basic_tag<TagType>;

            result->name_ = name;

            tag_payload *p =
                factory::payload_factories[static_cast<std::uint8_t>(TagType)](
                    first, last);
            if (p == nullptr) {
                delete result;
                return nullptr;
            }

            result->data_ = p;

            return result;
        }

        auto data() -> tag_payload * override { return data_; }

        template <TagPayloadType Tp>
        auto typed_data() -> Tp * {
            return static_cast<Tp *>(data_);
        }
    };

    using tag_end = basic_tag<tag_type::TAG_END>;
    using tag_byte = basic_tag<tag_type::TAG_BYTE>;
    using tag_short = basic_tag<tag_type::TAG_SHORT>;
    using tag_int = basic_tag<tag_type::TAG_INT>;
    using tag_long = basic_tag<tag_type::TAG_LONG>;
    using tag_float = basic_tag<tag_type::TAG_FLOAT>;
    using tag_double = basic_tag<tag_type::TAG_DOUBLE>;
    using tag_byte_array = basic_tag<tag_type::TAG_BYTE_ARRAY>;
    using tag_string = basic_tag<tag_type::TAG_STRING>;
    using tag_list = basic_tag<tag_type::TAG_LIST>;
    using tag_compound = basic_tag<tag_type::TAG_COMPOUND>;
    using tag_int_array = basic_tag<tag_type::TAG_INT_ARRAY>;
    using tag_long_array = basic_tag<tag_type::TAG_LONG_ARRAY>;
} // namespace pixel_terrain::nbt

#endif

// SPDX-License-Identifier: MIT

#ifndef NBT_TAG_HH
#define NBT_TAG_HH

#include <algorithm>
#include <cstdint>
#include <functional>
#include <iterator>
#include <optional>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

#include "nbt/nbt-path.hh"

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
    inline constexpr std::size_t num_types = 13;

    class tag_payload;

    template <class Tp, class = std::is_convertible<Tp, tag_payload>>
    struct cxx_to_nbt_type {
        static auto type() -> tag_type { return tag_type::TAG_END; }
    };

    class tag_payload {
        auto query_internal(nbt_path *path) -> tag_payload *;

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

        virtual auto clone() const -> tag_payload * = 0;

        template <class Tp, class = std::is_convertible<Tp, tag_payload>>
        [[nodiscard]] auto query(nbt_path path) -> Tp * {
            tag_payload *result = query_internal(&path);

            if (result == nullptr ||
                cxx_to_nbt_type<Tp>::type() != result->type()) {
                return nullptr;
            }

            return static_cast<Tp *>(result);
        }
    };

    class tag_null_payload : public tag_payload {
    public:
        tag_null_payload() { type_ = tag_type::TAG_END; }

        static constexpr auto type() -> tag_type { return tag_type::TAG_END; }

        [[nodiscard]] auto clone() const -> tag_payload * override {
            return new tag_null_payload;
        }
    };

    class tag {
    public:
        using data_iterator = std::vector<std::uint8_t>::const_iterator;

    protected:
        std::string name_;

        auto query_internal(nbt_path *path) -> tag_payload *;

    public:
        virtual ~tag() = default;

        [[nodiscard]] virtual auto data() -> tag_payload * = 0;

        [[nodiscard]] auto type() -> tag_type { return data()->type(); }
        [[nodiscard]] auto name() const -> std::string const & { return name_; }

        [[nodiscard]] virtual auto clone() const -> tag * = 0;
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
        tag_byte_payload(decltype(data_) b) {
            type_ = tag_type::TAG_BYTE;
            data_ = b;
        }

        [[nodiscard]] auto operator*() -> std::uint8_t & { return data_; }

        static constexpr auto tag_type() -> tag_type {
            return tag_type::TAG_BYTE;
        }

        auto clone() const -> tag_payload * override {
            return new tag_byte_payload(data_);
        }
    };

    class tag_short_payload : public tag_payload {
        std::int16_t data_;

    public:
        tag_short_payload() { type_ = tag_type::TAG_SHORT; }
        tag_short_payload(decltype(data_) data) : data_(data) {
            type_ = tag_type::TAG_SHORT;
        }

        [[nodiscard]] auto operator*() -> std::int16_t & { return data_; }

        static constexpr auto tag_type() -> tag_type {
            return tag_type::TAG_SHORT;
        }

        auto clone() const -> tag_payload * override {
            return new tag_short_payload(data_);
        }
    };

    class tag_int_payload : public tag_payload {
        std::int32_t data_;

    public:
        tag_int_payload() { type_ = tag_type::TAG_INT; }
        tag_int_payload(decltype(data_) data) : data_(data) {
            type_ = tag_type::TAG_INT;
        }

        [[nodiscard]] auto operator*() -> std::int32_t & { return data_; }

        static constexpr auto tag_type() -> tag_type {
            return tag_type::TAG_INT;
        }

        auto clone() const -> tag_payload * override {
            return new tag_int_payload(data_);
        }
    };

    class tag_long_payload : public tag_payload {
        std::uint64_t data_;

    public:
        tag_long_payload() { type_ = tag_type::TAG_LONG; }
        tag_long_payload(decltype(data_) data) : data_(data) {
            type_ = tag_type::TAG_LONG;
        }

        [[nodiscard]] auto operator*() -> std::uint64_t & { return data_; }

        static constexpr auto tag_type() -> tag_type {
            return tag_type::TAG_LONG;
        }

        auto clone() const -> tag_long_payload * override {
            return new tag_long_payload(data_);
        }
    };

    class tag_float_payload : public tag_payload {
        float data_;

    public:
        tag_float_payload() { type_ = tag_type::TAG_FLOAT; }
        tag_float_payload(decltype(data_) data) : data_(data) {
            type_ = tag_type::TAG_FLOAT;
        }

        [[nodiscard]] auto operator*() -> float & { return data_; }

        static constexpr auto tag_type() -> tag_type {
            return tag_type::TAG_FLOAT;
        }

        auto clone() const -> tag_float_payload * override {
            return new tag_float_payload(data_);
        }
    };

    class tag_double_payload : public tag_payload {
        double data_;

    public:
        tag_double_payload() { type_ = tag_type::TAG_DOUBLE; }
        tag_double_payload(decltype(data_) data) : data_(data) {
            type_ = tag_type::TAG_DOUBLE;
        }

        [[nodiscard]] auto operator*() -> double & { return data_; }

        static constexpr auto tag_type() -> tag_type {
            return tag_type::TAG_DOUBLE;
        }

        auto clone() const -> tag_payload * override {
            return new tag_double_payload(data_);
        }
    };

    class tag_byte_array_payload : public tag_payload {
        std::vector<std::uint8_t> data_;

    public:
        tag_byte_array_payload() { type_ = tag_type::TAG_BYTE_ARRAY; }
        tag_byte_array_payload(decltype(data_) const &data) : data_(data) {
            type_ = tag_type::TAG_BYTE_ARRAY;
        }

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

        auto clone() const -> tag_byte_array_payload * override {
            return new tag_byte_array_payload(data_);
        }
    };

    class tag_string_payload : public tag_payload {
        std::string data_;

    public:
        tag_string_payload() { type_ = tag_type::TAG_STRING; }
        tag_string_payload(decltype(data_) const &data) : data_(data) {
            type_ = tag_type::TAG_STRING;
        }

        [[nodiscard]] auto operator*() -> std::string & { return data_; }

        static constexpr auto tag_type() -> tag_type {
            return tag_type::TAG_STRING;
        }

        auto clone() const -> tag_payload * override {
            return new tag_string_payload(data_);
        }
    };

    class tag_list_payload : public tag_payload {
        std::vector<tag_payload *> data_;
        tag_type payload_type_;

    public:
        tag_list_payload() { type_ = tag_type::TAG_LIST; }
        tag_list_payload(tag_type payload_type) : payload_type_(payload_type) {
            type_ = tag_type::TAG_LIST;
        }

        ~tag_list_payload() override {
            for (tag_payload *t : data_) {
                delete t;
            }
        }

        [[nodiscard]] auto operator*() -> std::vector<tag_payload *> & {
            return data_;
        }

        auto operator->() -> std::vector<tag_payload *> * { return &data_; }

        template <class Tp, class = std::is_convertible<Tp, tag_payload>>
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

        auto clone() const -> tag_payload * override {
            auto *result = new tag_list_payload(payload_type_);
            for (auto *payload : data_) {
                result->data_.push_back(payload->clone());
            }
            return result;
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

        auto clone() const -> tag_payload * override {
            auto *result = new tag_compound_payload;
            for (auto *inner : data_) {
                result->data_.push_back(inner->clone());
            }
            return result;
        }
    };

    class tag_int_array_payload : public tag_payload {
        std::vector<std::int32_t> data_;

    public:
        tag_int_array_payload() { type_ = tag_type::TAG_INT_ARRAY; }
        tag_int_array_payload(decltype(data_) const &data) : data_(data) {
            type_ = tag_type::TAG_INT_ARRAY;
        }

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

        auto clone() const -> tag_payload * override {
            return new tag_int_array_payload(data_);
        }
    };

    class tag_long_array_payload : public tag_payload {
        std::vector<std::uint64_t> data_;

    public:
        tag_long_array_payload() { type_ = tag_type::TAG_LONG_ARRAY; }
        tag_long_array_payload(decltype(data_) const &data) : data_(data) {
            type_ = tag_type::TAG_LONG_ARRAY;
        }

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

        auto clone() const -> tag_payload * override {
            return new tag_long_array_payload(data_);
        }
    };

    auto parse_type_and_name(tag::data_iterator *first,
                             tag::data_iterator const &last)
        -> std::optional<std::pair<tag_type, std::string>>;

    template <tag_type TT>
    class basic_tag : public tag {
        tag_payload *data_ = nullptr;

        basic_tag() {}

    public:
        ~basic_tag() { delete data_; }

        static auto parse_buffer(data_iterator *first,
                                 data_iterator const &last) -> basic_tag<TT> * {
            auto hdr = parse_type_and_name(first, last);
            if (!hdr) {
                return nullptr;
            }

            auto [type, name] = *hdr;
            if (type != TT) {
                return nullptr;
            }

            auto *result = new basic_tag<TT>;

            result->name_ = name;

            tag_payload *p =
                factory::payload_factories[static_cast<std::uint8_t>(TT)](first,
                                                                          last);
            if (p == nullptr) {
                delete result;
                return nullptr;
            }

            result->data_ = p;

            return result;
        }

        auto data() -> tag_payload * override { return data_; }

        auto clone() const -> tag * override {
            auto *result = new basic_tag<TT>;
            result->name_ = name_;
            if (data_ != nullptr) {
                result->data_ = data_->clone();
            }
            return result;
        }

        template <class Tp, class = std::is_convertible<Tp, tag_payload>>
        auto typed_data() -> Tp * {
            return static_cast<Tp *>(data_);
        }

        template <class Tp, class = std::is_convertible<Tp, tag_payload>>
        [[nodiscard]] auto query(nbt_path path) -> Tp * {
            tag_payload *result = query_internal(&path);

            if (result == nullptr ||
                cxx_to_nbt_type<Tp>::type() != result->type()) {
                return nullptr;
            }

            return static_cast<Tp *>(result);
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

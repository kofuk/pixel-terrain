#ifndef NBT_HH
#define NBT_HH

#include <cstdint>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <vector>

#include "utils.hh"

namespace pixel_terrain::nbt {
    using tagtype_t = unsigned char;

    static constexpr tagtype_t TAG_END = 0;
    static constexpr tagtype_t TAG_BYTE = 1;
    static constexpr tagtype_t TAG_SHORT = 2;
    static constexpr tagtype_t TAG_INT = 3;
    static constexpr tagtype_t TAG_LONG = 4;
    static constexpr tagtype_t TAG_FLOAT = 5;
    static constexpr tagtype_t TAG_DOUBLE = 6;
    static constexpr tagtype_t TAG_BYTE_ARRAY = 7;
    static constexpr tagtype_t TAG_STRING = 8;
    static constexpr tagtype_t TAG_LIST = 9;
    static constexpr tagtype_t TAG_COMPOUND = 10;
    static constexpr tagtype_t TAG_INT_ARRAY = 11;
    static constexpr tagtype_t TAG_LONG_ARRAY = 12;

    struct tag {
        tagtype_t tag_type;
        std::string name;

        tag(tagtype_t type, std::shared_ptr<unsigned char[]> buf,
            std::size_t len, std::size_t &off);
        tag(tagtype_t type);
        virtual ~tag(){};

    protected:
        std::shared_ptr<unsigned char[]> raw_buf;
        std::size_t raw_len;
        std::size_t raw_off;
    };

    /* virtual tag type to hold common resource for primitive types */
    struct tag_primitive : tag {
        tag_primitive(tagtype_t type, std::shared_ptr<unsigned char[]> buf,
                      std::size_t const len, std::size_t &off);

    protected:
        bool parsed;
    };

    struct tag_byte : tag_primitive {
        tag_byte(std::shared_ptr<unsigned char[]> buf, std::size_t const len,
                 std::size_t &off);
        static inline unsigned char
        get_value(std::shared_ptr<unsigned char[]> buf, std::size_t const len,
                  std::size_t &off) {
            unsigned char v = *(buf.get() + off);

            ++off;
            if (off >= len) {
                throw std::out_of_range("off >= len in TagByte");
            }

            return v;
        }

        unsigned char operator*();

    private:
        unsigned char value;
    };

    struct tag_short : tag_primitive {
        tag_short(std::shared_ptr<unsigned char[]> buf, std::size_t const len,
                  std::size_t &off);
        static inline std::int16_t
        get_value(std::shared_ptr<unsigned char[]> buf, std::size_t const len,
                  std::size_t &off) {
            std::int16_t v = utils::to_host_byte_order(
                *reinterpret_cast<std::int16_t *>(buf.get() + off));

            off += 2;
            if (off >= len) {
                throw std::out_of_range("off >= len in TagShort");
            }

            return v;
        }
        std::int16_t operator*();

    private:
        std::int16_t value;
    };

    struct tag_int : tag_primitive {
        tag_int(std::shared_ptr<unsigned char[]> buf, std::size_t const len,
                std::size_t &off);

        static inline std::int32_t
        get_value(std::shared_ptr<unsigned char[]> buf, std::size_t const len,
                  std::size_t &off) {
            std::int32_t v = utils::to_host_byte_order(
                *reinterpret_cast<std::int32_t *>(buf.get() + off));

            off += 4;
            if (off >= len) {
                throw std::out_of_range("off >= len in TagInt");
            }

            return v;
        }
        std::int32_t operator*();

    private:
        std::int32_t value;
    };

    struct tag_long : tag_primitive {
        tag_long(std::shared_ptr<unsigned char[]> buf, std::size_t const len,
                 std::size_t &off);

        static inline std::uint64_t
        get_value(std::shared_ptr<unsigned char[]> buf, std::size_t const len,
                  std::size_t &off) {
            std::uint64_t v = utils::to_host_byte_order(
                *reinterpret_cast<std::int64_t *>(buf.get() + off));

            off += 8;
            if (off >= len) {
                throw std::out_of_range("off >= len in TagLong");
            }

            return v;
        }

        std::uint64_t operator*();

    private:
        std::uint64_t value;
    };

    struct tag_float : tag_primitive {
        tag_float(std::shared_ptr<unsigned char[]> buf, std::size_t const len,
                  std::size_t &off);

        static inline float get_value(std::shared_ptr<unsigned char[]> buf,
                                      std::size_t const len, std::size_t &off) {
            float v = utils::to_host_byte_order(
                *reinterpret_cast<float *>(buf.get() + off));

            off += 4;
            if (off >= len) {
                throw std::out_of_range("off >= len in TagFloat");
            }

            return v;
        }

        float operator*();

    private:
        float value;
    };

    struct tag_double : tag_primitive {
        tag_double(std::shared_ptr<unsigned char[]> buf, std::size_t const len,
                   std::size_t &off);

        static inline double get_value(std::shared_ptr<unsigned char[]> buf,
                                       std::size_t const len,
                                       std::size_t &off) {
            double v = utils::to_host_byte_order(
                *reinterpret_cast<double *>(buf.get() + off));

            off += 8;
            if (off >= len) {
                throw std::out_of_range("off >= len in TagDouble");
            }

            return v;
        }
        double operator*();

    private:
        double value;
    };

    /* virtual class to hold common resource to array type */
    struct tag_array : tag {
        tag_array(tagtype_t type, std::shared_ptr<unsigned char[]> buf,
                  std::size_t const len, std::size_t &off);

    protected:
        bool parsed;
        std::int32_t array_len;
    };

    struct tag_byte_array : tag_array {
        tag_byte_array(std::shared_ptr<unsigned char[]> buf,
                       std::size_t const len, std::size_t &off);
        std::vector<char> operator*();

    private:
        std::vector<char> values;
    };

    struct tag_int_array : tag_array {
        tag_int_array(std::shared_ptr<unsigned char[]> buf,
                      std::size_t const len, std::size_t &off);
        std::vector<std::int32_t> operator*();

    private:
        std::vector<std::int32_t> values;
    };

    struct tag_long_array : tag_array {
        tag_long_array(std::shared_ptr<unsigned char[]> buf,
                       std::size_t const len, std::size_t &off);
        std::vector<std::int64_t> operator*();

    private:
        std::vector<std::int64_t> value;
    };

    struct tag_string : tag {
        tag_string(std::shared_ptr<unsigned char[]> buf, std::size_t const len,
                   std::size_t &off);
        ~tag_string();
        std::string *operator*();

        static inline std::string *
        get_value(std::shared_ptr<unsigned char[]> buf, std::size_t const len,
                  std::size_t &off) {
            std::int16_t str_len = tag_short::get_value(buf, len, off);
            std::string *v =
                new std::string(reinterpret_cast<char *>(buf.get() + off),
                                static_cast<std::size_t>(str_len));
            off += static_cast<std::size_t>(str_len);

            if (off >= len) {
                throw std::runtime_error("off >= len in tagString");
            }

            return v;
        }

    private:
        std::int16_t str_len;
        std::string *value;
    };

    struct tag_list : tag {
        tagtype_t payload_type;

        tag_list(std::shared_ptr<unsigned char[]> buf, std::size_t const len,
                 std::size_t &off);
        ~tag_list();

        void parse_buffer(std::shared_ptr<unsigned char[]> buf,
                          std::size_t const len, std::size_t &off);
        std::vector<nbt::tag *> &operator*();

    private:
        bool parsed;
        std::int32_t list_len;
        std::vector<nbt::tag *> tags;
    };

    template <typename T, class C> T value(C *clazz) {
        if (clazz == nullptr) {
            throw std::runtime_error("no value");
        }

        return **clazz;
    }

    struct tag_compound : tag {
        std::unordered_map<std::string, nbt::tag *> tags;

        tag_compound(std::shared_ptr<unsigned char[]> buf,
                     std::size_t const len, std::size_t &off, bool toplevel);
        tag_compound();
        ~tag_compound();

        void parse_buffer(std::shared_ptr<unsigned char[]> buf,
                          std::size_t const len, std::size_t &off);
        void parse_until(std::string &tag_name);

        template <typename T, tagtype_t TT> T *get_as(std::string key) {
            auto r = tags.find(key);
            if (r == end(tags)) {
                if (toplevel) {
                    parse_until(key);
                }
                r = tags.find(key);
                if (r == end(tags) || r->second->tag_type != TT) {
                    return nullptr;
                }
            } else if (r->second->tag_type != TT) {
                return nullptr;
            }

            return static_cast<T *>(r->second);
        }

    private:
        bool toplevel;
    };

    struct nbt_file : tag_compound {
        utils::decompressed_data *data;

        nbt_file(utils::decompressed_data *data);
        ~nbt_file();

        void parse_file();
    };
} // namespace pixel_terrain::nbt

#endif

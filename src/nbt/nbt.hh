#ifndef NBT_HH
#define NBT_HH

#include <cstdint>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <vector>

#include "utils.hh"

using namespace std;

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
        string name;

        tag(tagtype_t type, shared_ptr<unsigned char[]> buf, size_t len,
            size_t &off);
        tag(tagtype_t type);
        virtual ~tag(){};

    protected:
        shared_ptr<unsigned char[]> raw_buf;
        size_t raw_len;
        size_t raw_off;
    };

    /* virtual tag type to hold common resource for primitive types */
    struct tag_primitive : tag {
        tag_primitive(tagtype_t type, shared_ptr<unsigned char[]> buf,
                      size_t const len, size_t &off);

    protected:
        bool parsed;
    };

    struct tag_byte : tag_primitive {
        tag_byte(shared_ptr<unsigned char[]> buf, size_t const len,
                 size_t &off);
        static inline unsigned char get_value(shared_ptr<unsigned char[]> buf,
                                              size_t const len, size_t &off) {
            unsigned char v = *(buf.get() + off);

            ++off;
            if (off >= len) {
                throw out_of_range("off >= len in TagByte");
            }

            return v;
        }

        unsigned char operator*();

    private:
        unsigned char value;
    };

    struct tag_short : tag_primitive {
        tag_short(shared_ptr<unsigned char[]> buf, size_t const len,
                  size_t &off);
        static inline int16_t get_value(shared_ptr<unsigned char[]> buf,
                                        size_t const len, size_t &off) {
            int16_t v = utils::to_host_byte_order(
                *reinterpret_cast<int16_t *>(buf.get() + off));

            off += 2;
            if (off >= len) {
                throw out_of_range("off >= len in TagShort");
            }

            return v;
        }
        int16_t operator*();

    private:
        int16_t value;
    };

    struct tag_int : tag_primitive {
        tag_int(shared_ptr<unsigned char[]> buf, size_t const len, size_t &off);

        static inline int32_t get_value(shared_ptr<unsigned char[]> buf,
                                        size_t const len, size_t &off) {
            int32_t v = utils::to_host_byte_order(
                *reinterpret_cast<int32_t *>(buf.get() + off));

            off += 4;
            if (off >= len) {
                throw out_of_range("off >= len in TagInt");
            }

            return v;
        }
        int32_t operator*();

    private:
        int32_t value;
    };

    struct tag_long : tag_primitive {
        tag_long(shared_ptr<unsigned char[]> buf, size_t const len,
                 size_t &off);

        static inline uint64_t get_value(shared_ptr<unsigned char[]> buf,
                                         size_t const len, size_t &off) {
            uint64_t v = utils::to_host_byte_order(
                *reinterpret_cast<int64_t *>(buf.get() + off));

            off += 8;
            if (off >= len) {
                throw out_of_range("off >= len in TagLong");
            }

            return v;
        }

        uint64_t operator*();

    private:
        uint64_t value;
    };

    struct tag_float : tag_primitive {
        tag_float(shared_ptr<unsigned char[]> buf, size_t const len,
                  size_t &off);

        static inline float get_value(shared_ptr<unsigned char[]> buf,
                                      size_t const len, size_t &off) {
            float v = utils::to_host_byte_order(
                *reinterpret_cast<float *>(buf.get() + off));

            off += 4;
            if (off >= len) {
                throw out_of_range("off >= len in TagFloat");
            }

            return v;
        }

        float operator*();

    private:
        float value;
    };

    struct tag_double : tag_primitive {
        tag_double(shared_ptr<unsigned char[]> buf, size_t const len,
                   size_t &off);

        static inline double get_value(shared_ptr<unsigned char[]> buf,
                                       size_t const len, size_t &off) {
            double v = utils::to_host_byte_order(
                *reinterpret_cast<double *>(buf.get() + off));

            off += 8;
            if (off >= len) {
                throw out_of_range("off >= len in TagDouble");
            }

            return v;
        }
        double operator*();

    private:
        double value;
    };

    /* virtual class to hold common resource to array type */
    struct tag_array : tag {
        tag_array(tagtype_t type, shared_ptr<unsigned char[]> buf,
                  size_t const len, size_t &off);

    protected:
        bool parsed;
        int32_t array_len;
    };

    struct tag_byte_array : tag_array {
        tag_byte_array(shared_ptr<unsigned char[]> buf, size_t const len,
                       size_t &off);
        vector<char> operator*();

    private:
        vector<char> values;
    };

    struct tag_int_array : tag_array {
        tag_int_array(shared_ptr<unsigned char[]> buf, size_t const len,
                      size_t &off);
        vector<int32_t> operator*();

    private:
        vector<int32_t> values;
    };

    struct tag_long_array : tag_array {
        tag_long_array(shared_ptr<unsigned char[]> buf, size_t const len,
                       size_t &off);
        vector<int64_t> operator*();

    private:
        vector<int64_t> value;
    };

    struct tag_string : tag {
        tag_string(shared_ptr<unsigned char[]> buf, size_t const len,
                   size_t &off);
        ~tag_string();
        string *operator*();

        static inline string *get_value(shared_ptr<unsigned char[]> buf,
                                        size_t const len, size_t &off) {
            int16_t str_len = tag_short::get_value(buf, len, off);
            string *v = new string(reinterpret_cast<char *>(buf.get() + off),
                                   static_cast<size_t>(str_len));
            off += static_cast<size_t>(str_len);

            if (off >= len) {
                throw runtime_error("off >= len in tagString");
            }

            return v;
        }

    private:
        int16_t str_len;
        string *value;
    };

    struct tag_list : tag {
        tagtype_t payload_type;

        tag_list(shared_ptr<unsigned char[]> buf, size_t const len,
                 size_t &off);
        ~tag_list();

        void parse_buffer(shared_ptr<unsigned char[]> buf, size_t const len,
                          size_t &off);
        vector<nbt::tag *> &operator*();

    private:
        bool parsed;
        int32_t list_len;
        vector<nbt::tag *> tags;
    };

    template <typename T, class C> T value(C *clazz) {
        if (clazz == nullptr) {
            throw runtime_error("no value");
        }

        return **clazz;
    }

    struct tag_compound : tag {
        unordered_map<string, nbt::tag *> tags;

        tag_compound(shared_ptr<unsigned char[]> buf, size_t const len,
                     size_t &off, bool toplevel);
        tag_compound();
        ~tag_compound();

        void parse_buffer(shared_ptr<unsigned char[]> buf, size_t const len,
                          size_t &off);
        void parse_until(string &tag_name);

        template <typename T, tagtype_t TT> T *get_as(string key) {
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

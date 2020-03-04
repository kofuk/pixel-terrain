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

namespace NBT {
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

    struct Tag {
        tagtype_t tag_type;
        string name;

        Tag (tagtype_t type, shared_ptr<unsigned char[]> buf, size_t len,
             size_t &off);
        Tag (tagtype_t type);
        virtual ~Tag (){};

    protected:
        shared_ptr<unsigned char[]> raw_buf;
        size_t raw_len;
        size_t raw_off;
    };

    /* virtual tag type to hold common resource for primitive types */
    struct TagPrimitive : Tag {
        TagPrimitive (tagtype_t type, shared_ptr<unsigned char[]> buf,
                      size_t const len, size_t &off);

    protected:
        bool parsed;
    };

    struct TagByte : TagPrimitive {
        TagByte (shared_ptr<unsigned char[]> buf, size_t const len,
                 size_t &off);
        static inline unsigned char get_value (shared_ptr<unsigned char[]> buf,
                                               size_t const len, size_t &off) {
            unsigned char v = *(buf.get () + off);

            ++off;
            if (off >= len) {
                throw out_of_range ("off >= len in TagByte");
            }

            return v;
        }

        unsigned char operator* ();

    private:
        unsigned char value;
    };

    struct TagShort : TagPrimitive {
        TagShort (shared_ptr<unsigned char[]> buf, size_t const len,
                  size_t &off);
        static inline int16_t get_value (shared_ptr<unsigned char[]> buf,
                                         size_t const len, size_t &off) {
            int16_t v = Utils::to_host_byte_order (
                *reinterpret_cast<int16_t *> (buf.get () + off));

            off += 2;
            if (off >= len) {
                throw out_of_range ("off >= len in TagShort");
            }

            return v;
        }
        int16_t operator* ();

    private:
        int16_t value;
    };

    struct TagInt : TagPrimitive {
        TagInt (shared_ptr<unsigned char[]> buf, size_t const len, size_t &off);

        static inline int32_t get_value (shared_ptr<unsigned char[]> buf,
                                         size_t const len, size_t &off) {
            int32_t v = Utils::to_host_byte_order (
                *reinterpret_cast<int32_t *> (buf.get () + off));

            off += 4;
            if (off >= len) {
                throw out_of_range ("off >= len in TagInt");
            }

            return v;
        }
        int32_t operator* ();

    private:
        int32_t value;
    };

    struct TagLong : TagPrimitive {
        TagLong (shared_ptr<unsigned char[]> buf, size_t const len,
                 size_t &off);

        static inline uint64_t get_value (shared_ptr<unsigned char[]> buf,
                                          size_t const len, size_t &off) {
            uint64_t v = Utils::to_host_byte_order (
                *reinterpret_cast<int64_t *> (buf.get () + off));

            off += 8;
            if (off >= len) {
                throw out_of_range ("off >= len in TagLong");
            }

            return v;
        }

        uint64_t operator* ();

    private:
        uint64_t value;
    };

    struct TagFloat : TagPrimitive {
        TagFloat (shared_ptr<unsigned char[]> buf, size_t const len,
                  size_t &off);

        static inline float get_value (shared_ptr<unsigned char[]> buf,
                                       size_t const len, size_t &off) {
            float v = Utils::to_host_byte_order (
                *reinterpret_cast<float *> (buf.get () + off));

            off += 4;
            if (off >= len) {
                throw out_of_range ("off >= len in TagFloat");
            }

            return v;
        }

        float operator* ();

    private:
        float value;
    };

    struct TagDouble : TagPrimitive {
        TagDouble (shared_ptr<unsigned char[]> buf, size_t const len,
                   size_t &off);

        static inline double get_value (shared_ptr<unsigned char[]> buf,
                                        size_t const len, size_t &off) {
            double v = Utils::to_host_byte_order (
                *reinterpret_cast<double *> (buf.get () + off));

            off += 8;
            if (off >= len) {
                throw out_of_range ("off >= len in TagDouble");
            }

            return v;
        }
        double operator* ();

    private:
        double value;
    };

    /* virtual class to hold common resource to array type */
    struct TagArray : Tag {
        TagArray (tagtype_t type, shared_ptr<unsigned char[]> buf, size_t const len,
                      size_t &off);

    protected:
        bool parsed;
        int32_t array_len;
    };

    struct TagByteArray : TagArray {
        TagByteArray (shared_ptr<unsigned char[]> buf, size_t const len,
                      size_t &off);
        vector<char> operator* ();

    private:
        vector<char> values;
    };

    struct TagIntArray : TagArray {
        TagIntArray (shared_ptr<unsigned char[]> buf, size_t const len,
                     size_t &off);
        vector<int32_t> operator* ();

    private:
        vector<int32_t> values;
    };

    struct TagLongArray : TagArray {
        TagLongArray (shared_ptr<unsigned char[]> buf, size_t const len,
                      size_t &off);
        vector<int64_t> operator* ();

    private:
        vector<int64_t> value;
    };

    struct TagString : Tag {
        TagString (shared_ptr<unsigned char[]> buf, size_t const len,
                   size_t &off);
        ~TagString ();
        string *operator* ();

        static inline string *get_value (shared_ptr<unsigned char[]> buf,
                                         size_t const len, size_t &off) {
            int16_t str_len = TagShort::get_value (buf, len, off);
            string *v = new string (reinterpret_cast<char *> (buf.get () + off),
                                    static_cast<size_t> (str_len));
            off += static_cast<size_t> (str_len);

            if (off >= len) {
                throw runtime_error ("off >= len in tagString");
            }

            return v;
        }

    private:
        int16_t str_len;
        string *value;
    };

    struct TagList : Tag {
        vector<NBT::Tag *> tags;
        tagtype_t payload_type;

        TagList (shared_ptr<unsigned char[]> buf, size_t const len,
                 size_t &off);
        ~TagList ();

        void parse_buffer (shared_ptr<unsigned char[]> buf, size_t const len,
                           size_t &off);
    };

    template <typename T, class C> T value (C *clazz) {
        if (clazz == nullptr) {
            throw runtime_error ("no value");
        }

        return **clazz;
    }

    struct TagCompound : Tag {
        unordered_map<string, NBT::Tag *> tags;

        TagCompound (shared_ptr<unsigned char[]> buf, size_t const len,
                     size_t &off);
        TagCompound ();
        ~TagCompound ();

        void parse_buffer (shared_ptr<unsigned char[]> buf, size_t const len,
                           size_t &off);

        template <typename T, tagtype_t TT> T *get_as (string key) {
            auto r = tags.find (key);
            if (r == end (tags) || r->second->tag_type != TT) {
                return nullptr;
            }

            return static_cast<T *> (r->second);
        }
    };

    struct NBTFile : TagCompound {
        Utils::DecompressedData *data;

        NBTFile (Utils::DecompressedData *data);
        ~NBTFile ();

        void parse_file ();
    };
} // namespace NBT

#endif

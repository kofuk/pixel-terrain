#ifndef NBT_HH
#define NBT_HH

#include <cstdint>
#include <iostream>
#include <unordered_map>
#include <vector>

#include "Utils.hh"

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

        Tag(tagtype_t type);
        virtual ~Tag(){};

        virtual void parse_buffer(unsigned char const *buf, size_t const len,
                                  size_t &off) = 0;
    };

    struct TagByte : Tag {
        TagByte(unsigned char const *buf, size_t const len, size_t &off);

        void parse_buffer(unsigned char const *buf, size_t const len,
                          size_t &off);
        unsigned char value;
    };

    struct TagShort : Tag {
        TagShort(unsigned char const *buf, size_t const len, size_t &off);

        void parse_buffer(unsigned char const *buf, size_t const len,
                          size_t &off);
        int16_t value;
    };

    struct TagInt : Tag {
        TagInt(unsigned char const *buf, size_t const len, size_t &off);

        void parse_buffer(unsigned char const *buf, size_t const len,
                          size_t &off);
        int32_t value;
    };

    struct TagLong : Tag {
        TagLong(unsigned char const *buf, size_t const len, size_t &off);

        void parse_buffer(unsigned char const *buf, size_t const len,
                          size_t &off);
        uint64_t value;
    };

    struct TagFloat : Tag {
        TagFloat(unsigned char const *buf, size_t const len, size_t &off);

        void parse_buffer(unsigned char const *buf, size_t const len,
                          size_t &off);
        float value;
    };

    struct TagDouble : Tag {
        TagDouble(unsigned char const *buf, size_t const len, size_t &off);

        void parse_buffer(unsigned char const *buf, size_t const len,
                          size_t &off);
        double value;
    };

    struct TagByteArray : Tag {
        vector<char> values;

        TagByteArray(unsigned char const *buf, size_t const len, size_t &off);

        void parse_buffer(unsigned char const *buf, size_t const len,
                          size_t &off);
    };

    struct TagIntArray : Tag {
        vector<int32_t> values;

        TagIntArray(unsigned char const *buf, size_t const len, size_t &off);

        void parse_buffer(unsigned char const *buf, size_t const len,
                          size_t &off);
    };

    struct TagLongArray : Tag {
        vector<int64_t> values;

        TagLongArray(unsigned char const *buf, size_t const len, size_t &off);

        void parse_buffer(unsigned char const *buf, size_t const len,
                          size_t &off);
    };

    struct TagString : Tag {
        TagString(unsigned char const *buf, size_t const len, size_t &off);
        ~TagString();

        void parse_buffer(unsigned char const *buf, size_t const len,
                          size_t &off);
        string *value;
    };

    struct TagList : Tag {
        vector<NBT::Tag *> tags;
        tagtype_t payload_type;

        TagList(unsigned char const *buf, size_t const len, size_t &off);
        ~TagList();

        void parse_buffer(unsigned char const *buf, size_t const len,
                          size_t &off);
    };

    struct TagCompound : Tag {
        unordered_map<string, NBT::Tag *> tags;

        TagCompound(unsigned char const *buf, size_t const len, size_t &off);
        TagCompound();
        ~TagCompound();

        void parse_buffer(unsigned char const *buf, size_t const len,
                          size_t &off);
        Tag *operator[](string key);
    };

    struct NBTFile : TagCompound {
        Utils::DecompressedData *data;

        NBTFile(Utils::DecompressedData *data);
        ~NBTFile();

        void parse_file();
    };
} // namespace NBT

#endif

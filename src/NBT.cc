#include <cassert>
#include <cstdint>
#include <iterator>

#include "NBT.hh"
#include "Utils.hh"

namespace NBT {
    Tag::Tag(tagtype_t type) : tag_type(type) {}

    TagByte::TagByte(unsigned char const *buf, size_t const len, size_t &off)
        : Tag(TAG_BYTE) {
        parse_buffer(buf, len, off);
    }

    void TagByte::parse_buffer(unsigned char const *buf, size_t const len,
                               size_t &off) {
        value = *(buf + off);

        ++off;
        assert(off < len);
    }

    TagShort::TagShort(unsigned char const *buf, size_t const len, size_t &off)
        : Tag(TAG_SHORT) {
        parse_buffer(buf, len, off);
    }

    void TagShort::parse_buffer(unsigned char const *buf, size_t const len,
                                size_t &off) {
        value = Utils::to_host_byte_order(*(int16_t *)(buf + off));

        off += 2;
        assert(off < len);
    }

    TagInt::TagInt(unsigned char const *buf, size_t const len, size_t &off)
        : Tag(TAG_INT) {
        parse_buffer(buf, len, off);
    }

    void TagInt::parse_buffer(unsigned char const *buf, size_t const len,
                              size_t &off) {
        value = Utils::to_host_byte_order(*(int32_t *)(buf + off));

        off += 4;
        assert(off < len);
    }

    TagLong::TagLong(unsigned char const *buf, size_t const len, size_t &off)
        : Tag(TAG_LONG) {
        parse_buffer(buf, len, off);
    }

    void TagLong::parse_buffer(unsigned char const *buf, size_t const len,
                               size_t &off) {
        value = Utils::to_host_byte_order(*(int64_t *)(buf + off));

        off += 8;
        assert(off < len);
    }

    TagFloat::TagFloat(unsigned char const *buf, size_t const len, size_t &off)
        : Tag(TAG_FLOAT) {
        parse_buffer(buf, len, off);
    }

    void TagFloat::parse_buffer(unsigned char const *buf, size_t const len,
                                size_t &off) {
        value = Utils::to_host_byte_order(*(float *)(buf + off));

        off += 4;
        assert(off < len);
    }

    TagDouble::TagDouble(unsigned char const *buf, size_t const len,
                         size_t &off)
        : Tag(TAG_DOUBLE) {
        parse_buffer(buf, len, off);
    }

    void TagDouble::parse_buffer(unsigned char const *buf, size_t const len,
                                 size_t &off) {
        value = Utils::to_host_byte_order(*(double *)(buf + off));

        off += 8;
        assert(off < len);
    }

    TagByteArray::TagByteArray(unsigned char const *buf, size_t const len,
                               size_t &off)
        : Tag(TAG_BYTE_ARRAY) {
        parse_buffer(buf, len, off);
    }

    void TagByteArray::parse_buffer(unsigned char const *buf, size_t const len,
                                    size_t &off) {
        TagInt len_tag(buf, len, off);
        int32_t arr_len = len_tag.value;
        for (int32_t i = 0; i < arr_len; ++i) {
            values.push_back(*(buf + off));

            ++off;
            assert(off < len);
        }
    }

    TagIntArray::TagIntArray(unsigned char const *buf, size_t const len,
                             size_t &off)
        : Tag(TAG_INT_ARRAY) {
        parse_buffer(buf, len, off);
    }

    void TagIntArray::parse_buffer(unsigned char const *buf, size_t const len,
                                   size_t &off) {
        TagInt len_tag(buf, len, off);
        int32_t arr_len = len_tag.value;
        for (int32_t i = 0; i < arr_len; ++i) {
            values.push_back(
                Utils::to_host_byte_order(*(int32_t *)(buf + off)));

            off += 4;
            assert(off < len);
        }
    }

    TagLongArray::TagLongArray(unsigned char const *buf, size_t const len,
                               size_t &off)
        : Tag(TAG_LONG_ARRAY) {
        parse_buffer(buf, len, off);
    }

    void TagLongArray::parse_buffer(unsigned char const *buf, size_t const len,
                                    size_t &off) {
        TagInt len_tag(buf, len, off);
        int32_t arr_len = len_tag.value;
        for (int32_t i = 0; i < arr_len; ++i) {
            values.push_back(
                (uint64_t)Utils::to_host_byte_order(*(int64_t *)(buf + off)));
            off += 8;

            assert(off < len);
        }
    }

    TagString::TagString(unsigned char const *buf, size_t const len,
                         size_t &off)
        : Tag(TAG_STRING) {
        parse_buffer(buf, len, off);
    }

    TagString::~TagString() { delete value; }

    void TagString::parse_buffer(unsigned char const *buf, size_t const len,
                                 size_t &off) {
        TagShort len_tag(buf, len, off);
        int16_t str_len = len_tag.value;
        value = new string((char *)buf + off, (size_t)str_len);
        off += (size_t)str_len;

        assert(off < len);
    }

    TagList::TagList(unsigned char const *buf, size_t const len, size_t &off)
        : Tag(TAG_LIST) {
        parse_buffer(buf, len, off);
    }

    TagList::~TagList() {
        for (auto itr = std::begin(tags); itr != std::end(tags); ++itr) {
            delete *itr;
        }
    }

    void TagList::parse_buffer(unsigned char const *buf, size_t const len,
                               size_t &off) {
        TagByte type_tag(buf, len, off);
        payload_type = type_tag.value;

        TagInt len_tag(buf, len, off);
        int32_t list_len = len_tag.value;

        Tag *tag;
        for (int32_t i = 0; i < list_len; ++i) {
            if (payload_type == TAG_BYTE)
                tag = new TagByte(buf, len, off);
            else if (payload_type == TAG_SHORT)
                tag = new TagShort(buf, len, off);
            else if (payload_type == TAG_INT)
                tag = new TagInt(buf, len, off);
            else if (payload_type == TAG_LONG)
                tag = new TagLong(buf, len, off);
            else if (payload_type == TAG_FLOAT)
                tag = new TagFloat(buf, len, off);
            else if (payload_type == TAG_DOUBLE)
                tag = new TagDouble(buf, len, off);
            else if (payload_type == TAG_BYTE_ARRAY)
                tag = new TagByteArray(buf, len, off);
            else if (payload_type == TAG_STRING)
                tag = new TagString(buf, len, off);
            else if (payload_type == TAG_LIST)
                tag = new TagList(buf, len, off);
            else if (payload_type == TAG_COMPOUND)
                tag = new TagCompound(buf, len, off);
            else if (payload_type == TAG_INT_ARRAY)
                tag = new TagIntArray(buf, len, off);
            else if (payload_type == TAG_LONG_ARRAY)
                tag = new TagLongArray(buf, len, off);
            else
                assert(0); /* will never happen */

            tags.push_back(tag);
        }
    }

    TagCompound::TagCompound(unsigned char const *buf, size_t const len,
                             size_t &off)
        : Tag(TAG_COMPOUND) {
        parse_buffer(buf, len, off);
    }

    TagCompound::TagCompound() : Tag(TAG_COMPOUND) {}

    TagCompound::~TagCompound() {
        for (auto itr = std::begin(tags); itr != std::end(tags); ++itr) {
            delete itr->second;
        }
    }

    void TagCompound::parse_buffer(unsigned char const *buf, size_t const len,
                                   size_t &off) {
        for (;;) {
            TagByte type_tag(buf, len, off);
            tagtype_t type = type_tag.value;

            if (type == TAG_END) break;

            TagString name_tag(buf, len, off);

            Tag *tag;
            if (type == TAG_BYTE)
                tag = new TagByte(buf, len, off);
            else if (type == TAG_SHORT)
                tag = new TagShort(buf, len, off);
            else if (type == TAG_INT)
                tag = new TagInt(buf, len, off);
            else if (type == TAG_LONG)
                tag = new TagLong(buf, len, off);
            else if (type == TAG_FLOAT)
                tag = new TagFloat(buf, len, off);
            else if (type == TAG_DOUBLE)
                tag = new TagDouble(buf, len, off);
            else if (type == TAG_BYTE_ARRAY)
                tag = new TagByteArray(buf, len, off);
            else if (type == TAG_STRING)
                tag = new TagString(buf, len, off);
            else if (type == TAG_LIST)
                tag = new TagList(buf, len, off);
            else if (type == TAG_COMPOUND)
                tag = new TagCompound(buf, len, off);
            else if (type == TAG_INT_ARRAY)
                tag = new TagIntArray(buf, len, off);
            else if (type == TAG_LONG_ARRAY)
                tag = new TagLongArray(buf, len, off);
            else
                assert(0); /* will never happen */

            string name = *name_tag.value;
            tags[name] = tag;

            /* FIXME: Work around for known issue, reaches end of file after
             * last TagCompound element read. */
            if (off >= len - 1) {
                break;
            }
        }
    }

    Tag *TagCompound::operator[](string key) { return tags[key]; }

    NBTFile::NBTFile(Utils::DecompressedData *data)
        : TagCompound(), data(data) {
        parse_file();
    }

    NBTFile::~NBTFile() { delete data; }

    void NBTFile::parse_file() {
        size_t off = 0;

        TagByte type_tag(data->data, data->len, off);
        tagtype_t type = type_tag.value;

        if (type != tag_type) {
            throw 1;
        }

        TagString name_tag(data->data, data->len, off);
        name = *name_tag.value;
        parse_buffer(data->data, data->len, off);
    }
} // namespace NBT

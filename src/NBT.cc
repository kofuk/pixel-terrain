#include <cstdint>
#include <iterator>
#include <stdexcept>

#include "NBT.hh"
#include "utils.hh"

namespace NBT {
    Tag::Tag (tagtype_t type) : tag_type (type) {}

    TagByte::TagByte (unsigned char const *buf, size_t const len, size_t &off)
        : Tag (TAG_BYTE) {
        parse_buffer (buf, len, off);
    }

    void TagByte::parse_buffer (unsigned char const *buf, size_t const len,
                                size_t &off) {
        value = get_value (buf, len, off);
    }

    TagShort::TagShort (unsigned char const *buf, size_t const len, size_t &off)
        : Tag (TAG_SHORT) {
        parse_buffer (buf, len, off);
    }

    void TagShort::parse_buffer (unsigned char const *buf, size_t const len,
                                 size_t &off) {
        value = get_value (buf, len, off);
    }

    TagInt::TagInt (unsigned char const *buf, size_t const len, size_t &off)
        : Tag (TAG_INT) {
        parse_buffer (buf, len, off);
    }

    void TagInt::parse_buffer (unsigned char const *buf, size_t const len,
                               size_t &off) {
        value = get_value (buf, len, off);
    }

    TagLong::TagLong (unsigned char const *buf, size_t const len, size_t &off)
        : Tag (TAG_LONG) {
        parse_buffer (buf, len, off);
    }

    void TagLong::parse_buffer (unsigned char const *buf, size_t const len,
                                size_t &off) {
        value = get_value (buf, len, off);
    }

    TagFloat::TagFloat (unsigned char const *buf, size_t const len, size_t &off)
        : Tag (TAG_FLOAT) {
        parse_buffer (buf, len, off);
    }

    void TagFloat::parse_buffer (unsigned char const *buf, size_t const len,
                                 size_t &off) {
        value = get_value (buf, len, off);
    }

    TagDouble::TagDouble (unsigned char const *buf, size_t const len,
                          size_t &off)
        : Tag (TAG_DOUBLE) {
        parse_buffer (buf, len, off);
    }

    void TagDouble::parse_buffer (unsigned char const *buf, size_t const len,
                                  size_t &off) {
        value = get_value (buf, len, off);
    }

    TagByteArray::TagByteArray (unsigned char const *buf, size_t const len,
                                size_t &off)
        : Tag (TAG_BYTE_ARRAY) {
        parse_buffer (buf, len, off);
    }

    void TagByteArray::parse_buffer (unsigned char const *buf, size_t const len,
                                     size_t &off) {
        int32_t arr_len = TagInt::get_value (buf, len, off);
        for (int32_t i = 0; i < arr_len; ++i) {
            values.push_back (*(buf + off));

            ++off;
            if (off >= len) {
                throw out_of_range ("off >= len in TagByteArray");
            }
        }
    }

    TagIntArray::TagIntArray (unsigned char const *buf, size_t const len,
                              size_t &off)
        : Tag (TAG_INT_ARRAY) {
        parse_buffer (buf, len, off);
    }

    void TagIntArray::parse_buffer (unsigned char const *buf, size_t const len,
                                    size_t &off) {
        int32_t arr_len = TagInt::get_value (buf, len, off);
        for (int32_t i = 0; i < arr_len; ++i) {
            values.push_back (TagInt::get_value (buf, len, off));
        }
    }

    TagLongArray::TagLongArray (unsigned char const *buf, size_t const len,
                                size_t &off)
        : Tag (TAG_LONG_ARRAY) {
        parse_buffer (buf, len, off);
    }

    void TagLongArray::parse_buffer (unsigned char const *buf, size_t const len,
                                     size_t &off) {
        int32_t arr_len = TagInt::get_value (buf, len, off);
        for (int32_t i = 0; i < arr_len; ++i) {
            value.push_back (TagLong::get_value (buf, len, off));
        }
    }

    TagString::TagString (unsigned char const *buf, size_t const len,
                          size_t &off)
        : Tag (TAG_STRING) {
        parse_buffer (buf, len, off);
    }

    TagString::~TagString () { delete value; }

    void TagString::parse_buffer (unsigned char const *buf, size_t const len,
                                  size_t &off) {
        value = get_value (buf, len, off);
    }

    TagList::TagList (unsigned char const *buf, size_t const len, size_t &off)
        : Tag (TAG_LIST) {
        parse_buffer (buf, len, off);
    }

    TagList::~TagList () {
        for (auto itr = std::begin (tags); itr != std::end (tags); ++itr) {
            delete *itr;
        }
    }

    void TagList::parse_buffer (unsigned char const *buf, size_t const len,
                                size_t &off) {
        payload_type = TagByte::get_value (buf, len, off);

        int32_t list_len = TagInt::get_value (buf, len, off);

        Tag *tag;
        for (int32_t i = 0; i < list_len; ++i) {
            if (payload_type == TAG_BYTE)
                tag = new TagByte (buf, len, off);
            else if (payload_type == TAG_SHORT)
                tag = new TagShort (buf, len, off);
            else if (payload_type == TAG_INT)
                tag = new TagInt (buf, len, off);
            else if (payload_type == TAG_LONG)
                tag = new TagLong (buf, len, off);
            else if (payload_type == TAG_FLOAT)
                tag = new TagFloat (buf, len, off);
            else if (payload_type == TAG_DOUBLE)
                tag = new TagDouble (buf, len, off);
            else if (payload_type == TAG_BYTE_ARRAY)
                tag = new TagByteArray (buf, len, off);
            else if (payload_type == TAG_STRING)
                tag = new TagString (buf, len, off);
            else if (payload_type == TAG_LIST)
                tag = new TagList (buf, len, off);
            else if (payload_type == TAG_COMPOUND)
                tag = new TagCompound (buf, len, off);
            else if (payload_type == TAG_INT_ARRAY)
                tag = new TagIntArray (buf, len, off);
            else if (payload_type == TAG_LONG_ARRAY)
                tag = new TagLongArray (buf, len, off);
            else
                throw runtime_error ("Unknown type of tag: " +
                                     to_string (payload_type));

            tags.push_back (tag);
        }
    }

    TagCompound::TagCompound (unsigned char const *buf, size_t const len,
                              size_t &off)
        : Tag (TAG_COMPOUND) {
        parse_buffer (buf, len, off);
    }

    TagCompound::TagCompound () : Tag (TAG_COMPOUND) {}

    TagCompound::~TagCompound () {
        for (auto itr = std::begin (tags); itr != std::end (tags); ++itr) {
            delete itr->second;
        }
    }

    void TagCompound::parse_buffer (unsigned char const *buf, size_t const len,
                                    size_t &off) {
        for (;;) {
            tagtype_t type = TagByte::get_value (buf, len, off);

            if (type == TAG_END) break;

            string *name = TagString::get_value (buf, len, off);

            Tag *tag;
            if (type == TAG_BYTE)
                tag = new TagByte (buf, len, off);
            else if (type == TAG_SHORT)
                tag = new TagShort (buf, len, off);
            else if (type == TAG_INT)
                tag = new TagInt (buf, len, off);
            else if (type == TAG_LONG)
                tag = new TagLong (buf, len, off);
            else if (type == TAG_FLOAT)
                tag = new TagFloat (buf, len, off);
            else if (type == TAG_DOUBLE)
                tag = new TagDouble (buf, len, off);
            else if (type == TAG_BYTE_ARRAY)
                tag = new TagByteArray (buf, len, off);
            else if (type == TAG_STRING)
                tag = new TagString (buf, len, off);
            else if (type == TAG_LIST)
                tag = new TagList (buf, len, off);
            else if (type == TAG_COMPOUND)
                tag = new TagCompound (buf, len, off);
            else if (type == TAG_INT_ARRAY)
                tag = new TagIntArray (buf, len, off);
            else if (type == TAG_LONG_ARRAY)
                tag = new TagLongArray (buf, len, off);
            else
                throw runtime_error ("unknown type of tag: " +
                                     to_string (type));

            tags[*name] = tag;

            delete name;

            /* FIXME: Work around for known issue, reaches end of file after
             * last TagCompound element read. */
            if (off >= len - 1) {
                break;
            }
        }
    }

    NBTFile::NBTFile (Utils::DecompressedData *data)
        : TagCompound (), data (data) {
        parse_file ();
    }

    NBTFile::~NBTFile () { delete data; }

    void NBTFile::parse_file () {
        size_t off = 0;

        tagtype_t type = TagByte::get_value (data->data, data->len, off);

        if (type != tag_type) {
            throw invalid_argument ("corrupted data file");
        }

        name = *TagString::get_value (data->data, data->len, off);
        parse_buffer (data->data, data->len, off);
    }
} // namespace NBT

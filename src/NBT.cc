#include <cstdint>
#include <iterator>
#include <stdexcept>

#include "NBT.hh"
#include "utils.hh"

namespace NBT {
    Tag::Tag (tagtype_t type, shared_ptr<unsigned char[]> buf, size_t len,
              size_t &off)
        : tag_type (type), raw_buf (move (buf)), raw_len (len), raw_off (off) {}

    Tag::Tag (tagtype_t type)
        : tag_type (type), raw_buf (nullptr), raw_len (0), raw_off (0) {}

    TagPrimitive::TagPrimitive (tagtype_t type, shared_ptr<unsigned char[]> buf,
                                size_t len, size_t &off)
        : Tag (type, buf, len, off), parsed (false) {}

    TagByte::TagByte (shared_ptr<unsigned char[]> buf, size_t const len,
                      size_t &off)
        : TagPrimitive (TAG_BYTE, buf, len, off) {
        ++off;
    }

    unsigned char TagByte::operator* () {
        if (!parsed) {
            value = get_value (raw_buf, raw_len, raw_off);
            parsed = true;
        }
        return value;
    }

    TagShort::TagShort (shared_ptr<unsigned char[]> buf, size_t const len,
                        size_t &off)
        : TagPrimitive (TAG_SHORT, buf, len, off) {
        off += 2;
    }

    int16_t TagShort::operator* () {
        if (!parsed) {
            value = get_value (raw_buf, raw_len, raw_off);
            parsed = true;
        }
        return value;
    }

    TagInt::TagInt (shared_ptr<unsigned char[]> buf, size_t const len,
                    size_t &off)
        : TagPrimitive (TAG_INT, buf, len, off) {
        off += 4;
    }

    int32_t TagInt::operator* () {
        if (!parsed) {
            value = get_value (raw_buf, raw_len, raw_off);
            parsed = true;
        }
        return value;
    }

    TagLong::TagLong (shared_ptr<unsigned char[]> buf, size_t const len,
                      size_t &off)
        : TagPrimitive (TAG_LONG, buf, len, off) {
        off += 8;
    }

    uint64_t TagLong::operator* () {
        if (!parsed) {
            value = get_value (raw_buf, raw_len, raw_off);
            parsed = true;
        }
        return value;
    }

    TagFloat::TagFloat (shared_ptr<unsigned char[]> buf, size_t const len,
                        size_t &off)
        : TagPrimitive (TAG_FLOAT, buf, len, off) {
        off += 4;
    }

    float TagFloat::operator* () {
        if (!parsed) {
            value = get_value (raw_buf, raw_len, raw_off);
            parsed = true;
        }
        return value;
    }

    TagDouble::TagDouble (shared_ptr<unsigned char[]> buf, size_t const len,
                          size_t &off)
        : TagPrimitive (TAG_DOUBLE, buf, len, off) {
        off += 8;
    }

    double TagDouble::operator* () {
        if (!parsed) {
            value = get_value (raw_buf, raw_len, raw_off);
            parsed = true;
        }
        return value;
    }

    TagArray::TagArray (tagtype_t type, shared_ptr<unsigned char[]> buf,
                        size_t const len, size_t &off)
        : Tag (type, buf, len, off), parsed (false) {}

    TagByteArray::TagByteArray (shared_ptr<unsigned char[]> buf,
                                size_t const len, size_t &off)
        : TagArray (TAG_BYTE_ARRAY, buf, len, off) {
        array_len = TagInt::get_value (buf, len, off);
        raw_off = off;
        off += array_len;
    }

    vector<char> TagByteArray::operator* () {
        if (!parsed) {
            for (int32_t i = 0; i < array_len; ++i) {
                values.push_back (*(raw_buf.get () + raw_off));

                ++raw_off;
                if (raw_off >= raw_len) {
                    throw out_of_range ("off >= len in TagByteArray");
                }
            }
            parsed = true;
        }
        return values; }

    TagIntArray::TagIntArray (shared_ptr<unsigned char[]> buf, size_t const len,
                              size_t &off)
        : TagArray (TAG_INT_ARRAY, buf, len, off) {
        array_len = TagInt::get_value (buf, len, off);
        raw_off = off;
        off += array_len * 4;
    }

    vector<int32_t> TagIntArray::operator* () {
        if (!parsed) {
            for (int32_t i = 0; i < array_len; ++i) {
                values.push_back (TagInt::get_value (raw_buf, raw_len, raw_off));
            }
            parsed = true;
        }
        return values; }

    TagLongArray::TagLongArray (shared_ptr<unsigned char[]> buf,
                                size_t const len, size_t &off)
        : TagArray (TAG_LONG_ARRAY, buf, len, off) {
        array_len = TagInt::get_value (buf, len, off);
        raw_off = off;
        off += array_len * 8;
    }

    vector<int64_t> TagLongArray::operator* () {
        if (!parsed) {
            for (int32_t i = 0; i < array_len; ++i) {
                value.push_back (
                    TagLong::get_value (raw_buf, raw_len, raw_off));
            }
            parsed = true;
        }
        return value;
    }

    TagString::TagString (shared_ptr<unsigned char[]> buf, size_t const len,
                          size_t &off)
        : Tag (TAG_STRING, buf, len, off), value (nullptr) {
        str_len = TagShort::get_value (buf, len, off);
        raw_off = off;
        off += static_cast<size_t> (str_len);
        if (off >= len) {
            throw runtime_error ("off >= len in TagString");
        }
    }

    TagString::~TagString () { delete value; }

    string *TagString::operator* () {
        if (value == nullptr) {
            value =
                new string (reinterpret_cast<char *> (raw_buf.get () + raw_off),
                            static_cast<size_t> (str_len));
        }
        return value;
    }

    TagList::TagList (shared_ptr<unsigned char[]> buf, size_t const len,
                      size_t &off)
        : Tag (TAG_LIST, buf, len, off) {
        parse_buffer (buf, len, off);
    }

    TagList::~TagList () {
        for (auto itr = std::begin (tags); itr != std::end (tags); ++itr) {
            delete *itr;
        }
    }

    void TagList::parse_buffer (shared_ptr<unsigned char[]> buf,
                                size_t const len, size_t &off) {
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

    TagCompound::TagCompound (shared_ptr<unsigned char[]> buf, size_t const len,
                              size_t &off)
        : Tag (TAG_COMPOUND, buf, len, off) {
        parse_buffer (buf, len, off);
    }

    TagCompound::TagCompound () : Tag (TAG_COMPOUND) {}

    TagCompound::~TagCompound () {
        for (auto itr = std::begin (tags); itr != std::end (tags); ++itr) {
            delete itr->second;
        }
    }

    void TagCompound::parse_buffer (shared_ptr<unsigned char[]> buf,
                                    size_t const len, size_t &off) {
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

#include <cstdint>
#include <iterator>
#include <stdexcept>

#include "nbt.hh"
#include "utils.hh"

namespace nbt {
    namespace {
        Tag *tag_byte_factory (shared_ptr<unsigned char[]> buf, size_t len,
                               size_t &off) {
            return new TagByte (buf, len, off);
        }

        Tag *tag_short_factory (shared_ptr<unsigned char[]> buf, size_t len,
                                size_t &off) {
            return new TagShort (buf, len, off);
        }

        Tag *tag_int_factory (shared_ptr<unsigned char[]> buf, size_t len,
                              size_t &off) {
            return new TagInt (buf, len, off);
        }

        Tag *tag_long_factory (shared_ptr<unsigned char[]> buf, size_t len,
                               size_t &off) {
            return new TagLong (buf, len, off);
        }

        Tag *tag_float_factory (shared_ptr<unsigned char[]> buf, size_t len,
                                size_t &off) {
            return new TagFloat (buf, len, off);
        }

        Tag *tag_double_factory (shared_ptr<unsigned char[]> buf, size_t len,
                                 size_t &off) {
            return new TagDouble (buf, len, off);
        }

        Tag *tag_byte_array_factory (shared_ptr<unsigned char[]> buf,
                                     size_t len, size_t &off) {
            return new TagByteArray (buf, len, off);
        }

        Tag *tag_string_factory (shared_ptr<unsigned char[]> buf, size_t len,
                                 size_t &off) {
            return new TagString (buf, len, off);
        }

        Tag *tag_list_factory (shared_ptr<unsigned char[]> buf, size_t len,
                               size_t &off) {
            return new TagList (buf, len, off);
        }

        Tag *tag_compound_factory (shared_ptr<unsigned char[]> buf, size_t len,
                                   size_t &off) {
            return new TagCompound (buf, len, off, false);
        }

        Tag *tag_int_array_factory (shared_ptr<unsigned char[]> buf, size_t len,
                                    size_t &off) {
            return new TagIntArray (buf, len, off);
        }

        Tag *tag_long_array_factory (shared_ptr<unsigned char[]> buf,
                                     size_t len, size_t &off) {
            return new TagLongArray (buf, len, off);
        }

        function<Tag *(shared_ptr<unsigned char[]> buf, size_t len,
                       size_t &off)>
            tag_factories[] = {
                &tag_byte_factory,       &tag_short_factory,
                &tag_int_factory,        &tag_long_factory,
                &tag_float_factory,      &tag_double_factory,
                &tag_byte_array_factory, &tag_string_factory,
                &tag_list_factory,       &tag_compound_factory,
                &tag_int_array_factory,  &tag_long_array_factory};
    } // namespace

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
        return values;
    }

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
                values.push_back (
                    TagInt::get_value (raw_buf, raw_len, raw_off));
            }
            parsed = true;
        }
        return values;
    }

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
        : Tag (TAG_LIST, buf, len, off), parsed (false) {
        payload_type = TagByte::get_value (buf, len, off);
        list_len = TagInt::get_value (buf, len, off);

        if (list_len == 0) {
            parsed = true;
            return;
        }

        if (payload_type == TAG_BYTE_ARRAY || payload_type == TAG_STRING ||
            payload_type == TAG_LIST || payload_type == TAG_COMPOUND ||
            payload_type == TAG_INT_ARRAY || payload_type == TAG_LONG_ARRAY) {

            parse_buffer (buf, len, off);
            parsed = true;
            return;
        }

        raw_off = off;

        int size;
        if (payload_type == TAG_BYTE)
            size = 1;
        else if (payload_type == TAG_SHORT)
            size = 2;
        else if (payload_type == TAG_INT)
            size = 4;
        else if (payload_type == TAG_LONG)
            size = 8;
        else if (payload_type == TAG_FLOAT)
            size = 4;
        else if (payload_type == TAG_DOUBLE)
            size = 8;
        else
            throw runtime_error ("Unknown type of tag: " +
                                 to_string (payload_type));

        off += size * list_len;
    }

    TagList::~TagList () {
        if (!parsed) return;

        for (auto itr = std::begin (tags); itr != std::end (tags); ++itr) {
            delete *itr;
        }
    }

    void TagList::parse_buffer (shared_ptr<unsigned char[]> buf,
                                size_t const len, size_t &off) {
        Tag *tag;
        for (int32_t i = 0; i < list_len; ++i) {
            if (payload_type > 12) {
                throw runtime_error ("Unknown type of tag: " +
                                     to_string (payload_type));
            }

            tag = (tag_factories[payload_type - 1]) (buf, len, off);

            tags.push_back (tag);
        }
    }

    vector<nbt::Tag *> &TagList::operator* () {
        if (!parsed) {
            parse_buffer (raw_buf, raw_len, raw_off);
            parsed = true;
        }
        return tags;
    }

    TagCompound::TagCompound (shared_ptr<unsigned char[]> buf, size_t const len,
                              size_t &off, bool toplevel)
        : Tag (TAG_COMPOUND, buf, len, off), toplevel (toplevel) {
        if (!toplevel) parse_buffer (buf, len, off);
    }

    TagCompound::TagCompound () : Tag (TAG_COMPOUND), toplevel (false) {}

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
            bool next_toplevel = *name == "Level";
            if (next_toplevel) {
                /* for performance reason, it assumes that there is only one
                   Level tag in one chunk and no trailing tag after that.
                   it is dangerous but works correctly for now. */
                tag = new TagCompound (buf, len, off, true);
            } else {
                if (type > 12) {
                    throw runtime_error ("unknown type of tag: " +
                                         to_string (type));
                }

                tag = (tag_factories[type - 1]) (buf, len, off);
            }

            tags[*name] = tag;

            delete name;

            if (off >= len - 1 || next_toplevel) {
                break;
            }
        }
    }

    void TagCompound::parse_until (string &tag_name) {
        for (;;) {
            tagtype_t type = TagByte::get_value (raw_buf, raw_len, raw_off);

            if (type == TAG_END) {
                toplevel = false;
                break;
            }

            string *name = TagString::get_value (raw_buf, raw_len, raw_off);

            if (type > 12) {
                throw runtime_error ("unknown type of tag: " +
                                     to_string (type));
            }

            Tag *tag = (tag_factories[type - 1]) (raw_buf, raw_len, raw_off);

            tags[*name] = tag;

            if (*name == tag_name) {
                delete name;
                break;
            }

            delete name;

            if (raw_off >= raw_len - 1) {
                toplevel = false;
                break;
            }
        }
    }

    NBTFile::NBTFile (utils::DecompressedData *data)
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
} // namespace nbt

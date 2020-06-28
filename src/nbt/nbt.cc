/* NBT parser based on twoolie/NBT. */

#include <cstdint>
#include <functional>
#include <iterator>
#include <stdexcept>
#include <string>

#include "nbt.hh"
#include "utils.hh"

namespace pixel_terrain::nbt {
    namespace {
        tag *tag_byte_factory(std::shared_ptr<unsigned char[]> buf,
                              std::size_t len, std::size_t &off) {
            return new tag_byte(buf, len, off);
        }

        tag *tag_short_factory(std::shared_ptr<unsigned char[]> buf,
                               std::size_t len, std::size_t &off) {
            return new tag_short(buf, len, off);
        }

        tag *tag_int_factory(std::shared_ptr<unsigned char[]> buf,
                             std::size_t len, std::size_t &off) {
            return new tag_int(buf, len, off);
        }

        tag *tag_long_factory(std::shared_ptr<unsigned char[]> buf,
                              std::size_t len, std::size_t &off) {
            return new tag_long(buf, len, off);
        }

        tag *tag_float_factory(std::shared_ptr<unsigned char[]> buf,
                               std::size_t len, std::size_t &off) {
            return new tag_float(buf, len, off);
        }

        tag *tag_double_factory(std::shared_ptr<unsigned char[]> buf,
                                std::size_t len, std::size_t &off) {
            return new tag_double(buf, len, off);
        }

        tag *tag_byte_array_factory(std::shared_ptr<unsigned char[]> buf,
                                    std::size_t len, std::size_t &off) {
            return new tag_byte_array(buf, len, off);
        }

        tag *tag_string_factory(std::shared_ptr<unsigned char[]> buf,
                                std::size_t len, std::size_t &off) {
            return new tag_string(buf, len, off);
        }

        tag *tag_list_factory(std::shared_ptr<unsigned char[]> buf,
                              std::size_t len, std::size_t &off) {
            return new tag_list(buf, len, off);
        }

        tag *tag_compound_factory(std::shared_ptr<unsigned char[]> buf,
                                  std::size_t len, std::size_t &off) {
            return new tag_compound(buf, len, off, false);
        }

        tag *tag_int_array_factory(std::shared_ptr<unsigned char[]> buf,
                                   std::size_t len, std::size_t &off) {
            return new tag_int_array(buf, len, off);
        }

        tag *tag_long_array_factory(std::shared_ptr<unsigned char[]> buf,
                                    std::size_t len, std::size_t &off) {
            return new tag_long_array(buf, len, off);
        }

        std::function<tag *(std::shared_ptr<unsigned char[]> buf,
                            std::size_t len, std::size_t &off)>
            tag_factories[] = {
                &tag_byte_factory,       &tag_short_factory,
                &tag_int_factory,        &tag_long_factory,
                &tag_float_factory,      &tag_double_factory,
                &tag_byte_array_factory, &tag_string_factory,
                &tag_list_factory,       &tag_compound_factory,
                &tag_int_array_factory,  &tag_long_array_factory};
    } // namespace

    tag::tag(tagtype_t type, std::shared_ptr<unsigned char[]> buf,
             std::size_t len, std::size_t &off)
        : tag_type(type), raw_buf(move(buf)), raw_len(len), raw_off(off) {}

    tag::tag(tagtype_t type)
        : tag_type(type), raw_buf(nullptr), raw_len(0), raw_off(0) {}

    tag_primitive::tag_primitive(tagtype_t type,
                                 std::shared_ptr<unsigned char[]> buf,
                                 std::size_t len, std::size_t &off)
        : tag(type, buf, len, off), parsed(false) {}

    tag_byte::tag_byte(std::shared_ptr<unsigned char[]> buf,
                       std::size_t const len, std::size_t &off)
        : tag_primitive(TAG_BYTE, buf, len, off) {
        ++off;
    }

    unsigned char tag_byte::operator*() {
        if (!parsed) {
            value = get_value(raw_buf, raw_len, raw_off);
            parsed = true;
        }
        return value;
    }

    tag_short::tag_short(std::shared_ptr<unsigned char[]> buf,
                         std::size_t const len, std::size_t &off)
        : tag_primitive(TAG_SHORT, buf, len, off) {
        off += 2;
    }

    int16_t tag_short::operator*() {
        if (!parsed) {
            value = get_value(raw_buf, raw_len, raw_off);
            parsed = true;
        }
        return value;
    }

    tag_int::tag_int(std::shared_ptr<unsigned char[]> buf,
                     std::size_t const len, std::size_t &off)
        : tag_primitive(TAG_INT, buf, len, off) {
        off += 4;
    }

    std::int32_t tag_int::operator*() {
        if (!parsed) {
            value = get_value(raw_buf, raw_len, raw_off);
            parsed = true;
        }
        return value;
    }

    tag_long::tag_long(std::shared_ptr<unsigned char[]> buf,
                       std::size_t const len, std::size_t &off)
        : tag_primitive(TAG_LONG, buf, len, off) {
        off += 8;
    }

    std::uint64_t tag_long::operator*() {
        if (!parsed) {
            value = get_value(raw_buf, raw_len, raw_off);
            parsed = true;
        }
        return value;
    }

    tag_float::tag_float(std::shared_ptr<unsigned char[]> buf,
                         std::size_t const len, std::size_t &off)
        : tag_primitive(TAG_FLOAT, buf, len, off) {
        off += 4;
    }

    float tag_float::operator*() {
        if (!parsed) {
            value = get_value(raw_buf, raw_len, raw_off);
            parsed = true;
        }
        return value;
    }

    tag_double::tag_double(std::shared_ptr<unsigned char[]> buf,
                           std::size_t const len, std::size_t &off)
        : tag_primitive(TAG_DOUBLE, buf, len, off) {
        off += 8;
    }

    double tag_double::operator*() {
        if (!parsed) {
            value = get_value(raw_buf, raw_len, raw_off);
            parsed = true;
        }
        return value;
    }

    tag_array::tag_array(tagtype_t type, std::shared_ptr<unsigned char[]> buf,
                         std::size_t const len, std::size_t &off)
        : tag(type, buf, len, off), parsed(false) {}

    tag_byte_array::tag_byte_array(std::shared_ptr<unsigned char[]> buf,
                                   std::size_t const len, std::size_t &off)
        : tag_array(TAG_BYTE_ARRAY, buf, len, off) {
        array_len = tag_int::get_value(buf, len, off);
        raw_off = off;
        off += array_len;
    }

    std::vector<char> tag_byte_array::operator*() {
        if (!parsed) {
            for (int32_t i = 0; i < array_len; ++i) {
                values.push_back(*(raw_buf.get() + raw_off));

                ++raw_off;
                if (raw_off >= raw_len) {
                    throw std::out_of_range("off >= len in TagByteArray");
                }
            }
            parsed = true;
        }
        return values;
    }

    tag_int_array::tag_int_array(std::shared_ptr<unsigned char[]> buf,
                                 std::size_t const len, std::size_t &off)
        : tag_array(TAG_INT_ARRAY, buf, len, off) {
        array_len = tag_int::get_value(buf, len, off);
        raw_off = off;
        off += array_len * 4;
    }

    std::vector<std::int32_t> tag_int_array::operator*() {
        if (!parsed) {
            for (int32_t i = 0; i < array_len; ++i) {
                values.push_back(tag_int::get_value(raw_buf, raw_len, raw_off));
            }
            parsed = true;
        }
        return values;
    }

    tag_long_array::tag_long_array(std::shared_ptr<unsigned char[]> buf,
                                   std::size_t const len, std::size_t &off)
        : tag_array(TAG_LONG_ARRAY, buf, len, off) {
        array_len = tag_int::get_value(buf, len, off);
        raw_off = off;
        off += array_len * 8;
    }

    std::vector<std::int64_t> tag_long_array::operator*() {
        if (!parsed) {
            for (int32_t i = 0; i < array_len; ++i) {
                value.push_back(tag_long::get_value(raw_buf, raw_len, raw_off));
            }
            parsed = true;
        }
        return value;
    }

    tag_string::tag_string(std::shared_ptr<unsigned char[]> buf,
                           std::size_t const len, std::size_t &off)
        : tag(TAG_STRING, buf, len, off), value(nullptr) {
        str_len = tag_short::get_value(buf, len, off);
        raw_off = off;
        off += static_cast<size_t>(str_len);
        if (off >= len) {
            throw std::runtime_error("off >= len in TagString");
        }
    }

    tag_string::~tag_string() { delete value; }

    std::string *tag_string::operator*() {
        if (value == nullptr) {
            value = new std::string(
                reinterpret_cast<char *>(raw_buf.get() + raw_off),
                static_cast<std::size_t>(str_len));
        }
        return value;
    }

    tag_list::tag_list(std::shared_ptr<unsigned char[]> buf,
                       std::size_t const len, std::size_t &off)
        : tag(TAG_LIST, buf, len, off), parsed(false) {
        payload_type = tag_byte::get_value(buf, len, off);
        list_len = tag_int::get_value(buf, len, off);

        if (list_len == 0) {
            parsed = true;
            return;
        }

        if (payload_type == TAG_BYTE_ARRAY || payload_type == TAG_STRING ||
            payload_type == TAG_LIST || payload_type == TAG_COMPOUND ||
            payload_type == TAG_INT_ARRAY || payload_type == TAG_LONG_ARRAY) {

            parse_buffer(buf, len, off);
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
            throw std::runtime_error("Unknown type of tag: " +
                                     std::to_string(payload_type));

        off += size * list_len;
    }

    tag_list::~tag_list() {
        if (!parsed) return;

        for (auto itr = std::begin(tags); itr != std::end(tags); ++itr) {
            delete *itr;
        }
    }

    void tag_list::parse_buffer(std::shared_ptr<unsigned char[]> buf,
                                std::size_t const len, std::size_t &off) {
        tag *tag;
        for (std::int32_t i = 0; i < list_len; ++i) {
            if (payload_type > 12) {
                throw std::runtime_error("Unknown type of tag: " +
                                         std::to_string(payload_type));
            }

            tag = (tag_factories[payload_type - 1])(buf, len, off);

            tags.push_back(tag);
        }
    }

    std::vector<nbt::tag *> &tag_list::operator*() {
        if (!parsed) {
            parse_buffer(raw_buf, raw_len, raw_off);
            parsed = true;
        }
        return tags;
    }

    tag_compound::tag_compound(std::shared_ptr<unsigned char[]> buf,
                               std::size_t const len, std::size_t &off,
                               bool toplevel)
        : tag(TAG_COMPOUND, buf, len, off), toplevel(toplevel) {
        if (!toplevel) parse_buffer(buf, len, off);
    }

    tag_compound::tag_compound() : tag(TAG_COMPOUND), toplevel(false) {}

    tag_compound::~tag_compound() {
        for (auto itr = std::begin(tags); itr != std::end(tags); ++itr) {
            delete itr->second;
        }
    }

    void tag_compound::parse_buffer(std::shared_ptr<unsigned char[]> buf,
                                    std::size_t const len, std::size_t &off) {
        for (;;) {
            tagtype_t type = tag_byte::get_value(buf, len, off);

            if (type == TAG_END) break;

            std::string *name = tag_string::get_value(buf, len, off);
            tag *tag;
            bool next_toplevel = *name == "Level";
            if (next_toplevel) {
                /* for performance reason, it assumes that there is only one
                   Level tag in one chunk and no trailing tag after that.
                   it is dangerous but works correctly for now. */
                tag = new tag_compound(buf, len, off, true);
            } else {
                if (type > 12) {
                    throw std::runtime_error("unknown type of tag: " +
                                             std::to_string(type));
                }

                tag = (tag_factories[type - 1])(buf, len, off);
            }

            tags[*name] = tag;

            delete name;

            if (off >= len - 1 || next_toplevel) {
                break;
            }
        }
    }

    void tag_compound::parse_until(std::string &tag_name) {
        for (;;) {
            tagtype_t type = tag_byte::get_value(raw_buf, raw_len, raw_off);

            if (type == TAG_END) {
                toplevel = false;
                break;
            }

            std::string *name =
                tag_string::get_value(raw_buf, raw_len, raw_off);

            if (type > 12) {
                throw std::runtime_error("unknown type of tag: " +
                                         std::to_string(type));
            }

            tag *cur_tag = (tag_factories[type - 1])(raw_buf, raw_len, raw_off);

            tags[*name] = cur_tag;

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

    nbt_file::nbt_file(utils::decompressed_data *data)
        : tag_compound(), data(data) {
        parse_file();
    }

    nbt_file::~nbt_file() { delete data; }

    void nbt_file::parse_file() {
        std::size_t off = 0;

        tagtype_t type = tag_byte::get_value(data->data, data->len, off);

        if (type != tag_type) {
            throw std::invalid_argument("corrupted data file");
        }

        name = *tag_string::get_value(data->data, data->len, off);
        parse_buffer(data->data, data->len, off);
    }
} // namespace pixel_terrain::nbt

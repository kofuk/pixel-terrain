#include <stdexcept>

#include "../utils.hh"
#include "nbt_pull_parser.hh"

namespace pixel_terrain::nbt {
    namespace {
        inline unsigned char read_byte(unsigned char *data, size_t *offset) {
            unsigned char d = data[*offset];
            ++(*offset);
            return d;
        }

        std::int16_t read_short(unsigned char *data, size_t *offset) {
            std::int16_t d = utils::to_host_byte_order(
                *reinterpret_cast<std::int16_t *>(data + *offset));
            *offset += 2;
            return d;
        }

        inline std::int32_t read_int(unsigned char *data, size_t *offset) {
            std::int32_t d = utils::to_host_byte_order(
                *reinterpret_cast<std::int32_t *>(data + *offset));
            *offset += 4;
            return d;
        }

        inline std::int64_t read_long(unsigned char *data, size_t *offset) {
            std::int32_t d = utils::to_host_byte_order(
                *reinterpret_cast<std::int64_t *>(data + *offset));
            *offset += 8;
            return d;
        }

        inline float read_float(unsigned char *data, size_t *offset) {
            float d = utils::to_host_byte_order(
                *reinterpret_cast<float *>(data + *offset));
            *offset += 4;
            return d;
        }

        inline float read_double(unsigned char *data, size_t *offset) {
            double d = utils::to_host_byte_order(
                *reinterpret_cast<double *>(data + *offset));
            *offset += 8;
            return d;
        }

        inline std::string read_string(unsigned char *data, size_t *offset,
                                       int strlength) {
            std::string str(data + *offset, data + *offset + strlength);
            *offset += strlength;
            return str;
        }

        inline bool is_array_type(unsigned char tag_type) {
            return tag_type == TAG_BYTE_ARRAY || tag_type == TAG_INT_ARRAY ||
                   tag_type == TAG_LONG_ARRAY;
        }
    } // namespace

    nbt_pull_parser::nbt_pull_parser(std::shared_ptr<unsigned char[]> data,
                                     const size_t length)
        : data(data.get()), sp_data(data), length(length) {}

    nbt_pull_parser::nbt_pull_parser(unsigned char *data, const size_t length)
        : data(data), length(length) {}

    void nbt_pull_parser::parse_array_header() {
        if (offset + 4 > length) {
            throw std::out_of_range("buffer exhausted on array header");
        }
        std::int32_t len = read_int(data, &offset);
        lengths.push(len);
        indices.push(0);
    }

    parser_event nbt_pull_parser::parse_tag_header() {
        if (offset + 1 > length) {
            throw std::out_of_range("buffer exhausted on tag type");
        }
        unsigned char type = read_byte(data, &offset);

        if (type == TAG_END) {
            if (types.empty() || names.empty()) {
                throw std::runtime_error("too many closing tag");
            }
            tag_ended = true;
            handle_tag_end();
            end_emitted = true;
            current_event = parser_event::TAG_END;
            return current_event;
        }

        types.push(type);
        if (offset + 2 > length) {
            throw std::out_of_range("buffer exhausted on tag name length");
        }
        std::int16_t name_len = read_short(data, &offset);
        if (offset + name_len >= length) {
            throw std::out_of_range("buffer exhausted on tag name");
        }
        std::string name = read_string(data, &offset, name_len);
        names.push(name);
        if (is_array_type(type)) {
            parse_array_header();
        } else if (type == TAG_LIST) {
            parse_list_header();
        }

        tag_ended = false;

        current_event = parser_event::TAG_START;
        return current_event;
    }

    void nbt_pull_parser::parse_list_header() {
        if (offset + 1 > length) {
            throw std::out_of_range(
                "buffer exhausted on list header (tag type)");
        }
        payload_types.push(read_byte(data, &offset));
        if (offset + 4 > length) {
            throw std::out_of_range("buffer exhausted on list header (length)");
        }
        std::int32_t len = read_int(data, &offset);
        lengths.push(len);
        indices.push(0);
    }

    parser_event nbt_pull_parser::parse_list_data() {
        ++(indices.top());
        if (payload_types.empty()) {
            throw std::runtime_error("corrupted list (payload type missing)");
        }
        unsigned char payload_type = payload_types.top();
        if (payload_type == TAG_END) {
            tag_ended = true;
            payload_types.pop();
            handle_tag_end();
            current_event = parser_event::TAG_END;
            return current_event;
        }
        types.push(payload_type);
        names.push("");
        if (is_array_type(payload_type)) {
            parse_array_header();
        } else if (payload_type == TAG_LIST) {
            parse_list_header();
        }
        current_event = parser_event::TAG_START;
        return current_event;
    }

    void nbt_pull_parser::handle_tag_end() {
        last_tag_name = names.top();
        names.pop();
        if (!types.empty() && types.top() == TAG_STRING) {
            delete tag_data.string_data;
        }
        last_tag_type = types.top();
        types.pop();
        end_emitted = true;
    }

    parser_event nbt_pull_parser::next() noexcept(false) {
        if (offset >= length) {
            if (!names.empty()) {
                handle_tag_end();
                current_event = parser_event::TAG_END;
                return current_event;
            }
            current_event = parser_event::DOCUMENT_END;
            return current_event;
        }
        if (tag_ended) {
            if (!end_emitted) {
                handle_tag_end();
                current_event = parser_event::TAG_END;
                return current_event;
            }
            tag_ended = false;
            end_emitted = false;
            if (types.empty() || types.top() != TAG_LIST) {
                return parse_tag_header();
            }
        }
        if (types.empty()) {
            throw std::runtime_error(
                "parser is in tag, but header data missing");
        }
        unsigned char type = types.top();
        int tmp;
        switch (type) {
        case TAG_BYTE:
            if (offset + 1 > length) {
                throw std::out_of_range("buffer exhausted on tag byte (name: " +
                                        get_tag_name() + ")");
            }
            tag_data.byte_data = read_byte(data, &offset);
            tag_ended = true;
            break;

        case TAG_SHORT:
            if (offset + 2 > length) {
                throw std::out_of_range(
                    "buffer exhausted on tag short (name: " + get_tag_name() +
                    ")");
            }
            tag_data.short_data = read_short(data, &offset);
            tag_ended = true;
            break;

        case TAG_INT:
            if (offset + 4 > length) {
                throw std::out_of_range("buffer exhausted on tag int (name: " +
                                        get_tag_name() + ")");
            }
            tag_data.int_data = read_int(data, &offset);
            tag_ended = true;
            break;

        case TAG_LONG:
            if (offset + 8 > length) {
                throw std::out_of_range("buffer exhausted on tag long (name: " +
                                        get_tag_name() + ")");
            }
            tag_data.long_data = read_long(data, &offset);
            tag_ended = true;
            break;

        case TAG_FLOAT:
            if (offset + 4 > length) {
                throw std::out_of_range(
                    "buffer exhausted on tag float (name: " + get_tag_name() +
                    ")");
            }
            tag_data.float_data = read_float(data, &offset);
            tag_ended = true;
            break;

        case TAG_DOUBLE:
            if (offset + 8 > length) {
                throw std::out_of_range(
                    "buffer exhausted on tag double (name: " + get_tag_name() +
                    ")");
            }
            tag_data.double_data = read_double(data, &offset);
            tag_ended = true;
            break;

        case TAG_BYTE_ARRAY:
            if (indices.empty() || lengths.empty()) {
                throw std::runtime_error(
                    "corrupted array length on tag byte array (name: " +
                    get_tag_name() + ")");
            }
            if (indices.top() >= lengths.top()) {
                tag_ended = true;
                indices.pop();
                lengths.pop();
                handle_tag_end();
                current_event = parser_event::TAG_END;
                return current_event;
            }
            if (offset + 1 > length) {
                throw std::out_of_range(
                    "buffer exhausted on tag byte array (name: " +
                    get_tag_name() + ")");
            }
            tag_data.byte_data = read_byte(data, &offset);
            ++(indices.top());
            break;

        case TAG_STRING:
            if (offset + 2 > length) {
                throw std::out_of_range(
                    "buffer exhausted on tag string header (name: " +
                    get_tag_name() + ")");
            }
            tmp = read_short(data, &offset);
            if (offset + tmp > length) {
                throw std::out_of_range(
                    "buffer exhausted on tag string data (name: " +
                    get_tag_name() + ")");
            }
            tag_data.string_data =
                new std::string(read_string(data, &offset, tmp));
            tag_ended = true;
            break;

        case TAG_LIST:
            if (indices.empty() || lengths.empty()) {
                throw std::runtime_error(
                    "corrupted array length on tag list (name: " +
                    get_tag_name() + ")");
            }
            if (indices.top() >= lengths.top()) {
                tag_ended = true;
                indices.pop();
                lengths.pop();
                payload_types.pop();
                handle_tag_end();
                current_event = parser_event::TAG_END;
                return current_event;
            }
            return parse_list_data();

        case TAG_COMPOUND:
            return parse_tag_header();

        case TAG_INT_ARRAY:
            if (indices.empty() || lengths.empty()) {
                throw std::runtime_error(
                    "corrupted array length on tag int array (name: " +
                    get_tag_name() + ")");
            }
            if (indices.top() >= lengths.top()) {
                tag_ended = true;
                indices.pop();
                lengths.pop();
                handle_tag_end();
                current_event = parser_event::TAG_END;
                return current_event;
            }
            if (offset + 4 > length) {
                throw std::out_of_range(
                    "buffer exhausted on tag int array (name: " +
                    get_tag_name() + ")");
            }
            tag_data.int_data = read_int(data, &offset);
            ++(indices.top());
            break;

        case TAG_LONG_ARRAY:
            if (indices.empty() || lengths.empty()) {
                throw std::runtime_error(
                    "corrupted array length on tag long array (name: " +
                    get_tag_name() + ")");
            }
            if (indices.top() >= lengths.top()) {
                tag_ended = true;
                indices.pop();
                lengths.pop();
                handle_tag_end();
                current_event = parser_event::TAG_END;
                return current_event;
            }
            if (offset + 8 > length) {
                throw std::out_of_range(
                    "buffer exhausted on tag long array (name: " +
                    get_tag_name() + ")");
            }
            tag_data.long_data = read_long(data, &offset);
            ++(indices.top());

            break;
        }
        current_event = parser_event::DATA;
        return current_event;
    }

    parser_event nbt_pull_parser::get_event_type() noexcept {
        return current_event;
    }

    std::string nbt_pull_parser::get_tag_name() {
        if (current_event == parser_event::TAG_END) {
            return last_tag_name;
        }
        if (names.empty()) {
            throw std::logic_error("parser have not parsed any header");
        }
        return names.top();
    }

    unsigned char nbt_pull_parser::get_tag_type() {
        if (current_event == parser_event::TAG_END) {
            return last_tag_type;
        }

        if (types.empty()) {
            throw std::logic_error("parser have not parsed any header");
        }
        return types.top();
    }

    unsigned char nbt_pull_parser::get_byte() const {
        if (types.empty()) {
            throw std::logic_error(
                "You tried to get byte, but parser is not in any tags");
        }
        unsigned char type = types.top();
        if (type != TAG_BYTE && type != TAG_BYTE_ARRAY) {
            throw std::logic_error("Tried to get byte on other type of tag");
        }
        return tag_data.byte_data;
    }

    std::int16_t nbt_pull_parser::get_short() const {
        if (types.empty()) {
            throw std::logic_error(
                "You tried to get short, but parser is not in any tags");
        }
        unsigned char type = types.top();
        if (type != TAG_SHORT) {
            throw std::logic_error("Tried to get short on other type of tag");
        }
        return tag_data.short_data;
    }

    std::int32_t nbt_pull_parser::get_int() const {
        if (types.empty()) {
            throw std::logic_error(
                "You tried to get int, but parser is not in any tags");
        }
        unsigned char type = types.top();
        if (type != TAG_INT && type != TAG_INT_ARRAY) {
            throw std::logic_error("Tried to get int on other type of tag");
        }
        return tag_data.int_data;
    }

    std::int64_t nbt_pull_parser::get_long() const {
        if (types.empty()) {
            throw std::logic_error(
                "You tried to get long, but parser is not in any tags");
        }
        unsigned char type = types.top();
        if (type != TAG_LONG && type != TAG_LONG_ARRAY) {
            throw std::logic_error("Tried to get long on other type of tag");
        }
        return tag_data.long_data;
    }

    float nbt_pull_parser::get_float() const {
        if (types.empty()) {
            throw std::logic_error(
                "You tried to get float, but parser is not in any tags");
        }
        unsigned char type = types.top();
        if (type != TAG_FLOAT) {
            throw std::logic_error("Tried to get float on other type of tag");
        }
        return tag_data.float_data;
    }

    double nbt_pull_parser::get_double() const {
        if (types.empty()) {
            throw std::logic_error(
                "You tried to get double, but parser is not in any tags");
        }
        unsigned char type = types.top();
        if (type != TAG_DOUBLE) {
            throw std::logic_error("Tried to get double on other type of tag");
        }
        return tag_data.double_data;
    }

    std::string nbt_pull_parser::get_string() const {
        if (types.empty()) {
            throw std::logic_error(
                "You tried to get string, but parser is not in any tags");
        }
        unsigned char type = types.top();
        if (type != TAG_STRING) {
            throw std::logic_error("Tried to get string on other type of tag");
        }
        return *tag_data.string_data;
    }
} // namespace pixel_terrain::nbt

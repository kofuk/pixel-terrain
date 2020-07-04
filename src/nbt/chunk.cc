/* Class to access chunk data structure.
   This implementation based on matcool/anvil-parser with
   performance tuning and biome support. */

#include <cmath>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>

#include "../logger/logger.hh"
#include "chunk.hh"
#include "pull_parser/nbt_pull_parser.hh"

namespace pixel_terrain::anvil {
    chunk::chunk(std::shared_ptr<unsigned char[]> data, std::size_t length)
        : parser(nbt::nbt_pull_parser(data, length)) {
        palettes.fill(nullptr);
        block_states.fill(nullptr);
    }

    chunk::~chunk() {}

    std::uint64_t chunk::get_last_update() noexcept(false) {
        if (!(loaded_fields & FIELD_LAST_UPDATE)) {
            do {
                parse_fields();
            } while ((loaded_fields & FIELD_LAST_UPDATE) ||
                     parser.get_event_type() ==
                         nbt::parser_event::DOCUMENT_END);

            if (!(loaded_fields & FIELD_LAST_UPDATE)) {
                throw std::runtime_error("No LastUpdate tag");
            }
        }

        return last_update;
    }

    void chunk::parse_sections() {
        nbt::parser_event ev = parser.next();
        for (;;) {
            if (ev == nbt::parser_event::TAG_END) {
                break;
            }
            if (ev != nbt::parser_event::TAG_START) {
                throw std::runtime_error("Broken Sections");
            }
            unsigned char y = 255;
            /* parse first tag */
            ev = parser.next();
            if (ev == nbt::parser_event::TAG_END) {
                ev = parser.next();
                continue;
            }
            if (ev != nbt::parser_event::TAG_START) {
                throw std::runtime_error("Broken contents of Sections");
            }
            /* if it is "Y", it propably invalid element in Sections so we check
             * for it to skip early. */
            if (parser.get_tag_type() == nbt::TAG_BYTE &&
                parser.get_tag_name() == "Y") {
                ev = parser.next();
                y = parser.get_byte();
                if (y >= 16) {
                    /* we just throw away content in invalid element. */
                    for (int level = 2; level != 0;) {
                        ev = parser.next();
                        if (ev == nbt::parser_event::TAG_START) {
                            ++level;
                        } else if (ev == nbt::parser_event::TAG_END) {
                            --level;
                        }
                    }

                    ev = parser.next();
                    continue;
                } else {
                    ev = parser.next();
                }
            }
            std::vector<std::string> *palette = new std::vector<std::string>;
            std::vector<std::uint64_t> *block_state =
                new std::vector<std::uint64_t>;
            for (;;) {
                if (parser.get_tag_name() == "BlockStates") {
                    if (parser.get_tag_type() != nbt::TAG_LONG_ARRAY) {
                        throw std::runtime_error(
                            "BlockStates is not TAG_Long_Array");
                    }
                    for (; parser.next() == nbt::parser_event::DATA;) {
                        block_state->push_back(parser.get_long());
                    }
                } else if (parser.get_tag_name() == "Palette") {
                    if (parser.get_tag_type() != nbt::TAG_LIST) {
                        throw std::runtime_error("Palette is not TAG_List");
                    }
                    ev = parser.next();
                    for (; ev != nbt::parser_event::TAG_END;) {
                        ev = parser.next();
                        for (; ev != nbt::parser_event::TAG_END;) {
                            if (ev == nbt::parser_event::TAG_START) {
                                if (parser.get_tag_type() == nbt::TAG_STRING &&
                                    parser.get_tag_name() == "Name") {
                                    /* parser_event::DATA */
                                    ev = parser.next();
                                    palette->push_back(parser.get_string());
                                    /* parser_event::TAG_END */
                                    ev = parser.next();
                                } else {
                                    for (int i = 1; i != 0;) {
                                        ev = parser.next();
                                        if (ev ==
                                            nbt::parser_event::TAG_START) {
                                            ++i;
                                        } else if (ev ==
                                                   nbt::parser_event::TAG_END) {
                                            --i;
                                        }
                                    }
                                }
                            }
                            ev = parser.next();
                        }
                        ev = parser.next();
                    }
                } else if (parser.get_tag_name() == "Y") {
                    if (parser.get_tag_type() != nbt::TAG_BYTE) {
                        throw std::runtime_error("Y is not TAG_Byte");
                    }
                    /* update only if this tag was parsed above. */
                    if (ev != nbt::parser_event::TAG_END) {
                        ev = parser.next();
                        y = parser.get_byte();
                    }
                    ev = parser.next();
                } else {
                    /* skip others because they aren't needed. */
                    for (int i = 1; i != 0;) {
                        ev = parser.next();
                        if (ev == nbt::parser_event::TAG_START) {
                            ++i;
                        } else if (ev == nbt::parser_event::TAG_END) {
                            --i;
                        }
                    }
                }

                ev = parser.next();
                if (ev == nbt::parser_event::TAG_START) {
                    continue;
                } else if (ev == nbt::parser_event::TAG_END) {
                    break;
                } else {
                    throw std::runtime_error("Broken Sections tag");
                }
            }

            if (y >= 16) {
                delete palette;
                delete block_state;
            } else {
                palettes[y] = palette;
                block_states[y] = block_state;
            }

            ev = parser.next();
        }
    }

    void chunk::parse_fields() {
        nbt::parser_event ev = parser.get_event_type();
        while (ev != nbt::parser_event::DOCUMENT_END) {
            if (ev == nbt::parser_event::TAG_START) {
                tag_structure.push_back(parser.get_tag_name());

                unsigned char f = current_field();
                if (f == FIELD_SECTIONS) {
                    parse_sections();

                    return;
                } else if (f == FIELD_LAST_UPDATE) {
                    ev = parser.next();
                    if (ev != nbt::parser_event::DATA ||
                        parser.get_tag_type() != nbt::TAG_LONG) {
                        throw std::runtime_error("LastUpdate is not TAG_Long");
                    }
                    last_update = parser.get_long();

                    parser.next();
                    return;
                } else if (f == FIELD_BIOMES) {
                    if (parser.get_tag_type() != nbt::TAG_INT_ARRAY) {
                        throw std::runtime_error("Biomes is not TAG_Int_Array");
                    }
                    while (parser.next() == nbt::parser_event::DATA) {
                        biomes.push_back(parser.get_int());
                    }

                    return;
                }
            } else if (ev == nbt::parser_event::TAG_END) {
                tag_structure.pop_back();
            }

            ev = parser.next();
        }
    }

    void chunk::make_sure_field_parsed(unsigned char field) noexcept(false) {
        if (!(loaded_fields & field)) {
            do {
                parse_fields();
            } while (!(loaded_fields & field) &&
                     parser.get_event_type() !=
                         nbt::parser_event::DOCUMENT_END);

            if (!(loaded_fields & field)) {
                throw std::runtime_error("No tag not found: " +
                                         std::to_string(field));
            }
        }
    }

    unsigned char chunk::current_field() {
        if (tag_structure.size() < 3) {
            return 0;
        }

        if (tag_structure[0] == "" && tag_structure[1] == "Level") {
            if (tag_structure[2] == "Sections") {
                loaded_fields |= FIELD_SECTIONS;
                return FIELD_SECTIONS;
            } else if (tag_structure[2] == "LastUpdate") {
                loaded_fields |= FIELD_LAST_UPDATE;
                return FIELD_LAST_UPDATE;
            } else if (tag_structure[2] == "Biomes") {
                loaded_fields |= FIELD_BIOMES;
                return FIELD_BIOMES;
            }
        }

        return 0;
    }

    std::vector<std::string> *chunk::get_palette(unsigned char y) {
        if (y >= 16) {
            return nullptr;
        }

        make_sure_field_parsed(FIELD_SECTIONS);

        return palettes[y];
    }

    int32_t chunk::get_biome(int32_t x, int32_t y, int32_t z) {
        make_sure_field_parsed(FIELD_BIOMES);

        if (biomes.size() == 256) {
            return biomes[(z / 2) * 16 + (x / 2)];
        } else if (biomes.size() == 1024) {
            return biomes[(y / 64) * 256 + (z / 4) * 4 + (x / 4)];
        }
        return 0;
    }

    std::string chunk::get_block(int32_t x, int32_t y, int32_t z) {
        if (x < 0 || 15 < x || y < 0 || 255 < y || z < 0 || 15 < z) {
            return "";
        }

        unsigned char section_no = y / 16;

        y %= 16;

        std::vector<std::string> *palette = palettes[section_no];
        if (palette == nullptr) {
            return "minecraft:air";
        }

        int bits = 4;
        if (palette->size() - 1 > 15) {
            bits = palette->size() - 1;

            /* calculate next squared number, in squared numbers larger than
               BITS. if BITS is already squared in this step, calculate next
               one. */
            bits = bits | (bits >> 1);
            bits = bits | (bits >> 2);
            bits = bits | (bits >> 4);
            bits = bits | (bits >> 8);
            bits = bits | (bits >> 16);
            bits += 1;

            bits = log2(bits);
        }

        int index = y * 16 * 16 + z * 16 + x;
        make_sure_field_parsed(FIELD_SECTIONS);
        std::vector<std::uint64_t> *states = block_states[section_no];
        int state = index * bits / 64;

        if (static_cast<std::uint64_t>(state) >= states->size())
            return "minecraft:air";

        std::uint64_t data = (*states)[state];

        std::uint64_t shifted_data = data >> ((bits * index) % 64);

        if (64 - ((bits * index) % 64) < bits) {
            data = (*states)[state + 1];
            int leftover = (bits - ((state + 1) * 64 % bits)) % bits;
            shifted_data =
                ((data & (static_cast<std::int64_t>(pow(2, leftover)) - 1))
                 << (bits - leftover)) |
                shifted_data;
        }

        std::int64_t palette_id =
            shifted_data & (static_cast<std::int64_t>(pow(2, bits)) - 1);

        if (palette_id <= 0 ||
            (*palette).size() <= static_cast<std::size_t>(palette_id))
            return "minecraft:air";

        return (*palette)[palette_id];
    }

    int chunk::get_max_height() {
        make_sure_field_parsed(FIELD_SECTIONS);

        for (int y = 15; y >= 0; --y) {
            if (palettes[y] != nullptr) {
                return (y + 1) * 16 - 1;
            }
        }
        return 0;
    }

} // namespace pixel_terrain::anvil

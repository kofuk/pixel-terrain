// SPDX-License-Identifier: MIT

/* Class to access chunk data structure.
   This implementation based on matcool/anvil-parser with
   performance tuning and biome support. */

#include <cmath>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>

#include "logger/logger.hh"
#include "nbt/chunk.hh"
#include "nbt/constants.hh"
#include "nbt/pull_parser/nbt_pull_parser.hh"

namespace pixel_terrain::anvil {
    chunk::chunk(std::shared_ptr<unsigned char[]> data, // NOLINT
                 std::size_t length)
        : parser(nbt::nbt_pull_parser(std::move(data), length)) {
        palettes.fill(nullptr);
        block_states.fill(nullptr);
    }

    chunk::~chunk() {
        for (std::vector<std::string> *e : palettes) {
            delete e;
        }
        for (std::vector<std::uint64_t> *e : block_states) {
            delete e;
        }
    }

    auto chunk::get_last_update() noexcept(false) -> std::uint64_t {
        make_sure_field_parsed(FIELD_LAST_UPDATE);

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
            unsigned char y = nbt::biomes::CHUNK_MAX_Y;
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
                parser.next();
                y = parser.get_byte();
                if (y >= nbt::biomes::PALETTE_Y_MAX) {
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
                }
                ev = parser.next();
            }
            auto *palette = new std::vector<std::string>;
            auto *block_state = new std::vector<std::uint64_t>;
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
                                    parser.next();
                                    palette->push_back(parser.get_string());
                                    /* parser_event::TAG_END */
                                    parser.next();
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
                        parser.next();
                        y = parser.get_byte();
                        parser.next();
                    }
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
                }
                if (ev == nbt::parser_event::TAG_END) {
                    break;
                }

                throw std::runtime_error("Broken Sections tag");
            }

            if (y >= nbt::biomes::PALETTE_Y_MAX) {
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
                }
                if (f == FIELD_LAST_UPDATE) {
                    ev = parser.next();
                    if (ev != nbt::parser_event::DATA ||
                        parser.get_tag_type() != nbt::TAG_LONG) {
                        throw std::runtime_error("LastUpdate is not TAG_Long");
                    }
                    last_update = parser.get_long();

                    parser.next();
                    return;
                }
                if (f == FIELD_BIOMES) {
                    if (parser.get_tag_type() != nbt::TAG_INT_ARRAY) {
                        throw std::runtime_error("Biomes is not TAG_Int_Array");
                    }
                    while (parser.next() == nbt::parser_event::DATA) {
                        biomes.push_back(parser.get_int());
                    }

                    return;
                }
                if (f == FIELD_DATA_VERSION) {
                    ev = parser.next();
                    if (ev != nbt::parser_event::DATA ||
                        parser.get_tag_type() != nbt::TAG_INT) {
                        throw std::runtime_error(
                            "DataVersion is not TAG_Int_Array");
                    }
                    data_version = parser.get_int();

                    parser.next();
                    return;
                }
            } else if (ev == nbt::parser_event::TAG_END) {
                tag_structure.pop_back();
            }

            ev = parser.next();
        }
    }

    void chunk::make_sure_field_parsed(unsigned char field) noexcept(false) {
        if ((loaded_fields & field) == 0) {
            do {
                parse_fields();
            } while ((loaded_fields & field) == 0 &&
                     parser.get_event_type() !=
                         nbt::parser_event::DOCUMENT_END);

            if ((loaded_fields & field) == 0) {
                throw std::runtime_error("Tag not found: " +
                                         std::to_string(field));
            }
        }
    }

    auto chunk::current_field() -> unsigned char {
        if (tag_structure.size() < 2 || !tag_structure[0].empty()) {
            return 0;
        }

        unsigned char result = 0;

        if (tag_structure[1] == "DataVersion") {
            result = FIELD_DATA_VERSION;
        } else if (tag_structure.size() >= 3 && tag_structure[0].empty() &&
                   tag_structure[1] == "Level") {
            if (tag_structure[2] == "Sections") {
                result = FIELD_SECTIONS;
            } else if (tag_structure[2] == "LastUpdate") {
                result = FIELD_LAST_UPDATE;
            } else if (tag_structure[2] == "Biomes") {
                result = FIELD_BIOMES;
            }
        }
        loaded_fields |= result;

        return result;
    }

    auto chunk::get_palette(unsigned char y) -> std::vector<std::string> * {
        if (y >= nbt::biomes::PALETTE_Y_MAX) {
            return nullptr;
        }

        make_sure_field_parsed(FIELD_SECTIONS);

        return palettes[y];
    }

    auto chunk::get_biome(int32_t x, int32_t y, int32_t z) -> int32_t {
        make_sure_field_parsed(FIELD_BIOMES);

        if (biomes.size() == nbt::biomes::BIOME_DATA_OLD_VERSION_SIZE) {
            return biomes[(z / 2) * 16 + (x / 2)]; // NOLINT
        }
        if (biomes.size() == nbt::biomes::BIOME_DATA_NEW_VERSION_SIZE) {
            return biomes[(y / 64) * 256 + (z / 4) * 4 + (x / 4)]; // NOLINT
        }
        return 0;
    }

    auto chunk::get_block(int32_t x, int32_t y, int32_t z) -> std::string {
        if (x < 0 || 15 < x || y < 0 || 255 < y || z < 0 || 15 < z) { // NOLINT
            return "";
        }

        make_sure_field_parsed(FIELD_DATA_VERSION);

        unsigned char section_no = y / nbt::biomes::PALETTE_Y_MAX;

        y %= nbt::biomes::PALETTE_Y_MAX;

        std::vector<std::string> *palette = palettes[section_no];
        if (palette == nullptr || palette->empty()) {
            return "minecraft:air";
        }

        unsigned int bits = 4;
        if (palette->size() - 1 > 15) { // NOLINT
            bits = palette->size() - 1;

            /* calculate next squared number, in squared numbers larger than
               BITS. if BITS is already squared in this step, calculate next
               one. */
            bits = bits | (bits >> 1);
            bits = bits | (bits >> 2);
            bits = bits | (bits >> 4);
            bits = bits | (bits >> 8);  // NOLINT
            bits = bits | (bits >> 16); // NOLINT
            bits += 1;

            bits = static_cast<int>(log2(bits));
        }

        int index = y * 16 * 16 + z * 16 + x; // NOLINT
        make_sure_field_parsed(FIELD_SECTIONS);
        std::vector<std::uint64_t> *states = block_states[section_no];
        int state;
        bool stretches =
            data_version < nbt::biomes::NEED_STRETCH_DATA_VERSION_THRESHOLD;
        if (stretches) {
            state = index * bits / 64; // NOLINT
        } else {
            state = index / (64 / bits); // NOLINT
        }

        if (static_cast<std::uint64_t>(state) >= states->size()) {
            return "minecraft:air";
        }

        std::uint64_t data = (*states)[state];

        std::uint64_t shifted_data;
        if (stretches) {
            shifted_data = data >> ((bits * index) % 64); // NOLINT
        } else {
            shifted_data = data >> (index % (64 / bits) * bits); // NOLINT
        }

        if (stretches && 64 - ((bits * index) % 64) < bits) { // NOLINT
            data = (*states)[state + 1];

            int leftover = (bits - ((state + 1) * 64 % bits)) % bits; // NOLINT
            shifted_data =
                ((data & (static_cast<std::int64_t>(pow(2, leftover)) - 1))
                 << (bits - leftover)) |
                shifted_data;
        }

        std::int64_t palette_id =
            shifted_data & (static_cast<std::int64_t>(pow(2, bits)) - 1);

        if (palette_id <= 0 ||
            (*palette).size() <= static_cast<std::size_t>(palette_id)) {
            return "minecraft:air";
        }

        return (*palette)[palette_id];
    }

    auto chunk::get_max_height() -> int {
        make_sure_field_parsed(FIELD_SECTIONS);

        for (int y = nbt::biomes::SECTIONS_Y_DIV_COUNT - 1; y >= 0; --y) {
            if (palettes[y] != nullptr) {
                return (y + 1) * nbt::biomes::BLOCK_PER_SECTION - 1;
            }
        }
        return 0;
    }

} // namespace pixel_terrain::anvil

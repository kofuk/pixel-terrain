/* Class to access chunk data structure.
   This implementation based on matcool/anvil-parser with
   performance tuning and biome support. */

#include <cmath>
#include <cstdint>
#include <stdexcept>

#include "../logger/logger.hh"
#include "chunk.hh"
#include "nbt.hh"

namespace pixel_terrain::anvil {
    chunk::chunk(nbt::nbt_file *nbt_data) : nbt_file(nbt_data) {
        data = nbt_data->get_as<nbt::tag_compound, nbt::TAG_COMPOUND>("Level"s);
        if (data == nullptr) {
            throw runtime_error("Level tag not found in chunk"s);
        }

        last_update = nbt::value<uint64_t>(
            data->get_as<nbt::tag_long, nbt::TAG_LONG>("LastUpdate"s));

        palettes.fill(nullptr);
    }

    chunk::~chunk() { delete nbt_file; }

    void chunk::parse_fields() {
        palettes.fill(nullptr);
        nbt::tag_list *section =
            data->get_as<nbt::tag_list, nbt::TAG_LIST>("Sections"s);
        if (section == nullptr) {
            throw runtime_error("Sections tag not found in Level"s);
        }

        for (auto itr = begin(**section); itr != end(**section); ++itr) {
            if ((*itr)->tag_type != nbt::TAG_COMPOUND) {
                throw runtime_error("Sections' payload is not TAG_COMPOUND"s);
            }

            unsigned char tag_y;
            try {
                tag_y = nbt::value<unsigned char>(
                    (static_cast<nbt::tag_compound *>(*itr))
                        ->get_as<nbt::tag_byte, nbt::TAG_BYTE>("Y"s));
            } catch (runtime_error const &) {
                continue;
            }
            if (15 < tag_y) {
                continue;
            }

            palettes[tag_y] =
                get_palette(static_cast<nbt::tag_compound *>(*itr));
        }

        nbt::tag_int_array *biomes_tag =
            data->get_as<nbt::tag_int_array, nbt::TAG_INT_ARRAY>("Biomes"s);
        if (biomes_tag != nullptr) {
            biomes = **biomes_tag;
        }
    }

    nbt::tag_compound *chunk::get_section(unsigned char y) {
        if (15 < y) {
            return nullptr;
        }

        nbt::tag_list *section =
            data->get_as<nbt::tag_list, nbt::TAG_LIST>("Sections"s);
        if (section == nullptr) {
            throw runtime_error("Sections tag not found in Level"s);
        }

        for (auto itr = begin(**section); itr != end(**section); ++itr) {
            if ((*itr)->tag_type != nbt::TAG_COMPOUND) {
                throw runtime_error("Sections' payload is not TAG_COMPOUND"s);
            }

            unsigned char tag_y;
            try {
                tag_y = nbt::value<unsigned char>(
                    (static_cast<nbt::tag_compound *>(*itr))
                        ->get_as<nbt::tag_byte, nbt::TAG_BYTE>("Y"s));
            } catch (runtime_error const &) {
                continue;
            }

            if (tag_y == y) return static_cast<nbt::tag_compound *>(*itr);
        }

        return nullptr;
    }

    vector<string> *chunk::get_palette(nbt::tag_compound *section) {
        vector<string> *palette = new vector<string>;

        nbt::tag_list *palette_tag_list =
            section->get_as<nbt::tag_list, nbt::TAG_LIST>("Palette"s);
        if (palette_tag_list == nullptr) {
            return nullptr;
        }
        if (palette_tag_list->payload_type != nbt::TAG_COMPOUND) {
            throw runtime_error(
                "corrupted data (payload type != TAG_COMPOUND)"s);
        }

        for (auto itr = begin(**palette_tag_list);
             itr != end(**palette_tag_list); ++itr) {
            nbt::tag_compound *tag = static_cast<nbt::tag_compound *>(*itr);

            string *name = nbt::value<string *>(
                tag->get_as<nbt::tag_string, nbt::TAG_STRING>("Name"s));

            palette->push_back(*name);
        }

        return palette;
    }

    int32_t chunk::get_biome(int32_t x, int32_t y, int32_t z) {
        if (biomes.size() == 256) {
            return biomes[(z / 2) * 16 + (x / 2)];
        } else if (biomes.size() == 1024) {
            return biomes[(y / 64) * 256 + (z / 4) * 4 + (x / 4)];
        }
        return 0;
    }

    string chunk::get_block(int32_t x, int32_t y, int32_t z) {
        if (x < 0 || 15 < x || y < 0 || 255 < y || z < 0 || 15 < z) {
            return ""s;
        }

        unsigned char section_no = y / 16;
        nbt::tag_compound *section = get_section(section_no);

        y %= 16;

        vector<string> *palette = palettes[section_no];
        if (palette == nullptr) {
            return "minecraft:air"s;
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
        vector<int64_t> states = nbt::value<vector<int64_t>>(
            section->get_as<nbt::tag_long_array, nbt::TAG_LONG_ARRAY>(
                "BlockStates"s));
        int state = index * bits / 64;

        if (static_cast<uint64_t>(state) >= states.size())
            return "minecraft:air"s;

        uint64_t data = states[state];

        uint64_t shifted_data = data >> ((bits * index) % 64);

        if (64 - ((bits * index) % 64) < bits) {
            data = states[state + 1];
            int leftover = (bits - ((state + 1) * 64 % bits)) % bits;
            shifted_data =
                ((data & (static_cast<int64_t>(pow(2, leftover)) - 1))
                 << (bits - leftover)) |
                shifted_data;
        }

        int64_t palette_id =
            shifted_data & (static_cast<int64_t>(pow(2, bits)) - 1);

        if (palette_id <= 0 ||
            (*palette).size() <= static_cast<size_t>(palette_id))
            return "minecraft:air"s;

        return (*palette)[palette_id];
    }

    int chunk::get_max_height() {
        for (int y = 15; y >= 0; --y) {
            if (palettes[y] != nullptr) {
                return (y + 1) * 16 - 1;
            }
        }
        return 0;
    }

} // namespace pixel_terrain::anvil

#include <cmath>
#include <cstdint>
#include <stdexcept>

#include "../logger/logger.hh"
#include "Chunk.hh"
#include "nbt.hh"

namespace anvil {
    Chunk::Chunk (nbt::NBTFile *nbt_data) : nbt_file (nbt_data) {
        data = nbt_data->get_as<nbt::TagCompound, nbt::TAG_COMPOUND> ("Level"s);
        if (data == nullptr) {
            throw runtime_error ("Level tag not found in chunk"s);
        }

        last_update = nbt::value<uint64_t> (
            data->get_as<nbt::TagLong, nbt::TAG_LONG> ("LastUpdate"s));

        palettes.fill (nullptr);
    }

    Chunk::~Chunk () { delete nbt_file; }

    void Chunk::parse_fields () {
        palettes.fill (nullptr);
        nbt::TagList *section =
            data->get_as<nbt::TagList, nbt::TAG_LIST> ("Sections"s);
        if (section == nullptr) {
            throw runtime_error ("Sections tag not found in Level"s);
        }

        for (auto itr = begin (**section); itr != end (**section); ++itr) {
            if ((*itr)->tag_type != nbt::TAG_COMPOUND) {
                throw runtime_error ("Sections' payload is not TAG_COMPOUND"s);
            }

            unsigned char tag_y;
            try {
                tag_y = nbt::value<unsigned char> (
                    (static_cast<nbt::TagCompound *> (*itr))
                        ->get_as<nbt::TagByte, nbt::TAG_BYTE> ("Y"s));
            } catch (runtime_error const &) {
                continue;
            }
            if (15 < tag_y) {
                continue;
            }

            palettes[tag_y] =
                get_palette (static_cast<nbt::TagCompound *> (*itr));
        }

        nbt::TagIntArray *biomes_tag =
            data->get_as<nbt::TagIntArray, nbt::TAG_INT_ARRAY> ("Biomes"s);
        if (biomes_tag != nullptr) {
            biomes = **biomes_tag;
        }
    }

    nbt::TagCompound *Chunk::get_section (unsigned char y) {
        if (15 < y) {
            return nullptr;
        }

        nbt::TagList *section =
            data->get_as<nbt::TagList, nbt::TAG_LIST> ("Sections"s);
        if (section == nullptr) {
            throw runtime_error ("Sections tag not found in Level"s);
        }

        for (auto itr = begin (**section); itr != end (**section); ++itr) {
            if ((*itr)->tag_type != nbt::TAG_COMPOUND) {
                throw runtime_error ("Sections' payload is not TAG_COMPOUND"s);
            }

            unsigned char tag_y;
            try {
                tag_y = nbt::value<unsigned char> (
                    (static_cast<nbt::TagCompound *> (*itr))
                        ->get_as<nbt::TagByte, nbt::TAG_BYTE> ("Y"s));
            } catch (runtime_error const &) {
                continue;
            }

            if (tag_y == y) return static_cast<nbt::TagCompound *> (*itr);
        }

        return nullptr;
    }

    vector<string> *Chunk::get_palette (nbt::TagCompound *section) {
        vector<string> *palette = new vector<string>;

        nbt::TagList *palette_tag_list =
            section->get_as<nbt::TagList, nbt::TAG_LIST> ("Palette"s);
        if (palette_tag_list == nullptr) {
            return nullptr;
        }
        if (palette_tag_list->payload_type != nbt::TAG_COMPOUND) {
            throw runtime_error (
                "corrupted data (payload type != TAG_COMPOUND)"s);
        }

        for (auto itr = begin (**palette_tag_list);
             itr != end (**palette_tag_list); ++itr) {
            nbt::TagCompound *tag = static_cast<nbt::TagCompound *> (*itr);

            string *src_name = nbt::value<string *> (
                tag->get_as<nbt::TagString, nbt::TAG_STRING> ("Name"s));

            string name;
            if (src_name->find ("minecraft:"s) == 0) {
                name = src_name->substr (10);
            } else {
                name = *src_name;
            }
            palette->push_back (name);
        }

        return palette;
    }

    int32_t Chunk::get_biome (int32_t x, int32_t y, int32_t z) {
        if (biomes.size () == 256) {
            return biomes[(z / 2) * 16 + (x / 2)];
        } else if (biomes.size () == 1024) {
            return biomes[(y / 64) * 256 + (z / 4) * 4 + (x / 4)];
        }
        return 0;
    }

    string Chunk::get_block (int32_t x, int32_t y, int32_t z) {
        if (x < 0 || 15 < x || y < 0 || 255 < y || z < 0 || 15 < z) {
            return ""s;
        }

        unsigned char section_no = y / 16;
        nbt::TagCompound *section = get_section (section_no);

        y %= 16;

        vector<string> *palette = palettes[section_no];
        if (palette == nullptr) {
            return "air"s;
        }

        int bits = 4;
        if (palette->size () - 1 > 15) {
            bits = palette->size () - 1;

            /* calculate next squared number, in squared numbers larger than
               BITS. if BITS is already squared in this step, calculate next
               one. */
            bits = bits | (bits >> 1);
            bits = bits | (bits >> 2);
            bits = bits | (bits >> 4);
            bits = bits | (bits >> 8);
            bits = bits | (bits >> 16);
            bits += 1;

            bits = log2 (bits);
        }

        int index = y * 16 * 16 + z * 16 + x;
        vector<int64_t> states = nbt::value<vector<int64_t>> (
            section->get_as<nbt::TagLongArray, nbt::TAG_LONG_ARRAY> (
                "BlockStates"s));
        int state = index * bits / 64;

        if (static_cast<uint64_t> (state) >= states.size ()) return "air"s;

        uint64_t data = states[state];

        uint64_t shifted_data = data >> ((bits * index) % 64);

        if (64 - ((bits * index) % 64) < bits) {
            data = states[state + 1];
            int leftover = (bits - ((state + 1) * 64 % bits)) % bits;
            shifted_data =
                ((data & (static_cast<int64_t> (pow (2, leftover)) - 1))
                 << (bits - leftover)) |
                shifted_data;
        }

        int64_t palette_id =
            shifted_data & (static_cast<int64_t> (pow (2, bits)) - 1);

        if (palette_id <= 0 ||
            (*palette).size () <= static_cast<size_t> (palette_id))
            return "air"s;

        return (*palette)[palette_id];
    }

    int Chunk::get_max_height () {
        for (int y = 15; y >= 0; --y) {
            if (palettes[y] != nullptr) {
                return (y + 1) * 16 - 1;
            }
        }
        return 0;
    }

} // namespace anvil

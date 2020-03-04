#include <cmath>
#include <cstdint>
#include <stdexcept>

#include "Chunk.hh"
#include "NBT.hh"

namespace Anvil {
    Chunk::Chunk (NBT::NBTFile *nbt_data) : nbt_file (nbt_data) {
        data = nbt_data->get_as<NBT::TagCompound, NBT::TAG_COMPOUND> ("Level");
        if (data == nullptr) {
            throw runtime_error ("Level tag not found in chunk");
        }

        last_update = NBT::value<uint64_t> (
            data->get_as<NBT::TagLong, NBT::TAG_LONG> ("LastUpdate"));

        palettes.fill (nullptr);
    }

    Chunk::~Chunk () { delete nbt_file; }

    void Chunk::parse_palette () {
        palettes.fill (nullptr);
        NBT::TagList *section =
            data->get_as<NBT::TagList, NBT::TAG_LIST> ("Sections");
        if (section == nullptr) {
            throw runtime_error ("Sections tag not found in Level");
        }

        for (auto itr = begin (section->tags); itr != end (section->tags);
             ++itr) {
            if ((*itr)->tag_type != NBT::TAG_COMPOUND) {
                throw runtime_error ("Sections' payload is not TAG_COMPOUND");
            }

            unsigned char tag_y;
            try {
                tag_y = NBT::value<unsigned char> (
                    (static_cast<NBT::TagCompound *> (*itr))
                        ->get_as<NBT::TagByte, NBT::TAG_BYTE> ("Y"));
            } catch (runtime_error const &) {
                continue;
            }
            if (15 < tag_y) {
                continue;
            }

            palettes[tag_y] =
                get_palette (static_cast<NBT::TagCompound *> (*itr));
        }
    }

    NBT::TagCompound *Chunk::get_section (unsigned char y) {
        if (15 < y) {
            return nullptr;
        }

        NBT::TagList *section =
            data->get_as<NBT::TagList, NBT::TAG_LIST> ("Sections");
        if (section == nullptr) {
            throw runtime_error ("Sections tag not found in Level");
        }

        for (auto itr = begin (section->tags); itr != end (section->tags);
             ++itr) {
            if ((*itr)->tag_type != NBT::TAG_COMPOUND) {
                throw runtime_error ("Sections' payload is not TAG_COMPOUND");
            }

            unsigned char tag_y;
            try {
                tag_y = NBT::value<unsigned char> (
                    (static_cast<NBT::TagCompound *> (*itr))
                        ->get_as<NBT::TagByte, NBT::TAG_BYTE> ("Y"));
            } catch (runtime_error const &) {
                continue;
            }

            if (tag_y == y) return static_cast<NBT::TagCompound *> (*itr);
        }

        return nullptr;
    }

    vector<string> *Chunk::get_palette (NBT::TagCompound *section) {
        vector<string> *palette = new vector<string>;

        NBT::TagList *palette_tag_list =
            section->get_as<NBT::TagList, NBT::TAG_LIST> ("Palette");
        if (palette_tag_list == nullptr) {
            return nullptr;
        }
        if (palette_tag_list->payload_type != NBT::TAG_COMPOUND) {
            throw runtime_error (
                "corrupted data (payload type != TAG_COMPOUND)");
        }

        for (auto itr = begin (palette_tag_list->tags);
             itr != end (palette_tag_list->tags); ++itr) {
            NBT::TagCompound *tag = static_cast<NBT::TagCompound *> (*itr);

            string *src_name = NBT::value<string *> (
                tag->get_as<NBT::TagString, NBT::TAG_STRING> ("Name"));

            string name;
            if (src_name->find ("minecraft:") == 0) {
                name = src_name->substr (10);
            } else {
                name = *src_name;
            }
            palette->push_back (name);
        }

        return palette;
    }

    string Chunk::get_block (int32_t x, int32_t y, int32_t z) {
        if (x < 0 || 15 < x || y < 0 || 255 < y || z < 0 || 15 < z) {
            return "";
        }

        unsigned char section_no = y / 16;
        NBT::TagCompound *section = get_section (section_no);

        y %= 16;

        vector<string> *palette = palettes[section_no];
        if (palette == nullptr) {
            return "air";
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
        vector<int64_t> states = NBT::value<vector<int64_t>> (
            section->get_as<NBT::TagLongArray, NBT::TAG_LONG_ARRAY> (
                "BlockStates"));
        int state = index * bits / 64;

        if (static_cast<uint64_t> (state) >= states.size ()) return "air";

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
            return "air";

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

} // namespace Anvil

#include <cassert>
#include <cmath>
#include <cstdint>

#include "Chunk.hh"
#include "NBT.hh"

namespace Anvil {
    Chunk::Chunk(NBT::NBTFile *nbt_data)
        : nbt_file(nbt_data), cache_palette(nullptr),
          cache_palette_section(-1) {
        NBT::TagInt *version_tag = (NBT::TagInt *)((*nbt_data)["DataVersion"]);
        assert(version_tag->tag_type == NBT::TAG_INT);
        version = version_tag->value;

        data = (NBT::TagCompound *)((*nbt_data)["Level"]);
        assert(data->tag_type == NBT::TAG_COMPOUND);

        NBT::TagLong *last_update_tag = (NBT::TagLong *)((*data)["LastUpdate"]);
        assert(last_update_tag->tag_type == NBT::TAG_LONG);
        last_update = last_update_tag->value;

        NBT::TagInt *x_tag = (NBT::TagInt *)((*data)["xPos"]);
        assert(x_tag->tag_type == NBT::TAG_INT);
        x = x_tag->value;

        NBT::TagInt *z_tag = (NBT::TagInt *)((*data)["zPos"]);
        assert(x_tag->tag_type == NBT::TAG_INT);
        z = z_tag->value;
    }

    Chunk::~Chunk() {
        if (cache_palette != nullptr) {
            for (auto itr = begin(*cache_palette); itr != end(*cache_palette);
                 ++itr) {
                delete *itr;
            }

            delete cache_palette;

            cache_palette = nullptr;
            cache_palette_section = -1;
        }

        delete nbt_file;
    }

    NBT::TagCompound *Chunk::get_section(unsigned char y) {
        if (y < 0 || 15 < y) {
            return nullptr;
        }

        NBT::TagList *section = (NBT::TagList *)(*data)["Sections"];
        assert(section->tag_type == NBT::TAG_LIST);

        for (auto itr = begin(section->tags); itr != end(section->tags);
             ++itr) {
            assert((*itr)->tag_type == NBT::TAG_COMPOUND);
            NBT::TagByte *y_tag =
                (NBT::TagByte *)((*(NBT::TagCompound *)(*itr))["Y"]);
            assert(y_tag->tag_type == NBT::TAG_BYTE);

            if (y_tag->value == y) return (NBT::TagCompound *)(*itr);
        }

        return nullptr;
    }

    vector<string *> *Chunk::get_palette(NBT::TagCompound *section) {
        vector<string *> *palette = new vector<string *>;

        NBT::TagList *palette_tag_list =
            (NBT::TagList *)((*section)["Palette"]);
        assert(palette_tag_list->tag_type == NBT::TAG_LIST);
        assert(palette_tag_list->payload_type == NBT::TAG_COMPOUND);

        for (auto itr = begin(palette_tag_list->tags);
             itr != end(palette_tag_list->tags); ++itr) {
            NBT::TagCompound *tag = (NBT::TagCompound *)(*itr);

            NBT::TagString *name_tag = (NBT::TagString *)((*tag)["Name"]);
            assert(name_tag->tag_type == NBT::TAG_STRING);

            string *src_name = name_tag->value;
            string *name;
            if (src_name->find("minecraft:") == 0) {
                name = new string(src_name->substr(10));
            } else {
                name = new string(*src_name);
            }
            palette->push_back(name);
        }

        return palette;
    }

    string Chunk::get_block(int32_t x, int32_t y, int32_t z) {
        if (x < 0 || 15 < x || y < 0 || 255 < y || z < 0 || 15 < z) {
            return "";
        }

        unsigned char section_no = y / 16;
        NBT::TagCompound *section = get_section(section_no);

        y %= 16;

        if (section == nullptr || (*section)["BlockStates"] == nullptr) {
            return "air";
        }
        vector<string *> *palette;
        if (cache_palette_section != section_no || cache_palette == nullptr) {
            if (cache_palette != nullptr) {
                for (auto itr = begin(*cache_palette);
                     itr != end(*cache_palette); ++itr) {
                    delete *itr;
                }
                delete cache_palette;
            }

            cache_palette = get_palette(section);
            cache_palette_section = section_no;
        }

        palette = cache_palette;

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
        NBT::TagLongArray *states_tag =
            (NBT::TagLongArray *)((*section)["BlockStates"]);
        assert(states_tag->tag_type == NBT::TAG_LONG_ARRAY);
        vector<int64_t> states = states_tag->values;
        int state = index * bits / 64;

        if ((uint64_t)state >= states.size()) return "air";

        uint64_t data = states[state];
        if (data < 0) data += pow(2, 64);

        uint64_t shifted_data = data >> ((bits * index) % 64);

        if (64 - ((bits * index) % 64) < bits) {
            data = states[state + 1];
            if (data < 0) data += pow(2, 64);
            int leftover = (bits - ((state + 1) * 64 % bits)) % bits;
            shifted_data =
                ((data & (int64_t)pow(2, leftover) - 1) << (bits - leftover)) |
                shifted_data;
        }

        int64_t palette_id = shifted_data & (int64_t)pow(2, bits) - 1;

        if (palette_id <= 0 || (*palette).size() <= (size_t)palette_id)
            return "air";

        return *(*palette)[palette_id];
    }

} // namespace Anvil

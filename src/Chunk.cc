#include <cassert>
#include <cmath>
#include <cstdint>

#include "Chunk.hh"
#include "NBT.hh"

namespace Anvil {
    Chunk::Chunk(NBT::NBTFile *nbt_data) {
        NBT::Tag *version_tag = (*nbt_data)["DataVersion"];
        assert(version_tag->tag_type == NBT::TAG_INT);
        version = ((NBT::TagInt *)version_tag)->value;

        NBT::Tag *data_tag = (*nbt_data)["Level"];
        assert(data_tag->tag_type == NBT::TAG_COMPOUND);
        data = (NBT::TagCompound *)data_tag;

        NBT::Tag *x_tag = (*data)["xPos"];
        assert(x_tag->tag_type == NBT::TAG_INT);
        x = ((NBT::TagInt *)x_tag)->value;

        NBT::Tag *z_tag = (*data)["zPos"];
        assert(x_tag->tag_type == NBT::TAG_INT);
        z = ((NBT::TagInt *)z_tag)->value;
    }

    NBT::TagCompound *Chunk::get_section(unsigned char y) {
        if (y < 0 || 15 < y) {
            return nullptr;
        }

        NBT::Tag *section_tag = (*data)["Sections"];
        assert(section_tag->tag_type == NBT::TAG_LIST);
        NBT::TagList *section = (NBT::TagList *)section_tag;

        for (auto itr = std::begin(section->tags);
             itr != std::end(section->tags); ++itr) {
            assert((*itr)->tag_type == NBT::TAG_COMPOUND);
            NBT::Tag *y_tag = (*(NBT::TagCompound *)(*itr))["Y"];
            assert(y_tag->tag_type == NBT::TAG_BYTE);

            if (((NBT::TagByte *)y_tag)->value == y)
                return (NBT::TagCompound *)(*itr);
        }

        return nullptr;
    }

    vector<string *> *Chunk::get_palette(NBT::TagCompound *section) {
        vector<string *> *palette = new vector<string *>;

        NBT::Tag *palette_tag = (*section)["Palette"];
        assert(palette_tag->tag_type == NBT::TAG_LIST);
        NBT::TagList *palette_tag_list = (NBT::TagList *)palette_tag;
        assert(palette_tag_list->payload_type == NBT::TAG_COMPOUND);

        for (auto itr = std::begin(palette_tag_list->tags);
             itr != std::end(palette_tag_list->tags); ++itr) {
            NBT::TagCompound *tag = (NBT::TagCompound *)(*itr);

            NBT::Tag *name_tag = (*tag)["Name"];
            assert(name_tag->tag_type == NBT::TAG_STRING);

            string *src_name = ((NBT::TagString *)name_tag)->value;
            string *name;
            if (src_name->find("minecraft:") == 0) {
                name = new string(src_name->substr(10));
            } else {
                name = src_name;
            }
            palette->push_back(name);
        }

        return palette;
    }

    string Chunk::get_block(int32_t x, int32_t y, int32_t z) {
        if (x < 0 || 15 < x || y < 0 || 255 < y || z < 0 || 15 < z) {
            return "";
        }

        NBT::TagCompound *section = get_section(y / 16);

        y %= 16;

        if (section == nullptr || (*section)["BlockStates"] == nullptr) {
            return "air";
        }

        vector<string *> *palette = get_palette(section);

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
        NBT::Tag *states_tag = (*section)["BlockStates"];
        assert(states_tag->tag_type == NBT::TAG_LONG_ARRAY);
        vector<int64_t> states = ((NBT::TagLongArray *)states_tag)->values;
        int state = index * bits / 64;
        uint64_t data = states[state];
        if (data < 0) data += pow(2, 64);

        uint64_t shifted_data = data >> ((bits * index) % 64);

        if (64 - ((bits * index) % 64) < bits) {
            data = states[state + 1];
            if (data < 0) data += pow(2, 64);
            int leftover = (bits - ((state + 1) * 64 % bits)) % bits;
            shifted_data =
                ((data & (int64_t)pow(2, leftover - 1)) << (bits - leftover)) |
                shifted_data;
        }

        int64_t palette_id = shifted_data & (int64_t)pow(2, bits) - 1;

        string result = *(*palette)[palette_id];
        for (auto itr = std::begin(*palette); itr != std::end(*palette);
             ++itr) {
            delete *itr;
        }

        return result;
    }

} // namespace Anvil

#include <cstdint>
#include <cstring>

#include "blocks.hh"

#include "block/block_colors_data.hh"

using namespace std;

unordered_map<string_view, uint32_t> colors;

void init_block_list () {
    size_t off = 0;
    for (;;) {
        size_t len =
            strlen (reinterpret_cast<char *> (block_colors_data) + off);
        if (len == 0) {
            break;
        }
        string_view block_name (
            reinterpret_cast<char *> (block_colors_data) + off, len);
        off += len + 1;
        colors[block_name] =
            *reinterpret_cast<uint32_t *> (block_colors_data + off);
        off += 4;
    }
}

bool is_biome_overridden (string const &block) {
    /* first, check if block is water or grass_block because the most
     * blocks are one of them. */
    if (block == "water" || block == "grass_block") return true;

    /* next, capture grass, large_grass, seagrass and large_seagrass. */
    if (block.size () >= 5 && block.find ("grass") == block.size () - 5)
        return true;

    /* next, check if block is leaf-family. */
    if (block.find ("leaves") != string::npos) return true;

    /* capture remains */
    if (block == "fern" || block == "large_fern" || block == "vine" ||
        block == "bubble_column")
        return true;

    return false;
}

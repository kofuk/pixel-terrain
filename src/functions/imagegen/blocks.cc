/* Construct block-color mappng from embeded data.
   And decide that the block should be overridden by per-biome color. */

#include <cstdint>
#include <cstring>
#include <string>

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
    if (block == "water"s || block == "grass_block"s) return true;

    /* next, capture grass, large_grass, seagrass and large_seagrass. */
    if (block.size () >= 5 && block.find ("grass"s) == block.size () - 5)
        return true;

    /* next, check if block is leaf-family. */
    if (block.find ("leaves"s) != string::npos) return true;

    /* capture remains */
    if (block == "fern"s || block == "large_fern"s || block == "vine"s ||
        block == "bubble_column"s)
        return true;

    return false;
}

#include <cstdint>

#include "blocks.hh"

unordered_map<string, uint32_t> colors;

void init_block_list () {
#include "../block/require_all.hh"
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

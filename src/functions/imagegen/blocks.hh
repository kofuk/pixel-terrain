#ifndef BLOCKS_HH
#define BLOCKS_HH

#include <array>
#include <cstdint>
#include <string_view>
#include <unordered_map>

using namespace std;

namespace mcmap {
    extern unordered_map<string_view, uint32_t> colors;

    void init_block_list();
    bool is_biome_overridden(string const &block);
} // namespace mcmap

#endif

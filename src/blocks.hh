#ifndef BLOCKS_HH
#define BLOCKS_HH

#include <array>
#include <cstdint>
#include <string>
#include <unordered_map>

using namespace std;

extern unordered_map<string, uint32_t> colors;

void init_block_list ();
bool is_biome_overridden (string const &block);

#endif

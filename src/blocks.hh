#ifndef BLOCKS_HH
#define BLOCKS_HH

#include <array>
#include <string>
#include <unordered_map>

using namespace std;

extern unordered_map<string, array<unsigned char, 3>> colors;

void init_block_list();

#endif

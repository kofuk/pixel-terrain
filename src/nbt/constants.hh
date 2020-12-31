// SPDX-License-Identifier: MIT

#ifndef NBT_CONSTANTS_HH
#define NBT_CONSTANTS_HH

#include <cstdint>

namespace pixel_terrain::nbt {
    namespace biomes {
        inline constexpr std::int32_t OCEAN = 0;
        inline constexpr std::int32_t DEEP_OCEAN = 24;
        inline constexpr std::int32_t FROZEN_OCEAN = 10;
        inline constexpr std::int32_t DEEP_FROZEN_OCEAN = 50;
        inline constexpr std::int32_t COLD_OCEAN = 46;
        inline constexpr std::int32_t DEEP_COLD_OCEAN = 49;
        inline constexpr std::int32_t LUKEWARM_OCEAN = 45;
        inline constexpr std::int32_t DEEP_LUKEWARM_OCEAN = 48;
        inline constexpr std::int32_t WARM_OCEAN = 44;
        inline constexpr std::int32_t DEEP_WARM_OCEAN = 47;
        inline constexpr std::int32_t RIVER = 7;
        inline constexpr std::int32_t FROZEN_RIVER = 11;
        inline constexpr std::int32_t BEACH = 16;
        inline constexpr std::int32_t STONE_SHORE = 25;
        inline constexpr std::int32_t SNOWY_BEACH = 26;
        inline constexpr std::int32_t FOREST = 4;
        inline constexpr std::int32_t WOODED_HILLS = 18;
        inline constexpr std::int32_t FLOWER_FOREST = 132;
        inline constexpr std::int32_t BIRCH_FOREST = 27;
        inline constexpr std::int32_t BIRCH_FOREST_HILLS = 28;
        inline constexpr std::int32_t TALL_BIRCH_FOREST = 155;
        inline constexpr std::int32_t TALL_BIRCH_HILLS = 156;
        inline constexpr std::int32_t DARK_FOREST = 29;
        inline constexpr std::int32_t DARK_FOREST_HILLS = 157;
        inline constexpr std::int32_t JUNGLE = 21;
        inline constexpr std::int32_t JUNGLE_HILLS = 22;
        inline constexpr std::int32_t MODIFIED_JUNGLE = 149;
        inline constexpr std::int32_t JUNGLE_EDGE = 23;
        inline constexpr std::int32_t MODIFIED_JUNGLE_EDGE = 151;
        inline constexpr std::int32_t BAMBOO_JUNGLE = 168;
        inline constexpr std::int32_t BAMBOO_JUNGLE_HILLS = 169;
        inline constexpr std::int32_t TAIGA = 5;
        inline constexpr std::int32_t TAIGA_HILLS = 19;
        inline constexpr std::int32_t TAIGA_MOUNTAINS = 133;
        inline constexpr std::int32_t SNOWY_TAIGA = 30;
        inline constexpr std::int32_t SNOWY_TAIGA_HILLS = 31;
        inline constexpr std::int32_t SNOWY_TAIGA_MOUNTAINS = 158;
        inline constexpr std::int32_t GIANT_TREE_TAIGA = 32;
        inline constexpr std::int32_t GIANT_TREE_TAIGA_HILLS = 33;
        inline constexpr std::int32_t GIANT_SPRUCE_TAIGA = 160;
        inline constexpr std::int32_t GIANT_SPRUCE_TAIGA_HILLS = 161;
        inline constexpr std::int32_t MUSHROOM_FIELDS = 14;
        inline constexpr std::int32_t MUSHROOM_FIELD_SHORE = 15;
        inline constexpr std::int32_t SWAMP = 6;
        inline constexpr std::int32_t SWAMP_HILLS = 134;
        inline constexpr std::int32_t SAVANNA = 35;
        inline constexpr std::int32_t SAVANNA_PLATEAU = 36;
        inline constexpr std::int32_t SHATTERED_SAVANNA = 163;
        inline constexpr std::int32_t SHATTERED_SAVANNA_PLATEAU = 164;
        inline constexpr std::int32_t PLAINS = 1;
        inline constexpr std::int32_t SUNFLOWER_PLAINS = 129;
        inline constexpr std::int32_t DESERT = 2;
        inline constexpr std::int32_t DESERT_HILLS = 17;
        inline constexpr std::int32_t DESERT_LAKES = 130;
        inline constexpr std::int32_t SNOWY_TUNDRA = 12;
        inline constexpr std::int32_t SNOWY_MOUNTAINS = 13;
        inline constexpr std::int32_t ICE_SPIKES = 140;
        inline constexpr std::int32_t MOUNTAINS = 3;
        inline constexpr std::int32_t WOODED_MOUNTAINS = 34;
        inline constexpr std::int32_t GRAVELLY_MOUNTAINS = 131;
        inline constexpr std::int32_t MODIFIED_GRAVELLY_MOUNTAINS = 162;
        inline constexpr std::int32_t MOUNTAIN_EDGE = 20;
        inline constexpr std::int32_t BADLANDS = 37;
        inline constexpr std::int32_t BADLANDS_PLATEAU = 39;
        inline constexpr std::int32_t MODIFIED_BADLANDS_PLATEAU = 167;
        inline constexpr std::int32_t WOODED_BADLANDS_PLATEAU = 38;
        inline constexpr std::int32_t MODIFIED_WOODED_BADLANDS_PLATEAU = 166;
        inline constexpr std::int32_t ERODED_BADLANDS = 165;
        inline constexpr std::int32_t NETHER_WASTES = 8;
        inline constexpr std::int32_t CRIMSON_FOREST = 171;
        inline constexpr std::int32_t WARPED_FOREST = 172;
        inline constexpr std::int32_t SOUL_SAND_VALLEY = 170;
        inline constexpr std::int32_t THE_END = 9;
        inline constexpr std::int32_t SMALL_END_ISLANDS = 40;
        inline constexpr std::int32_t END_MIDLANDS = 41;
        inline constexpr std::int32_t END_HIGHLANDS = 42;
        inline constexpr std::int32_t END_BARRENS = 43;
        inline constexpr std::int32_t THE_VOID = 127;

        namespace overrides {
            inline constexpr std::uint_fast32_t SWAMP = 0x665956ff;
            inline constexpr std::uint_fast32_t JUNGLE = 0x83bd7eff;
            inline constexpr std::uint_fast32_t SAVANNA = 0xa8ab33ff;
        } // namespace overrides

        inline constexpr int CHUNK_WIDTH = 16;
        inline constexpr int CHUNK_MAX_Y = 255;
        inline constexpr int CHUNK_MAX_Y_NETHER = 127;
        inline constexpr int CHUNK_PER_REGION_WIDTH = 32;
        inline constexpr int BLOCK_PER_REGION_WIDTH = 512;

        inline constexpr int BIOME_DATA_OLD_VERSION_SIZE = 256;
        inline constexpr int BIOME_DATA_NEW_VERSION_SIZE = 1024;

        inline constexpr int PALETTE_Y_MAX = 16;
        inline constexpr int SECTIONS_Y_DIV_COUNT = 16;
        inline constexpr int BLOCK_PER_SECTION = 16;

        inline constexpr int BLOCK_STATES_COUNT = 16;

        inline constexpr int NEED_STRETCH_DATA_VERSION_THRESHOLD = 2529;
    } // namespace biomes
} // namespace pixel_terrain::nbt

#endif

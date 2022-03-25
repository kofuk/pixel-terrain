// SPDX-License-Identifier: MIT

#ifndef CONSTANTS_HH
#define CONSTANTS_HH

#include <cstdint>

namespace pixel_terrain::graphics {
    namespace color {
        inline constexpr unsigned int R_OFFSET = 24;
        inline constexpr unsigned int G_OFFSET = 16;
        inline constexpr unsigned int B_OFFSET = 8;
        inline constexpr unsigned int A_OFFSET = 0;
        inline constexpr unsigned int CHAN_MASK = 0xff;

        inline constexpr std::uint8_t CHAN_FULL = 255;
        inline constexpr std::uint8_t CHAN_MIN = 0;
    } // namespace color
} // namespace pixel_terrain::graphics

#endif

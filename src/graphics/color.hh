// SPDX-License-Identifier: MIT

#ifndef COLOR_HH
#define COLOR_HH

#include <cstdint>

#include "graphics/constants.hh"

namespace pixel_terrain::graphics {
    static constexpr auto alpha(std::uint_fast32_t color)
        -> std::uint_fast32_t {
        return (color >> color::A_OFFSET) & color::CHAN_MASK;
    }

    auto blend_color(std::uint_fast32_t fg, std::uint_fast32_t bg)
        -> std::uint_fast32_t;
    auto blend_color(std::uint_fast32_t source, std::uint_fast32_t overlay,
                     double opacity) -> std::uint_fast32_t;
    auto increase_brightness(std::uint_fast32_t color, int amount)
        -> std::uint_fast32_t;
} // namespace pixel_terrain::graphics

#endif

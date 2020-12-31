// SPDX-License-Identifier: MIT

/* Utilities for color blending, changing brightness, and so on. */

#include <algorithm>
#include <cstdint>

#include "graphics/color.hh"
#include "graphics/constants.hh"
#include "utils/array.hh"

namespace pixel_terrain::graphics {
    auto blend_color(std::uint_fast32_t fg, std::uint_fast32_t bg)
        -> std::uint_fast32_t {
        using namespace color;

        if ((bg & CHAN_MASK) == 0) {
            return fg;
        }

        std::uint_fast8_t b_r = (bg >> R_OFFSET) & CHAN_MASK;
        std::uint_fast8_t b_g = (bg >> G_OFFSET) & CHAN_MASK;
        std::uint_fast8_t b_b = (bg >> B_OFFSET) & CHAN_MASK;
        std::uint_fast8_t b_a = (bg << A_OFFSET) & CHAN_MASK;

        std::uint_fast8_t f_r = (fg >> R_OFFSET) & CHAN_MASK;
        std::uint_fast8_t f_g = (fg >> G_OFFSET) & CHAN_MASK;
        std::uint_fast8_t f_b = (fg >> B_OFFSET) & CHAN_MASK;
        std::uint_fast8_t f_a = (fg >> A_OFFSET) & CHAN_MASK;

        std::uint_fast8_t new_a = (b_a + f_a) - b_a * f_a / CHAN_FULL;

        std::uint_fast8_t r =
            ((f_r * f_a + b_r * (CHAN_FULL - f_a) * b_a / CHAN_FULL) / new_a);
        std::uint_fast8_t g =
            ((f_g * f_a + b_g * (CHAN_FULL - f_a) * b_a / CHAN_FULL) / new_a);
        std::uint_fast8_t b =
            ((f_b * f_a + b_b * (CHAN_FULL - f_a) * b_a / CHAN_FULL) / new_a);

        return ((r & CHAN_MASK) << R_OFFSET) | ((g & CHAN_MASK) << G_OFFSET) |
               ((b & CHAN_MASK) << B_OFFSET) |
               ((new_a & CHAN_MASK) << A_OFFSET);
    }

    auto blend_color(std::uint_fast32_t source, std::uint_fast32_t overlay,
                     double opacity) -> std::uint_fast32_t {
        using namespace color;

        if (opacity == 0) {
            return source;
        }

        std::uint_fast8_t b_r = (source >> R_OFFSET) & CHAN_MASK;
        std::uint_fast8_t b_g = (source >> G_OFFSET) & CHAN_MASK;
        std::uint_fast8_t b_b = (source >> B_OFFSET) & CHAN_MASK;
        std::uint_fast8_t a = (source >> A_OFFSET) & CHAN_MASK;

        std::uint_fast8_t f_r = (overlay >> R_OFFSET) & CHAN_MASK;
        std::uint_fast8_t f_g = (overlay >> G_OFFSET) & CHAN_MASK;
        std::uint_fast8_t f_b = (overlay >> B_OFFSET) & CHAN_MASK;

        std::uint_fast8_t r = b_r + (f_r - b_r) * opacity;
        std::uint_fast8_t g = b_g + (f_g - b_g) * opacity;
        std::uint_fast8_t b = b_b + (f_b - b_b) * opacity;

        return ((r & CHAN_MASK) << R_OFFSET) | ((g & CHAN_MASK) << G_OFFSET) |
               ((b & CHAN_MASK) << B_OFFSET) | ((a & CHAN_MASK) << A_OFFSET);
    }

    auto increase_brightness(std::uint_fast32_t color, int amount)
        -> std::uint_fast32_t {
        using namespace color;

        auto c = pixel_terrain::make_array<uint_fast8_t>(
            static_cast<std::uint_fast8_t>((color >> R_OFFSET) & CHAN_MASK),
            static_cast<std::uint_fast8_t>((color >> G_OFFSET) & CHAN_MASK),
            static_cast<std::uint_fast8_t>((color >> B_OFFSET) & CHAN_MASK),
            static_cast<std::uint_fast8_t>((color >> A_OFFSET) & CHAN_MASK));
        if (amount > 0) {
            for (int i = 0; i < 3; ++i) {
                c[i] = std::min<int>(c[i] + amount, CHAN_FULL);
            }
        } else {
            for (int i = 0; i < 3; ++i) {
                c[i] = std::max<int>(c[i] + amount, CHAN_MIN);
            }
        }

        return ((c[0] & CHAN_MASK) << R_OFFSET) |
               ((c[1] & CHAN_MASK) << G_OFFSET) |
               ((c[2] & CHAN_MASK) << B_OFFSET) |
               ((c[3] & CHAN_MASK) << A_OFFSET);
    }
} // namespace pixel_terrain::graphics

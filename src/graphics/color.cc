/*
 * Copyright (c) 2020 Koki Fukuda
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* Utilities for color blending, changing brightness, and so on. */

#include <algorithm>
#include <cstdint>

#include "graphics/color.hh"

namespace pixel_terrain::image {
    std::uint_fast32_t blend_color(std::uint_fast32_t fg,
                                   std::uint_fast32_t bg) {
        if ((bg & 0xff) == 0) {
            return fg;
        }

        std::uint_fast8_t b_r = (bg >> 24) & 0xff;
        std::uint_fast8_t b_g = (bg >> 16) & 0xff;
        std::uint_fast8_t b_b = (bg >> 8) & 0xff;
        std::uint_fast8_t b_a = bg & 0xff;

        std::uint_fast8_t f_r = (fg >> 24) & 0xff;
        std::uint_fast8_t f_g = (fg >> 16) & 0xff;
        std::uint_fast8_t f_b = (fg >> 8) & 0xff;
        std::uint_fast8_t f_a = fg & 0xff;

        std::uint_fast8_t new_a = (b_a + f_a) - b_a * f_a / 255;

        std::uint_fast8_t r =
            ((f_r * f_a + b_r * (255 - f_a) * b_a / 255) / new_a);
        std::uint_fast8_t g =
            ((f_g * f_a + b_g * (255 - f_a) * b_a / 255) / new_a);
        std::uint_fast8_t b =
            ((f_b * f_a + b_b * (255 - f_a) * b_a / 255) / new_a);

        return ((r & 0xff) << 24) | ((g & 0xff) << 16) | ((b & 0xff) << 8) |
               (new_a & 0xff);
    }

    std::uint_fast32_t blend_color(std::uint_fast32_t source,
                                   std::uint_fast32_t overlay, float opacity) {
        if (opacity == 0) return source;

        std::uint_fast8_t b_r = (source >> 24) & 0xff;
        std::uint_fast8_t b_g = (source >> 16) & 0xff;
        std::uint_fast8_t b_b = (source >> 8) & 0xff;
        std::uint_fast8_t a = source & 0xff;

        std::uint_fast8_t f_r = (overlay >> 24) & 0xff;
        std::uint_fast8_t f_g = (overlay >> 16) & 0xff;
        std::uint_fast8_t f_b = (overlay >> 8) & 0xff;

        std::uint_fast8_t r = b_r + (f_r - b_r) * opacity;
        std::uint_fast8_t g = b_g + (f_g - b_g) * opacity;
        std::uint_fast8_t b = b_b + (f_b - b_b) * opacity;

        return ((r & 0xff) << 24) | ((g & 0xff) << 16) | ((b & 0xff) << 8) |
               (a & 0xff);
    }

    std::uint_fast32_t increase_brightness(std::uint_fast32_t color,
                                           int amount) {
        int c[4] = {static_cast<std::uint_fast8_t>((color >> 24) & 0xff),
                    static_cast<std::uint_fast8_t>((color >> 16) & 0xff),
                    static_cast<std::uint_fast8_t>((color >> 8) & 0xff),
                    static_cast<std::uint_fast8_t>(color & 0xff)};
        if (amount > 0) {
            for (int i = 0; i < 3; ++i) {
                c[i] = std::min<int>(c[i] + amount, 255);
            }
        } else {
            for (int i = 0; i < 3; ++i) {
                c[i] = std::max<int>(c[i] + amount, 0);
            }
        }

        return ((c[0] & 0xff) << 24) | ((c[1] & 0xff) << 16) |
               ((c[2] & 0xff) << 8) | (c[3] & 0xff);
    }
} // namespace pixel_terrain::image

/* Utilities for color blending, changing brightness, and so on. */

#include "color.hh"

#include <algorithm>
#include <cstdint>

using namespace std;

namespace pixel_terrain::commands::generate {
    uint_fast32_t blend_color(uint_fast32_t fg, uint_fast32_t bg) {
        if ((bg & 0xff) == 0) {
            return fg;
        }

        uint_fast8_t b_r = (bg >> 24) & 0xff;
        uint_fast8_t b_g = (bg >> 16) & 0xff;
        uint_fast8_t b_b = (bg >> 8) & 0xff;
        uint_fast8_t b_a = bg & 0xff;

        uint_fast8_t f_r = (fg >> 24) & 0xff;
        uint_fast8_t f_g = (fg >> 16) & 0xff;
        uint_fast8_t f_b = (fg >> 8) & 0xff;
        uint_fast8_t f_a = fg & 0xff;

        uint_fast8_t new_a = (b_a + f_a) - b_a * f_a / 255;

        uint_fast8_t r = ((f_r * f_a + b_r * (255 - f_a) * b_a / 255) / new_a);
        uint_fast8_t g = ((f_g * f_a + b_g * (255 - f_a) * b_a / 255) / new_a);
        uint_fast8_t b = ((f_b * f_a + b_b * (255 - f_a) * b_a / 255) / new_a);

        return ((r & 0xff) << 24) | ((g & 0xff) << 16) | ((b & 0xff) << 8) |
               (new_a & 0xff);
    }

    uint_fast32_t blend_color(uint_fast32_t source, uint_fast32_t overlay,
                              float opacity) {
        if (opacity == 0) return source;

        uint_fast8_t b_r = (source >> 24) & 0xff;
        uint_fast8_t b_g = (source >> 16) & 0xff;
        uint_fast8_t b_b = (source >> 8) & 0xff;
        uint_fast8_t a = source & 0xff;

        uint_fast8_t f_r = (overlay >> 24) & 0xff;
        uint_fast8_t f_g = (overlay >> 16) & 0xff;
        uint_fast8_t f_b = (overlay >> 8) & 0xff;

        uint_fast8_t r = b_r + (f_r - b_r) * opacity;
        uint_fast8_t g = b_g + (f_g - b_g) * opacity;
        uint_fast8_t b = b_b + (f_b - b_b) * opacity;

        return ((r & 0xff) << 24) | ((g & 0xff) << 16) | ((b & 0xff) << 8) |
               (a & 0xff);
    }

    uint_fast32_t increase_brightness(uint_fast32_t color, int amount) {
        int c[4] = {static_cast<uint_fast8_t>((color >> 24) & 0xff),
                    static_cast<uint_fast8_t>((color >> 16) & 0xff),
                    static_cast<uint_fast8_t>((color >> 8) & 0xff),
                    static_cast<uint_fast8_t>(color & 0xff)};
        if (amount > 0) {
            for (int i = 0; i < 3; ++i) {
                c[i] = min<int>(c[i] + amount, 255);
            }
        } else {
            for (int i = 0; i < 3; ++i) {
                c[i] = max<int>(c[i] + amount, 0);
            }
        }

        return ((c[0] & 0xff) << 24) | ((c[1] & 0xff) << 16) |
               ((c[2] & 0xff) << 8) | (c[3] & 0xff);
    }
} // namespace pixel_terrain::commands::generate

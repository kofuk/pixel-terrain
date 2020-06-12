#ifndef COLOR_HH
#define COLOR_HH

#include <cstdint>

namespace pixel_terrain::generator {
    uint_fast32_t blend_color(uint_fast32_t fg, uint_fast32_t bg);
    uint_fast32_t blend_color(uint_fast32_t source, uint_fast32_t overlay,
                              float opacity);
    uint_fast32_t increase_brightness(uint_fast32_t color, int amount);
} // namespace pixel_terrain::generator

#endif

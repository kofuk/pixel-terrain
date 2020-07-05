#ifndef COLOR_HH
#define COLOR_HH

#include <cstdint>

namespace pixel_terrain::image {
    std::uint_fast32_t blend_color(std::uint_fast32_t fg,
                                   std::uint_fast32_t bg);
    std::uint_fast32_t blend_color(std::uint_fast32_t source,
                                   std::uint_fast32_t overlay, float opacity);
    std::uint_fast32_t increase_brightness(std::uint_fast32_t color,
                                           int amount);
} // namespace pixel_terrain::image

#endif

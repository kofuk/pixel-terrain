// SPDX-License-Identifier: MIT

#ifndef GRAPHICS_PNG_HH
#define GRAPHICS_PNG_HH

#include <cstdint>
#include <filesystem>
#include <string>

#include <png.h>

#include "utils/path_hack.hh"

namespace pixel_terrain::graphics {
    class png {
        unsigned int width;
        unsigned int height;
        std::filesystem::path path;
        ::png_bytep data;

    public:
        png(int width, int height);
        png(std::filesystem::path const &path);
        ~png();

        [[nodiscard]] auto get_width() const -> unsigned int;
        [[nodiscard]] auto get_height() const -> unsigned int;
        void fit(unsigned int width, unsigned int height);
        void set_pixel(unsigned int x, unsigned int y,
                       std::uint_fast32_t color);
        auto get_pixel(int x, int y) -> std::uint_fast32_t;
        void clear(int x, int y);
        auto save(std::filesystem::path const &path) -> bool;
        auto save() -> bool;
    };
} // namespace pixel_terrain::graphics

#endif

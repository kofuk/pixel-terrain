#ifndef PNG_HH
#define PNG_HH

#include <cstdint>
#include <filesystem>
#include <string>

#include <pngconf.h>

#include "../utils/path_hack.hh"

namespace pixel_terrain::image {
    class png {
        int width;
        int height;
        std::filesystem::path path;
        ::png_bytep data;

    public:
        png(int width, int height);
        png(std::filesystem::path path);
        ~png();

        int get_width();
        int get_height();
        void set_pixel(int x, int y, std::uint_fast32_t color);
        std::uint_fast32_t get_pixel(int x, int y);
        void clear(int x, int y);
        bool save(std::filesystem::path path);
        bool save();
    };
} // namespace pixel_terrain::image

#endif

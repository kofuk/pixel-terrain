#ifndef PNG_HH
#define PNG_HH

#include <cstdint>
#include <pngconf.h>
#include <string>

namespace pixel_terrain::commands::generate {
    class png {
        int width;
        int height;
        std::string filename;
        ::png_bytep data;

    public:
        png(int width, int height);
        png(std::string filename);
        ~png();

        int get_width();
        int get_height();
        void set_pixel(int x, int y, std::uint_fast32_t color);
        std::uint_fast32_t get_pixel(int x, int y);
        void clear(int x, int y);
        bool save(std::string filename);
        bool save();
    };
} // namespace pixel_terrain::commands::generate

#endif

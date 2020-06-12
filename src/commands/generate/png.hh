#ifndef PNG_HH
#define PNG_HH

#include <cstdint>
#include <pngconf.h>
#include <string>

using namespace std;

namespace pixel_terrain::commands::generate {
    class png {
        int width;
        int height;
        string filename;
        png_bytep data;

    public:
        png(int width, int height);
        png(string filename);
        ~png();

        int get_width();
        int get_height();
        void set_pixel(int x, int y, uint_fast32_t color);
        uint_fast32_t get_pixel(int x, int y);
        void clear(int x, int y);
        bool save(string filename);
        bool save();
    };
} // namespace pixel_terrain::commands::generate

#endif

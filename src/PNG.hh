#ifndef PNG_HH
#define PNG_HH

#include <cstdint>
#include <pngconf.h>
#include <string>

using namespace std;

class Png {
    int width;
    int height;
    string filename;
    png_bytep data;

public:
    Png (int width, int height);
    Png (string filename);
    ~Png ();

    int get_width ();
    int get_height ();
    unsigned char blend (int x, int y, uint32_t color);
    void increase_brightness (int x, int y, int num);
    void clear (int x, int y);
    bool save (string filename);
    bool save ();
};

#endif

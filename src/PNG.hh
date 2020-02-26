#ifndef PNG_HH
#define PNG_HH

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


    int get_width();
    int get_height();
    unsigned char blend (int x, int y, unsigned char r, unsigned char g,
                         unsigned char b, unsigned char a);
    void clear (int x, int y);
    bool save(string filename);
    bool save();
};

#endif

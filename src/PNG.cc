#include <algorithm>
#include <cerrno>
#include <cmath>
#include <csetjmp>
#include <cstring>
#include <stdexcept>

#include <png.h>
#include <pngconf.h>

#include "PNG.hh"

Png::Png (int width, int height)
    : width (width), height (height), data (new png_byte[width * height * 4]) {
    fill (data, data + width * height * 4, 0);
}

Png::Png (string filename) {
    FILE *in = fopen (filename.c_str (), "rb");
    if (in == nullptr) {
        throw runtime_error (strerror (errno));
    }

    unsigned char sig[8];
    fread (sig, 1, 8, in);
    if (!png_check_sig (sig, 8)) {
        throw runtime_error ("corrupted png file");
    }

    png_structp png = png_create_read_struct (PNG_LIBPNG_VER_STRING, nullptr,
                                              nullptr, nullptr);
    png_infop png_info = png_create_info_struct (png);

    if (setjmp (png_jmpbuf (png))) {
        png_destroy_read_struct (&png, &png_info, nullptr);

        throw runtime_error ("error reading png");
    }
    png_init_io (png, in);
    png_set_sig_bytes (png, 8);

    png_read_info (png, png_info);

    png_uint_32 width, height;
    int bit_depth, color_type;
    png_get_IHDR (png, png_info, &width, &height, &bit_depth, &color_type,
                  nullptr, nullptr, nullptr);

    if (bit_depth != 8 || color_type != PNG_COLOR_TYPE_RGBA) {
        png_destroy_read_struct (&png, &png_info, nullptr);

        throw runtime_error ("unsupported format");
    }

    data = new png_byte[width * height * 4];

    png_bytepp rows = new png_bytep[height];
    for (int i = 0; i < (int)height; ++i) {
        rows[i] = data + width * i * 4;
    }
    png_read_image (png, rows);
    delete[] rows;

    this->width = static_cast<int> (width);
    this->height = static_cast<int> (height);

    png_destroy_read_struct (&png, &png_info, nullptr);
}

Png::~Png () { delete[] data; }

unsigned char Png::blend (int x, int y, unsigned char r, unsigned char g,
                          unsigned char b, unsigned char a) {
    int base_off = (y * width + x) * 4;

    if (a == 0) {
        return data[base_off + 3];
    }

    unsigned char old_r = data[base_off];
    unsigned char old_g = data[base_off + 1];
    unsigned char old_b = data[base_off + 2];
    unsigned char old_a = data[base_off + 3];

    unsigned char new_a = (a + old_a) - a * old_a / 255;

    data[base_off] = (old_r * old_a + r * (255 - old_a) * a / 255) / new_a;
    data[base_off + 1] = (old_g * old_a + g * (255 - old_a) * a / 255) / new_a;
    data[base_off + 2] = (old_b * old_a + b * (255 - old_a) * a / 255) / new_a;
    data[base_off + 3] = new_a;

    return new_a;
}

void Png::increase_brightness (int x, int y, int num) {
    int base_off = (y * width + x) * 4;
    if (num > 0) {
        for (int i = 0; i < 3; ++i) {
            data[base_off + i] = min(data[base_off + i] + num, 255);
        }
    } else {
        for (int i = 0; i < 3; ++i) {
            data[base_off + i] = max(data[base_off + i] + num, 0);
        }
    }
}

int Png::get_width () { return width; }

int Png::get_height () { return height; }

void Png::clear (int x, int y) {
    int base_off = (width * y + x) * 4;

    data[base_off] = 0;
    data[++base_off] = 0;
    data[++base_off] = 0;
    data[++base_off] = 0;
}

bool Png::save (string filename) {
    FILE *f = fopen (filename.c_str (), "wb");
    if (f == nullptr) {
        return false;
    }

    png_structp png = png_create_write_struct (PNG_LIBPNG_VER_STRING, nullptr,
                                               nullptr, nullptr);
    png_infop info = png_create_info_struct (png);

    if (setjmp (png_jmpbuf (png))) {
        png_destroy_write_struct (&png, &info);

        return false;
    }
    png_init_io (png, f);

    if (setjmp (png_jmpbuf (png))) {
        png_destroy_write_struct (&png, &info);

        return false;
    }
    png_set_IHDR (png, info, width, height, 8, PNG_COLOR_TYPE_RGBA,
                  PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
                  PNG_FILTER_TYPE_DEFAULT);

    if (setjmp (png_jmpbuf (png))) {
        png_destroy_write_struct (&png, &info);

        return false;
    }
    png_write_info (png, info);

    if (setjmp (png_jmpbuf (png))) {
        png_destroy_write_struct (&png, &info);

        return false;
    }

    png_bytepp rows = new png_bytep[height];

    for (int i = 0; i < height; ++i) {
        rows[i] = data + i * width * 4;
    }

    png_write_rows (png, rows, 256);

    delete[] rows;

    if (setjmp (png_jmpbuf (png))) {
        png_destroy_write_struct (&png, &info);

        return false;
    }
    png_write_end (png, info);

    png_destroy_write_struct (&png, &info);

    fclose (f);

    return true;
}

bool Png::save () {
    if (filename.empty ()) throw logic_error ("filename is empty");

    return save (filename);
}

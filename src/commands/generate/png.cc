/* libpng wrapper.
   This file make easy to handle pixel buffer and PNG file. */

#include <algorithm>
#include <cerrno>
#include <cmath>
#include <csetjmp>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <stdexcept>

#include <png.h>
#include <pngconf.h>

#include "png.hh"

namespace pixel_terrain::commands::generate {
    png::png(int width, int height)
        : width(width), height(height), data(new png_byte[width * height * 4]) {
        fill(data, data + width * height * 4, 0);
    }

    png::png(string filename) {
        FILE *in = fopen(filename.c_str(), "rb");
        if (in == nullptr) {
            throw runtime_error(strerror(errno));
        }

        unsigned char sig[8];
        fread(sig, 1, 8, in);
        if (!png_check_sig(sig, 8)) {
            throw runtime_error("corrupted png file"s);
        }

        png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr,
                                                 nullptr, nullptr);
        png_infop png_info = png_create_info_struct(png);

        if (setjmp(png_jmpbuf(png))) {
            png_destroy_read_struct(&png, &png_info, nullptr);

            throw runtime_error("error reading png"s);
        }
        png_init_io(png, in);
        png_set_sig_bytes(png, 8);

        png_read_info(png, png_info);

        png_uint_32 width, height;
        int bit_depth, color_type;
        png_get_IHDR(png, png_info, &width, &height, &bit_depth, &color_type,
                     nullptr, nullptr, nullptr);

        if (bit_depth != 8 || color_type != PNG_COLOR_TYPE_RGBA) {
            png_destroy_read_struct(&png, &png_info, nullptr);

            throw runtime_error("unsupported format"s);
        }

        data = new png_byte[width * height * 4];

        png_bytepp rows = new png_bytep[height];
        for (int i = 0; i < (int)height; ++i) {
            rows[i] = data + width * i * 4;
        }
        png_read_image(png, rows);
        delete[] rows;

        this->width = static_cast<int>(width);
        this->height = static_cast<int>(height);

        png_destroy_read_struct(&png, &png_info, nullptr);

        fclose(in);
    }

    png::~png() { delete[] data; }

    void png::set_pixel(int x, int y, uint_fast32_t color) {
        int base_off = (y * width + x) * 4;
        data[base_off] = (color >> 24) & 0xff;
        data[++base_off] = (color >> 16) & 0xff;
        data[++base_off] = (color >> 8) & 0xff;
        data[++base_off] = color & 0xff;
    }

    uint_fast32_t png::get_pixel(int x, int y) {
        int base_off = (y * width + x) * 4;
        uint_fast8_t r = data[base_off];
        uint_fast8_t g = data[++base_off];
        uint_fast8_t b = data[++base_off];
        uint_fast8_t a = data[++base_off];

        return ((r & 0xff) << 24) | ((g & 0xff) << 16) | ((b & 0xff) << 8) |
               (a & 0xff);
    }

    int png::get_width() { return width; }

    int png::get_height() { return height; }

    void png::clear(int x, int y) {
        int base_off = (width * y + x) * 4;

        data[base_off] = 0;
        data[++base_off] = 0;
        data[++base_off] = 0;
        data[++base_off] = 0;
    }

    bool png::save(string filename) {
        FILE *f = fopen(filename.c_str(), "wb");
        if (f == nullptr) {
            return false;
        }

        png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING,
                                                  nullptr, nullptr, nullptr);
        png_infop info = png_create_info_struct(png);

        if (setjmp(png_jmpbuf(png))) {
            png_destroy_write_struct(&png, &info);

            return false;
        }
        png_init_io(png, f);

        if (setjmp(png_jmpbuf(png))) {
            png_destroy_write_struct(&png, &info);

            return false;
        }
        png_set_IHDR(png, info, width, height, 8, PNG_COLOR_TYPE_RGBA,
                     PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
                     PNG_FILTER_TYPE_DEFAULT);

        if (setjmp(png_jmpbuf(png))) {
            png_destroy_write_struct(&png, &info);

            return false;
        }
        png_write_info(png, info);

        if (setjmp(png_jmpbuf(png))) {
            png_destroy_write_struct(&png, &info);

            return false;
        }

        png_bytepp rows = new png_bytep[height];

        for (int i = 0; i < height; ++i) {
            rows[i] = data + i * width * 4;
        }

        png_write_rows(png, rows, 256);

        delete[] rows;

        if (setjmp(png_jmpbuf(png))) {
            png_destroy_write_struct(&png, &info);

            return false;
        }
        png_write_end(png, info);

        png_destroy_write_struct(&png, &info);

        fclose(f);

        return true;
    }

    bool png::save() {
        if (filename.empty()) throw logic_error("filename is empty"s);

        return save(filename);
    }
} // namespace pixel_terrain::commands::generate

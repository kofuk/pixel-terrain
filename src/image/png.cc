/*
 * Copyright (c) 2020 Koki Fukuda
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* libpng wrapper.
   This file make easy to handle pixel buffer and PNG file. */

#include <algorithm>
#include <cerrno>
#include <cmath>
#include <csetjmp>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <stdexcept>

#include <png.h>
#include <pngconf.h>

#include "../utils/path_hack.hh"
#include "png.hh"

namespace pixel_terrain::image {
    png::png(int width, int height)
        : width(width), height(height), data(new png_byte[width * height * 4]) {
        std::fill(data, data + width * height * 4, 0);
    }

    png::png(std::filesystem::path path) {
        std::FILE *in = FOPEN(path.c_str(), "rb");
        if (in == nullptr) {
            throw std::runtime_error(strerror(errno));
        }

        unsigned char sig[8];
        std::fread(sig, 1, 8, in);
        if (!png_check_sig(sig, 8)) {
            throw std::runtime_error("corrupted png file");
        }

        png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr,
                                                 nullptr, nullptr);
        png_infop png_info = png_create_info_struct(png);

        if (setjmp(png_jmpbuf(png))) {
            png_destroy_read_struct(&png, &png_info, nullptr);

            throw std::runtime_error("error reading png");
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

            throw std::runtime_error("unsupported format");
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

        std::fclose(in);
    }

    png::~png() { delete[] data; }

    void png::fit(unsigned int width, unsigned int height) {
        ::png_bytep new_data = new ::png_byte[width * height * 4];
        std::fill(new_data, new_data + width * height * 4, 0);
        for (int x = 0, x_end = std::min(this->width, width); x < x_end; ++x) {
            for (int y = 0, y_end = std::min(this->width, width); y < y_end; ++y) {
                unsigned int from_base = (y * this->width + x) * 4;
                unsigned int to_base = (y * width + x) * 4;
                for (int i = 0; i < 4; ++i) {
                    new_data[to_base + i] = this->data[from_base + i];
                }
            }
        }
        this->width = width;
        this->height = height;
        delete[] data;
        data = new_data;
    }

    void png::set_pixel(int x, int y, std::uint_fast32_t color) {
        int base_off = (y * width + x) * 4;
        data[base_off] = (color >> 24) & 0xff;
        data[++base_off] = (color >> 16) & 0xff;
        data[++base_off] = (color >> 8) & 0xff;
        data[++base_off] = color & 0xff;
    }

    std::uint_fast32_t png::get_pixel(int x, int y) {
        int base_off = (y * width + x) * 4;
        std::uint_fast8_t r = data[base_off];
        std::uint_fast8_t g = data[++base_off];
        std::uint_fast8_t b = data[++base_off];
        std::uint_fast8_t a = data[++base_off];

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

    bool png::save(std::filesystem::path path) {
        std::FILE *f = FOPEN(path.c_str(), "wb");
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

        png_write_rows(png, rows, width);

        delete[] rows;

        if (setjmp(png_jmpbuf(png))) {
            png_destroy_write_struct(&png, &info);

            return false;
        }
        png_write_end(png, info);

        png_destroy_write_struct(&png, &info);

        std::fclose(f);

        return true;
    }

    bool png::save() {
        if (path.empty()) throw std::logic_error("filename is empty");

        return save(path);
    }
} // namespace pixel_terrain::image

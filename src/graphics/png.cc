// SPDX-License-Identifier: MIT

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

#include "graphics/constants.hh"
#include "graphics/png.hh"
#include "utils/path_hack.hh"

namespace pixel_terrain::graphics {
    namespace {
        inline constexpr std::size_t PNG_SIG_LEN = 8;
        inline constexpr int SUPPORTED_BIT_DEPTH = 8;
        inline constexpr unsigned int N_CHANNEL = 4;
    } // namespace

    png::png(int width, int height)
        : width(width), height(height), data(new png_byte[width * height * 4]) {
        std::fill(data, data + width * height * 4, 0);
    }

    png::png(std::filesystem::path const &path) {
        std::FILE *in = FOPEN(path.c_str(), "rb");
        if (in == nullptr) {
            throw std::runtime_error(strerror(errno));
        }

        std::array<std::uint8_t, PNG_SIG_LEN> sig;
        std::fread(sig.data(), 1, PNG_SIG_LEN, in);
        if (!png_check_sig(sig.data(), PNG_SIG_LEN)) {
            throw std::runtime_error("corrupted png file");
        }

        ::png_structp png = ::png_create_read_struct(PNG_LIBPNG_VER_STRING,
                                                     nullptr, nullptr, nullptr);
        ::png_infop png_info = ::png_create_info_struct(png);

        if (setjmp(png_jmpbuf(png))) {
            ::png_destroy_read_struct(&png, &png_info, nullptr);

            throw std::runtime_error("error reading png");
        }
        ::png_init_io(png, in);
        ::png_set_sig_bytes(png, PNG_SIG_LEN);

        ::png_read_info(png, png_info);

        ::png_uint_32 width;
        ::png_uint_32 height;
        int bit_depth;
        int color_type;
        ::png_get_IHDR(png, png_info, &width, &height, &bit_depth, &color_type,
                       nullptr, nullptr, nullptr);

        if (bit_depth != SUPPORTED_BIT_DEPTH ||
            color_type != PNG_COLOR_TYPE_RGBA) {
            ::png_destroy_read_struct(&png, &png_info, nullptr);

            throw std::runtime_error("unsupported format");
        }

        data = new ::png_byte[width * height * 4];

        auto *rows = new ::png_bytep[height];
        for (int i = 0; i < (int)height; ++i) {
            rows[i] = data + width * i * 4;
        }
        ::png_read_image(png, rows);
        delete[] rows;

        this->width = static_cast<int>(width);
        this->height = static_cast<int>(height);

        ::png_destroy_read_struct(&png, &png_info, nullptr);

        std::fclose(in);
    }

    png::~png() { delete[] data; }

    void png::fit(unsigned int width, unsigned int height) {
        auto *new_data = new ::png_byte[width * height * 4];
        std::fill(new_data, new_data + width * height * 4, 0);
        for (int x = 0, x_end = std::min(this->width, width); x < x_end; ++x) {
            for (int y = 0, y_end = std::min(this->width, width); y < y_end;
                 ++y) {
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

    void png::set_pixel(unsigned int x, unsigned int y,
                        std::uint_fast32_t color) {
        using namespace color;

        unsigned int base_off = (y * width + x) * 4;
        data[base_off] = (color >> R_OFFSET) & CHAN_MASK;
        data[++base_off] = (color >> G_OFFSET) & CHAN_MASK;
        data[++base_off] = (color >> B_OFFSET) & CHAN_MASK;
        data[++base_off] = (color >> A_OFFSET) & CHAN_MASK;
    }

    auto png::get_pixel(int x, int y) -> std::uint_fast32_t {
        using namespace color;

        unsigned int base_off = (y * width + x) * 4;
        std::uint_fast8_t r = data[base_off];
        std::uint_fast8_t g = data[++base_off];
        std::uint_fast8_t b = data[++base_off];
        std::uint_fast8_t a = data[++base_off];

        return ((r & CHAN_MASK) << R_OFFSET) | ((g & CHAN_MASK) << G_OFFSET) |
               ((b & CHAN_MASK) << B_OFFSET) | ((a & CHAN_MASK) << A_OFFSET);
    }

    auto png::get_width() const -> unsigned int { return width; }

    auto png::get_height() const -> unsigned int { return height; }

    void png::clear(int x, int y) {
        unsigned int base_off = (width * y + x) * N_CHANNEL;

        data[base_off] = 0;
        data[++base_off] = 0;
        data[++base_off] = 0;
        data[++base_off] = 0;
    }

    auto png::save(std::filesystem::path const &path) -> bool {
        std::FILE *f = FOPEN(path.c_str(), "wb");
        if (f == nullptr) {
            return false;
        }

        ::png_structp png = ::png_create_write_struct(
            PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
        ::png_infop info = ::png_create_info_struct(png);

        if (setjmp(png_jmpbuf(png))) {
            ::png_destroy_write_struct(&png, &info);

            return false;
        }
        ::png_init_io(png, f);

        if (setjmp(png_jmpbuf(png))) {
            ::png_destroy_write_struct(&png, &info);

            return false;
        }
        ::png_set_IHDR(png, info, width, height, SUPPORTED_BIT_DEPTH,
                       PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE,
                       PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

        if (setjmp(png_jmpbuf(png))) {
            ::png_destroy_write_struct(&png, &info);

            return false;
        }
        ::png_write_info(png, info);

        if (setjmp(png_jmpbuf(png))) {
            ::png_destroy_write_struct(&png, &info);

            return false;
        }

        auto *rows = new png_bytep[height];

        for (unsigned int i = 0; i < height; ++i) {
            rows[i] = data + i * width * 4;
        }

        ::png_write_rows(png, rows, width);

        delete[] rows;

        if (setjmp(png_jmpbuf(png))) {
            ::png_destroy_write_struct(&png, &info);

            return false;
        }
        ::png_write_end(png, info);

        ::png_destroy_write_struct(&png, &info);

        std::fclose(f);

        return true;
    }

    auto png::save() -> bool {
        if (path.empty()) {
            throw std::logic_error("filename is empty");
        }

        return save(path);
    }
} // namespace pixel_terrain::graphics

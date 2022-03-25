// SPDX-License-Identifier: MIT

/* libpng wrapper.
   This file make easy to handle pixel buffer and PNG file. */

#include <algorithm>
#include <array>
#include <cerrno>
#include <cmath>
#include <csetjmp>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <vector>

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

        ::png_image png;
        std::memset(&png, 0, sizeof(::png_image));
        png.version = PNG_IMAGE_VERSION;

#ifdef OS_WIN
        /* XXX    I don't know why, but reader function called inside
           libpng cause this application to crash. */
        std::vector<std::uint8_t> png_data;
        std::array<std::uint8_t, 4096> buf;
        std::size_t nread;
        do {
            nread = std::fread(buf.data(), 1, buf.size(), in);
            png_data.insert(png_data.end(), buf.begin(), buf.begin() + nread);
        } while (nread > 0);
        if (!static_cast<bool>(std::feof(in))) {
            throw std::runtime_error("Failed to read data");
        }

        ::png_image_begin_read_from_memory(&png, png_data.data(),
                                           png_data.size());
        ;
#else  // not defined(OS_WIN)
        ::png_image_begin_read_from_stdio(&png, in);
#endif // defined(OS_WIN)

        if (PNG_IMAGE_FAILED(png)) {
            std::fclose(in);
            std::runtime_error err(png.message);
            ::png_image_free(&png);
            throw err;
        }

        ::png_uint_32 stride = PNG_IMAGE_ROW_STRIDE(png);
        data = new ::png_byte[png.width * png.height * 4];

        width = png.width;
        height = png.height;

        if (png.format == PNG_FORMAT_RGBA) {
            ::png_image_finish_read(&png, nullptr, data, stride, nullptr);
        } else {
            std::memset(data, 0, png.width * png.height * 4);
        }

        ::png_image_free(&png);
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

        ::png_image png;
        std::memset(&png, 0, sizeof(::png_image));
        png.version = PNG_IMAGE_VERSION;
        png.width = width;
        png.height = height;
        png.format = PNG_FORMAT_RGBA;
        ::png_uint_32 stride = PNG_IMAGE_ROW_STRIDE(png);

#if defined(OS_WIN)
        /* XXX   Same as its reader function.
           We write the data to memory once, then write to file. */
        int result;
        /* First we try 512 KiB because it's large enough in
           almost all cases. */
        ::png_alloc_size_t mem_size = 1024 * 512;
        std::uint8_t *out = nullptr;
        do {
            delete[] out;
            out = new std::uint8_t[mem_size];
            /* png_image_write_to_memory writes required memory size to
               `mem_size` even if the invocation fails. We allocate sifficient
               memory later, in case the function fails. */
            result = ::png_image_write_to_memory(&png, out, &mem_size, 0, data,
                                                 stride, nullptr);
        } while (!static_cast<bool>(result));
        std::size_t n = std::fwrite(out, 1, mem_size, f);
        if (n != mem_size) {
            delete[] out;
            return false;
        }
        delete[] out;
#else  // not defined(OS_WIN)
        if (::png_image_write_to_stdio(&png, f, 0, data, stride, nullptr) ==
            0) {
            ::png_image_free(&png);
            std::fclose(f);
            return false;
        }
#endif // defined(OS_WIN)

        ::png_image_free(&png);
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

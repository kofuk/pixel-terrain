#include <array>
#include <csetjmp>
#include <cstdio>
#include <filesystem>
#include <iostream>
#include <string>

#include <png.h>
#include <pngconf.h>

#include "blocks.hh"
#include "Region.hh"

using namespace std;

static inline void put_pixel(png_bytepp image, int x, int y, unsigned char r,
                             unsigned char g, unsigned char b) {
    int base_off = x * 3;
    image[y][base_off] = r;
    image[y][base_off + 1] = g;
    image[y][base_off + 2] = b;
}

static void generate_256(Anvil::Region *region, int region_x, int region_z,
                         int off_x, int off_z) {
    FILE *f = fopen((to_string(region_x * 2 + off_x) + ',' +
                     to_string(region_z * 2 + off_z) + ".png")
                        .c_str(),
                    "w");
    if (f == nullptr) {
        return;
    }

    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr,
                                              nullptr, nullptr);
    png_infop info = png_create_info_struct(png);

    if (setjmp(png_jmpbuf(png))) {
        png_destroy_write_struct(&png, &info);

        return;
    }
    png_init_io(png, f);

    if (setjmp(png_jmpbuf(png))) {
        png_destroy_write_struct(&png, &info);

        return;
    }
    png_set_IHDR(png, info, 256, 256, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

    if (setjmp(png_jmpbuf(png))) {
        png_destroy_write_struct(&png, &info);

        return;
    }
    png_write_info(png, info);

    png_bytepp rows = new png_bytep[256];
    /* allocate memory for each row, 3 bytes (rgb) per a pixel. */
    for (int i = 0; i < 256; ++i)
        rows[i] = new png_byte[256 * 3];

    for (int chunk_z = 0; chunk_z < 16; ++chunk_z) {
        for (int chunk_x = 0; chunk_x < 16; ++chunk_x) {
            Anvil::Chunk *chunk =
                region->get_chunk(off_x * 16 + chunk_x, off_z * 16 + chunk_z);

            if (chunk == nullptr) {
                for (int x = 0; x < 16; ++x) {
                    for (int z = 0; z < 16; ++z) {
                        put_pixel(rows, chunk_x * 16 + x, chunk_z * 16 + z, 0,
                                  0, 0);
                    }
                }

                continue;
            }

            for (int z = 0; z < 16; ++z) {
                int prev_y = -1;
                for (int x = 0; x < 16; ++x) {
                    for (int y = 255; y >= 0; --y) {
                        string block = chunk->get_block(x, y, z);

                        if (block == "air" || block == "cave_air" ||
                            block == "void_air") {
                            if (y == 0) {
                                put_pixel(rows, chunk_x * 16 + x,
                                          chunk_z * 16 + z, 0, 0, 0);
                            }
                            continue;
                        }

                        auto color_itr = colors.find(block);
                        if (color_itr == std::end(colors)) {
                            cout << R"(colors[")" << block << R"("] = ???)"
                                 << endl;

                            put_pixel(rows, chunk_x * 16 + x, chunk_z * 16 + z,
                                      0, 0, 0);
                        } else {
                            array<unsigned char, 3> color = color_itr->second;
                            array<int, 3> rcolor = {color[0], color[1],
                                                    color[2]};

                            int plus;
                            if (prev_y < 0 || prev_y == y) {
                                plus = 0;
                            } else if (prev_y < y) {
                                plus = 30;
                            } else {
                                plus = -30;
                            }

                            for (int i = 0; i < 3; ++i) {
                                rcolor[i] += plus;
                                if (rcolor[i] > 255)
                                    rcolor[i] = 255;
                                else if (rcolor[i] < 0)
                                    rcolor[i] = 0;
                            }

                            put_pixel(rows, chunk_x * 16 + x, chunk_z * 16 + z,
                                      rcolor[0], rcolor[1], rcolor[2]);
                        }

                        prev_y = y;

                        break;
                    }
                }
            }

            delete chunk;
        }
    }

    if (setjmp(png_jmpbuf(png))) {
        png_destroy_write_struct(&png, &info);

        return;
    }
    png_write_rows(png, rows, 256);

    for (int i = 0; i < 256; ++i)
        delete[] rows[i];
    delete[] rows;

    if (setjmp(png_jmpbuf(png))) {
        png_destroy_write_struct(&png, &info);

        return;
    }
    png_write_end(png, info);

    png_destroy_write_struct(&png, &info);

    fclose(f);
}

static void generate_all(string src_dir, int x, int z) {
    filesystem::path path(src_dir);
    path /= "r." + to_string(x) + "." + to_string(z) + ".mca";

    Anvil::Region r(path);

    for (int off_x = 0; off_x < 2; ++off_x) {
        for (int off_z = 0; off_z < 2; ++off_z) {
            generate_256(&r, x, z, off_x, off_z);
        }
    }
}

static void print_usage() {
    cout << "usage: gen_map src_dir region_x region_z" << endl;
}

int main(int argc, char **argv) {
    if (argc < 4) {
        print_usage();

        return 0;
    }

    init_block_list();

    generate_all(argv[1], stoi(argv[2]), stoi(argv[3]));

    return 0;
}

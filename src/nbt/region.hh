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

#ifndef REGION_HH
#define REGION_HH

#include <memory>
#include <string>
#include <utility>

#include "../utils/path_hack.hh"
#include "chunk.hh"
#include "file.hh"

namespace pixel_terrain::anvil {
    class region {
        std::unique_ptr<file<unsigned char>> data;
        std::size_t len;
        std::unique_ptr<file<std::uint64_t>> last_update;

        std::size_t header_offset(int chunk_x, int chunk_z);
        std::size_t chunk_location_off(int chunk_x, int chunk_z);
        std::size_t chunk_location_sectors(int chunk_x, int chunk_z);

    public:
        /* Construct new region object from given buffer of *.mca file content
         */
        region(std::filesystem::path filename);
        region(std::filesystem::path filename,
               std::filesystem::path journal_dir);
        std::pair<std::shared_ptr<unsigned char[]>, std::size_t>
        chunk_data(int chunk_x, int chunk_z);
        chunk *get_chunk(int chunk_x, int chunk_z);
        chunk *get_chunk_if_dirty(int chunk_x, int chunk_z);
        bool exists_chunk_data(int chunk_x, int chunk_z);
    };
} // namespace pixel_terrain::anvil

#endif

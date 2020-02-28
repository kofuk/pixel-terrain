#include <algorithm>
#include <stdexcept>
#include <vector>

#include <zlib.h>

#include "utils.hh"

using namespace std;

namespace NBT {
    namespace Utils {
        DecompressedData::DecompressedData () : data (nullptr) {}

        DecompressedData::~DecompressedData () {
            if (data != nullptr) {
                delete[] data;
            }
        }

        DecompressedData *zlib_decompress (unsigned char *data,
                                           size_t const len) {
            int z_ret;
            z_stream strm;
            strm.zalloc = Z_NULL;
            strm.zfree = Z_NULL;
            strm.opaque = Z_NULL;
            strm.avail_in = 0;
            strm.next_in = Z_NULL;
            unsigned char out[1024];

            z_ret = inflateInit (&strm);
            if (z_ret != Z_OK) return nullptr;

            vector<unsigned char> all_out;
            all_out.reserve (len * 2);

            strm.avail_in = len;
            strm.next_in = data;

            do {
                strm.avail_out = 1024;
                strm.next_out = out;

                z_ret = inflate (&strm, Z_NO_FLUSH);

                if (z_ret == Z_STREAM_ERROR || z_ret == Z_NEED_DICT ||
                    z_ret == Z_DATA_ERROR || z_ret == Z_MEM_ERROR) {
                    inflateEnd (&strm);

                    return nullptr;
                }

                all_out.insert (end (all_out), out,
                                out + 1024 - strm.avail_out);
            } while (strm.avail_out == 0);

            if (z_ret != Z_STREAM_END) {
                throw out_of_range (
                    "buffer exausted before stream end of zlib");
            }

            inflateEnd (&strm);

            DecompressedData *dd = new DecompressedData;
            unsigned char *dd_out = new unsigned char[all_out.size ()];
            copy (begin (all_out), end (all_out), dd_out);
            dd->data = dd_out;
            dd->len = all_out.size ();

            return dd;
        }
    } // namespace Utils
} // namespace NBT

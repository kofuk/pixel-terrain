#include <condition_variable>
#include <filesystem>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include <png.h>
#include <pngconf.h>

#include "Region.hh"
#include "blocks.hh"
#include "worker.hh"

using namespace std;

RegionContainer::RegionContainer(Anvil::Region *region, int rx, int rz)
    : region(region), rx(rx), rz(rz) {}

RegionContainer::~RegionContainer() {
    delete region;
}

void RegionContainer::set_ref_count(int ref_count) {
    unique_lock<mutex> lock(ref_count_mutex);
    this->ref_count = ref_count;
}

bool RegionContainer::decrease_ref() {
    {
        unique_lock<mutex> lock(ref_count_mutex);
        --ref_count;

        if (ref_count > 0) return false;
    }

    if (option_verbose) {
        cout << "discarding " + to_string(rx) + ", " + to_string(rz) << endl;
    }

    delete this;

    return true;
}

QueuedItem::QueuedItem(RegionContainer *region, int off_x, int off_z)
    : region(region), off_x(off_x), off_z(off_z) {}

string QueuedItem::debug_string() {
    if (region == nullptr) {
        return "(finishing job)";
    }

    return "(" + to_string(region->rx) + "+" + to_string(off_x) + ", " +
           to_string(region->rz) + "+" + to_string(off_z) + ")";
}

static queue<QueuedItem *> offs_queue;

static mutex queue_mutex;
static vector<thread *> threads;

static condition_variable queue_cap_cond;
static mutex queue_cap_cond_mutex;

static condition_variable queued_cond;
static mutex queued_cond_mutex;

string option_out_dir;
bool option_verbose;
int option_jobs;
bool option_nether;
bool option_generate_progress;

static inline void put_pixel(png_bytepp image, int x, int y, unsigned char r,
                             unsigned char g, unsigned char b) {
    int base_off = x * 3;
    image[y][base_off] = r;
    image[y][base_off + 1] = g;
    image[y][base_off + 2] = b;
}

static void generate_256(QueuedItem *item) {
    Anvil::Region *region = item->region->region;
    int region_x = item->region->rx;
    int region_z = item->region->rz;
    int off_x = item->off_x;
    int off_z = item->off_z;

    if (option_verbose) {
        cerr << "generating " + item->debug_string() + " ..." << endl;
    }

    filesystem::path path = option_out_dir;
    path /= (to_string(region_x * 2 + off_x) + ',' +
             to_string(region_z * 2 + off_z) + ".png");

    FILE *f = fopen(path.string().c_str(), "wb");
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
                    bool air_found = false;

                    int max_y = 255;
                    if (option_nether) max_y = 127;

                    for (int y = max_y; y >= 0; --y) {
                        string block = chunk->get_block(x, y, z);

                        if (block == "air" || block == "cave_air" ||
                            block == "void_air") {
                            air_found = true;
                            if (y == 0) {
                                put_pixel(rows, chunk_x * 16 + x,
                                          chunk_z * 16 + z, 0, 0, 0);
                            }
                            continue;
                        }

                        if (option_nether && !air_found) {
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

    if (option_verbose) {
        cerr << "generated " + item->debug_string() + "." << endl;
    }
}

static QueuedItem *fetch_item() {
    QueuedItem *result;

    unique_lock<mutex> lock(queue_mutex);

    if (offs_queue.empty()) return nullptr;

    result = offs_queue.front();
    offs_queue.pop();

    queue_cap_cond.notify_one();

    return result;
}

static void run_worker_loop() {
    for (;;) {
        QueuedItem *item = fetch_item();
        if (item == nullptr) {
            {
                unique_lock<mutex> lock(queued_cond_mutex);
                queued_cond.wait(lock);
            }

            continue;
        }

        if (item->region == nullptr) {
            if (option_verbose) {
                cout << "shutting down worker" << endl;
            }

            delete item;

            break;
        }

        generate_256(item);

        item->region->decrease_ref();
        delete item;
    }
}

void queue_item(QueuedItem *item) {
    if (option_verbose) {
        cout << "trying to queue " + item->debug_string() << endl;
    }

    unique_lock<mutex> lock(queue_cap_cond_mutex);

    for (;;) {
        {
            unique_lock<mutex> queue_lock(queue_mutex);

            if (offs_queue.size() < (size_t)option_jobs * 2) {
                offs_queue.push(item);

                queued_cond.notify_all();

                if (option_verbose) {
                    cout << "item " + item->debug_string() +
                                " successfully queued."
                         << endl;
                }
                return;
            }
        }

        queue_cap_cond.wait(lock);
    }
}

void start_worker() {
    if (option_verbose) {
        cout << "starting worker thread(s) ...";
        if (option_nether) {
            cout << "(nether mode)";
        }
        cout << endl;
    }

    for (int i = 0; i < option_jobs; ++i) {
        threads.push_back(new thread(&run_worker_loop));
    }
}

void wait_for_worker() {
    for (auto itr = begin(threads); itr != end(threads); ++itr) {
        (*itr)->join();
        delete *itr;
    }
}

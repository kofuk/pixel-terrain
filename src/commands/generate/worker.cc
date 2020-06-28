/* Helper to use multi-thread efficiently. */

#include <algorithm>
#include <condition_variable>
#include <cstdint>
#include <exception>
#include <filesystem>
#include <iterator>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "../../logger/logger.hh"
#include "../../nbt/region.hh"
#include "../threaded_worker.hh"
#include "generator.hh"
#include "worker.hh"

namespace pixel_terrain::commands::generate {
    region_container::region_container(anvil::region *region, int rx, int rz)
        : region(region), rx(rx), rz(rz) {}

    region_container::~region_container() { delete region; }

    queued_item::queued_item(std::shared_ptr<region_container> region,
                             int off_x, int off_z)
        : region(move(region)), off_x(off_x), off_z(off_z) {}

    std::string queued_item::debug_string() {
        if (region == nullptr) {
            return "(finishing job)";
        }

        return "(" + std::to_string(region->rx) + "+" + std::to_string(off_x) +
               ", " + std::to_string(region->rz) + "+" + std::to_string(off_z) +
               ")";
    }

    namespace {
        threaded_worker<std::shared_ptr<queued_item>> *worker;
    } // namespace

    std::string option_out_dir;
    bool option_verbose;
    int option_jobs;
    bool option_nether;
    bool option_generate_progress;
    bool option_generate_range;
    std::string option_journal_dir;

    void queue_item(std::shared_ptr<queued_item> item) {
        if (option_verbose) {
            logger::d("trying to queue " + item->debug_string());
        }

        worker->queue_job(move(item));
    }

    void start_worker() {
        if (option_verbose) {
            logger::d("starting worker thread(s) ...");
        }

        worker = new threaded_worker<std::shared_ptr<queued_item>>(
            option_jobs, &generate_256);
        worker->start();
    }

    void wait_for_worker() {
        worker->wait();
        delete worker;
    }

    void finish_worker() { worker->finish(); }
} // namespace pixel_terrain::commands::generate

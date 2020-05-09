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
#include "../../nbt/Region.hh"
#include "PNG.hh"
#include "generator.hh"
#include "worker.hh"
#include "../threaded_worker.hh"

using namespace std;

RegionContainer::RegionContainer (anvil::Region *region, int rx, int rz)
    : region (region), rx (rx), rz (rz) {}

RegionContainer::~RegionContainer () { delete region; }

QueuedItem::QueuedItem (shared_ptr<RegionContainer> region, int off_x,
                        int off_z)
    : region (move (region)), off_x (off_x), off_z (off_z) {}

string QueuedItem::debug_string () {
    if (region == nullptr) {
        return "(finishing job)"s;
    }

    return "("s + to_string (region->rx) + "+"s + to_string (off_x) + ", "s +
           to_string (region->rz) + "+"s + to_string (off_z) + ")"s;
}

namespace {
    ThreadedWorker<shared_ptr<QueuedItem>> *worker;
} // namespace

string option_out_dir;
bool option_verbose;
int option_jobs;
bool option_nether;
bool option_generate_progress;
bool option_generate_range;
string option_journal_dir;

void queue_item (shared_ptr<QueuedItem> item) {
    if (option_verbose) {
        logger::d ("trying to queue " + item->debug_string ());
    }

    worker->queue_job(move (item));
}

void start_worker () {
    if (option_verbose) {
        logger::d ("starting worker thread(s) ...");
    }

    worker = new ThreadedWorker<shared_ptr<QueuedItem>> (option_jobs, &generator::generate_256);
    worker->start ();
}

void wait_for_worker () {
    worker->wait();
    delete worker;
}

void finish_worker () {
    worker->finish();
}

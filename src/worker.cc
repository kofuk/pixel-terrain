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

#include "PNG.hh"
#include "Region.hh"
#include "generator.hh"
#include "logger.hh"
#include "worker.hh"

using namespace std;

RegionContainer::RegionContainer (anvil::Region *region, int rx, int rz)
    : region (region), rx (rx), rz (rz) {}

RegionContainer::~RegionContainer () { delete region; }

QueuedItem::QueuedItem (shared_ptr<RegionContainer> region, int off_x,
                        int off_z)
    : region (move (region)), off_x (off_x), off_z (off_z) {}

string QueuedItem::debug_string () {
    if (region == nullptr) {
        return "(finishing job)";
    }

    return "(" + to_string (region->rx) + "+" + to_string (off_x) + ", " +
           to_string (region->rz) + "+" + to_string (off_z) + ")";
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
bool option_generate_range;
string option_journal_dir;

QueuedItem *fetch_item () {
    QueuedItem *result;

    unique_lock<mutex> lock (queue_mutex);

    if (offs_queue.empty ()) return nullptr;

    result = offs_queue.front ();
    offs_queue.pop ();

    queue_cap_cond.notify_one ();

    return result;
}

static void run_worker_loop () {
    for (;;) {
        QueuedItem *item = fetch_item ();
        if (item == nullptr) {
            {
                unique_lock<mutex> lock (queued_cond_mutex);
                queued_cond.wait (lock);
            }

            continue;
        }

        if (item->region == nullptr) {
            if (option_verbose) {
                logger::d ("shutting down worker");
            }

            delete item;

            break;
        }

        generator::generate_256 (item);
    }
}

void queue_item (QueuedItem *item) {
    if (option_verbose) {
        logger::d ("trying to queue " + item->debug_string ());
    }

    unique_lock<mutex> lock (queue_cap_cond_mutex);

    for (;;) {
        {
            unique_lock<mutex> queue_lock (queue_mutex);

            if (offs_queue.size () < static_cast<size_t> (option_jobs) * 2) {
                offs_queue.push (item);

                queued_cond.notify_all ();

                if (option_verbose) {
                    logger::d ("item " + item->debug_string () +
                               " successfully queued");
                }
                return;
            }
        }

        queue_cap_cond.wait (lock);
    }
}

void start_worker () {
    if (option_verbose) {
        logger::d ("starting worker thread(s) ...");
    }

    try {
        for (int i = 0; i < option_jobs; ++i) {
            threads.push_back (new thread (&run_worker_loop));
        }
    } catch (system_error const &e) {
        logger::e (string ("Cannot create thread: ") + e.what ());

        for (thread *t : threads) {
            t->join ();
        }

        exit (1);
    }
}

void wait_for_worker () {
    for (auto itr = begin (threads); itr != end (threads); ++itr) {
        (*itr)->join ();
        delete *itr;
    }
}

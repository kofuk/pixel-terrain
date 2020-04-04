#ifndef WORKER_HH
#define WORKER_HH

#include <mutex>
#include <utility>

#include "../../nbt/Region.hh"

using namespace std;

struct RegionContainer {
    anvil::Region *region;
    int rx;
    int rz;

    RegionContainer (anvil::Region *region, int rx, int rz);
    ~RegionContainer ();
};

struct QueuedItem {
    shared_ptr<RegionContainer> region;
    int off_x;
    int off_z;

    QueuedItem (shared_ptr<RegionContainer> region, int off_x, int off_z);

    string debug_string ();
};

extern string option_out_dir;
extern bool option_verbose;
extern int option_jobs;
extern bool option_nether;
extern bool option_generate_range;
extern string option_journal_dir;

QueuedItem *fetch_item ();
void queue_item (shared_ptr<QueuedItem> item);
void start_worker ();
void wait_for_worker ();
void finish_worker ();

#endif

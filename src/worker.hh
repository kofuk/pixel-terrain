#ifndef WORKER_HH
#define WORKER_HH

#include <mutex>
#include <utility>

#include "Region.hh"

using namespace std;

class RegionContainer {
    int ref_count;
    mutex ref_count_mutex;

public:
    Anvil::Region *region;
    int rx;
    int rz;

    RegionContainer(Anvil::Region *region, int rx, int rz);
    void set_ref_count(int ref_count);
    /* if decreased ref count is less than 1, object commits suicide and returns
     * true. */
    bool decrease_ref();
};

struct QueuedItem {
    RegionContainer *region;
    int off_x;
    int off_z;

    QueuedItem(RegionContainer *region, int off_x, int off_z);

    string debug_string();
};

extern string option_out_dir;
extern bool option_verbose;
extern int option_jobs;
extern bool option_nether;
extern bool option_generate_progress;

void queue_item(QueuedItem *item);
void start_worker();
void wait_for_worker();

#endif

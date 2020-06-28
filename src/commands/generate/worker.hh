#ifndef WORKER_HH
#define WORKER_HH

#include <mutex>
#include <utility>

#include "../../nbt/region.hh"

namespace pixel_terrain::commands::generate {
    struct region_container {
        anvil::region *region;
        int rx;
        int rz;

        region_container(anvil::region *region, int rx, int rz);
        ~region_container();
    };

    struct queued_item {
        std::shared_ptr<region_container> region;
        int off_x;
        int off_z;

        queued_item(std::shared_ptr<region_container> region, int off_x,
                    int off_z);

        std::string debug_string();
    };

    extern std::string option_out_dir;
    extern bool option_verbose;
    extern int option_jobs;
    extern bool option_nether;
    extern bool option_generate_range;
    extern std::string option_journal_dir;

    queued_item *fetch_item();
    void queue_item(std::shared_ptr<queued_item> item);
    void start_worker();
    void wait_for_worker();
    void finish_worker();
} // namespace pixel_terrain::commands::generate

#endif

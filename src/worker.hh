#ifndef WORKER_HH
#define WORKER_HH

#include <utility>

#include "Region.hh"

using namespace std;

extern string option_out_dir;
extern bool option_verbose;
extern int option_jobs;
extern bool option_nether;

void init_worker(Anvil::Region *r, int rx, int rz);
void queue_offset(pair<int, int> *off);
void start_worker();

#endif

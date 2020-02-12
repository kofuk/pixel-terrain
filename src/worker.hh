#ifndef WORKER_HH
#define WORKER_HH

#include <utility>

#include "Region.hh"

using namespace std;

void init_worker(Anvil::Region *r, int rx, int rz, string out_dir,
                 bool verbose);
void queue_offset(pair<int, int> *off);
void start_worker(int jobs);

#endif

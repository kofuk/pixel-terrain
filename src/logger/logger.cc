/* Logger, that can be used from different thread safely. */

#include <iostream>
#include <mutex>

#include "logger.hh"

using namespace std;

namespace mcmap::logger {
    namespace {
        mutex m;
    }

    void d(string message) {
        unique_lock<mutex> lock(m);
        cerr << message << endl;
    }

    void e(string message) {
        unique_lock<mutex> lock(m);
        cerr << message << endl;
    }

    void i(string message) {
        unique_lock<mutex> lock(m);
        cout << message << endl;
    }
} // namespace mcmap::logger

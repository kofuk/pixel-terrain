/* Logger, that can be used from different thread safely. */

#include <iostream>
#include <mutex>

#include "logger.hh"

namespace pixel_terrain::logger {
    namespace {
        std::mutex m;
    }

    void d(std::string message) {
        std::unique_lock<std::mutex> lock(m);
        std::cerr << message << std::endl;
    }

    void e(std::string message) {
        std::unique_lock<std::mutex> lock(m);
        std::cerr << message << std::endl;
    }

    void i(std::string message) {
        std::unique_lock<std::mutex> lock(m);
        std::cout << message << std::endl;
    }
} // namespace pixel_terrain::logger
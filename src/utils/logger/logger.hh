#ifndef LOGGER_HH
#define LOGGER_HH

#include <string>

namespace pixel_terrain::logger {
    void d(std::string message);
    void e(std::string message);
    void i(std::string message);
} // namespace pixel_terrain::logger

#endif

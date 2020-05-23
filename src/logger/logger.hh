#ifndef LOGGER_HH
#define LOGGER_HH

#include <string>

using namespace std;

namespace mcmap::logger {
    void d(string message);
    void e(string message);
    void i(string message);
} // namespace mcmap::logger

#endif

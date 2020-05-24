#ifndef WRITER_HH
#define WRITER_HH

#include <string>

using namespace std;

namespace mcmap::server {
    template <typename clazz> class writer {
        clazz *instance;

        writer(writer const &) = delete;
        writer operator=(writer const &) = delete;

    public:
        writer(clazz *instance) : instance(instance) {}

        ~writer() { delete instance; }

        void write_data(string const &data) { instance->write_data(data); }
        void write_data(int const data) { instance->write_data(data); }

        clazz *get() { return instance; }
    };
} // namespace mcmap::server

#endif

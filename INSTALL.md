# Installation Guide

## Build instruction

### Using system-installed libraries

Make sure the following libraries are installed and can be found with CMake:

- zlib
- libpng

If you installed libraries above, this project builds in the way as usual CMake project:

```shell
$ mkdir build && cd $_ && cmake .. && cmake --build .
```

### Superbuild

In this way, all dependencies are built (instead of using libraries on your system)
and statically linked.

Make sure the following required dependencies installed:

- CMake
- Make (or other build system, such as Ninja)
- C++ compiler which supports ISO C++17
- Git

I recommend installing the following optional dependencies:

- Boost C++ library (for unit test)

You can build this project by the following commands:

```shell
$ ./third_party/prepare_libs.sh
$ mkdir superbuild/build
$ cd superbuild/build
$ cmake ..
$ make
```

If you are on Windows, you may have to specify CMake generator (to avoid
Visual Studio project to be generated; which does not work).
You should be able to choose any single configuration build system.

If your system doesn't have commands to execute `prepare_libs.sh`,
you have to manually download tarballs (URLs are written in `third_party/dep`)
and apply patches in third_party directory.

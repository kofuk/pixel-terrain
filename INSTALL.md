# Installation Guide

## Build instruction

Make sure the following libraries are installed and can be found with CMake:

- zlib
- libpng

Windows hint: You can specify `CMAKE_PREFIX_PATH` CMake variable
to help CMake to find your library installation.

If you installed libraries above, this project builds in the way as usual
CMake project:

```shell
$ mkdir build && cd $_ && cmake .. && cmake --build .
```

Test executables aren't built by default.
You need to build test executables, then
you can run tests. (It also requires Boost Unit Test Framework)

```shell
$ cmake --build . --target test_executable test
```

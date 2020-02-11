cmake_minimum_required(VERSION 3.16)
project(mcmap CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_STANDARD_REQUIRED ON)

find_package(ZLIB)
find_package(PkgConfig)
pkg_check_modules(LIBPNG REQUIRED libpng)

add_subdirectory(src)

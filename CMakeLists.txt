cmake_minimum_required(VERSION 3.15)
project(mcmap CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_STANDARD_REQUIRED ON)

find_package(ZLIB REQUIRED)
find_package(Threads REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBPNG REQUIRED libpng)

include(cmake/test_utils.cmake)

add_subdirectory(block)
add_subdirectory(src)

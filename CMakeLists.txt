cmake_minimum_required(VERSION 3.15)
project(mcmap CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_STANDARD_REQUIRED ON)

include(ExternalProject)
ExternalProject_Add(optlib_project
  GIT_REPOSITORY https://github.com/kofuk/optlib.git
  GIT_TAG 941f5f7
  TEST_BEFORE_INSTALL YES
  CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PROJECT_SOURCE_DIR}/external -DCMAKE_BUILD_TYPE=Release)
add_library(optlib IMPORTED STATIC)
if(MSVC)
  set_target_properties(optlib PROPERTIES
    IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/external/lib/optlib.lib)
else()
  set_target_properties(optlib PROPERTIES
    IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/external/lib/liboptlib.a)
endif()

find_package(ZLIB REQUIRED)
find_package(PNG REQUIRED)
find_package(Threads REQUIRED)
find_package(PkgConfig REQUIRED)

include(cmake/test_utils.cmake)

string(JOIN "\" \"" LIBS_INCLUDE_DIRS_DIR_LOCALS ${ZLIB_INCLUDE_DIRS} ${PNG_INCLUDE_DIRS})
configure_file(src/dir-locals.el.in ${PROJECT_SOURCE_DIR}/src/.dir-locals.el @ONLY)
unset(LIBS_INCLUDE_DIRS_DIR_LOCALS)

string(JOIN "\n-I" COMPILE_FLAGS_TXT_CFLAGS
  -I${CMAKE_SOURCE_DIR}/external/include
  ${ZLIB_INCLUDE_DIRS} ${PNG_INCLUDE_DIRS} ${CMAKE_BINARY_DIR})
configure_file(compile_flags.txt.in ${PROJECT_SOURCE_DIR}/compile_flags.txt @ONLY)
unset(COMPILE_FLAGS_TXT_CFLAGS)

add_subdirectory(block)
add_subdirectory(src)

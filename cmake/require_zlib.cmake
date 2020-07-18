# Copyright (c) 2020 Koki Fukuda
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

find_package(ZLIB)
if(NOT ZLIB_FOUND)
  include(ExternalProject)
  ExternalProject_Add(zlib_project
    URL https://zlib.net/zlib-1.2.11.tar.xz
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PROJECT_SOURCE_DIR}/external -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE})
  set(ZLIB_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/external/include)
  set(ZLIB_LIBRARIES zlib)
  if(MSVC)
    if(CMAKE_BUILD_TYPE MATCHES "^[Dd][Ee][Bb][Uu][Gg]$")
      set(ZLIB_LIBRARY ${PROJECT_SOURCE_DIR}/external/lib/zlibstaticd.lib)
    else()
      set(ZLIB_LIBRARY ${PROJECT_SOURCE_DIR}/external/lib/zlibstatic.lib)
    endif()
  else()
    set(ZLIB_LIBRARY ${PROJECT_SOURCE_DIR}/external/lib/libz.a)
  endif()
  add_library(zlib IMPORTED STATIC)
  if(MSVC)
    set_target_properties(zlib PROPERTIES
      IMPORTED_LOCATION ${ZLIB_LIBRARY})
  else()
    set_target_properties(zlib PROPERTIES
      IMPORTED_LOCATION ${ZLIB_LIBRARY})
  endif()
  set(ZLIB_USE_EXTERNAL_PROJECT YES)
else()
  set(ZLIB_USE_EXTERNAL_PROJECT NO)
endif()

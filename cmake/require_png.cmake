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

find_package(PNG)
if(NOT PNG_FOUND)
  include(ExternalProject)
  ExternalProject_Add(png_project
    URL https://download.sourceforge.net/libpng/libpng-1.6.37.tar.xz
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PROJECT_SOURCE_DIR}/external -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DPNG_BUILD_ZLIB=NO -DZLIB_INCLUDE_DIR=${ZLIB_INCLUDE_DIRS}
    -DZLIB_LIBRARY=${ZLIB_LIBRARY})
  if(ZLIB_USE_EXTERNAL_PROJECT)
    add_dependencies(png_project zlib_project)
  endif()
  set(PNG_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/external/include)
  set(PNG_LIBRARIES png)
  add_library(png IMPORTED STATIC)
  set_target_properties(png_project PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${ZLIB_INCLUDE_DIRS}
    INTERFACE_LINK_LIBRARIES ${ZLIB_LIBRARIES})
    if(MSVC)
    if(CMAKE_BUILD_TYPE MATCHES "^[Dd][Ee][Bb][Uu][Gg]$")
      set_target_properties(png PROPERTIES
        IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/external/lib/libpng16_staticd.lib)
    else()
      set_target_properties(png PROPERTIES
        IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/external/lib/libpng16_static.lib)
    endif()
  else()
    set_target_properties(png PROPERTIES
      IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/external/lib/libpng.a)
  endif()

  set(PNG_USE_EXTERNAL_PROJECT YES)
else()
  set(PNG_USE_EXTERNAL_PROJECT NO)
endif()

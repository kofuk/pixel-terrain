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

add_library(image_generator STATIC blocks.cc color.cc generator.cc png.cc)
target_include_directories(image_generator PRIVATE SYSTEM ${PNG_INCLUE_DIRS})
target_include_directories(image_generator PRIVATE ${CMAKE_BINARY_DIR})
target_link_libraries(image_generator PUBLIC ${PNG_MOD_NAME} ${CMAKE_THREAD_LIBS_INIT})
add_dependencies(image_generator block_colors_data)

add_boost_test(color_test imagegen_color color_test.cc)
if(TARGET color_test)
  target_link_libraries(color_test image_generator)
endif()

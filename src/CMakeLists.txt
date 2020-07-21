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

add_executable(terrain2png terrain2png.cc)
add_dependencies(terrain2png block_colors_data)
target_link_libraries(terrain2png region_parser)
target_link_libraries(terrain2png logger)
target_link_libraries(terrain2png image_generator)
target_link_libraries(terrain2png regetopt_static)
install(TARGETS terrain2png DESTINATION bin)

add_executable(blockd blockd.cc)
target_include_directories(blockd PRIVATE SYSTEM ${PROJECT_SOURCE_DIR}/external/include)
target_link_libraries(blockd region_parser)
target_link_libraries(blockd logger)
target_link_libraries(blockd block_server)
target_link_libraries(blockd regetopt_static)
install(TARGETS blockd DESTINATION bin)

add_executable(nbt2xml nbt2xml.cc)
target_include_directories(nbt2xml PRIVATE SYSTEM ${PROJECT_SOURCE_DIR}/external/include)
target_link_libraries(nbt2xml nbt_pull_parser regetopt_static)
install(TARGETS nbt2xml DESTINATION bin)

add_executable(dumpnbt dumpnbt.cc)
add_dependencies(dumpnbt regetopt)
target_include_directories(dumpnbt PRIVATE SYSTEM ${PROJECT_SOURCE_DIR}/external/include)
target_link_libraries(dumpnbt region_parser)
target_link_libraries(dumpnbt regetopt_static)
install(TARGETS dumpnbt DESTINATION bin)

add_subdirectory(image)
add_subdirectory(nbt)
add_subdirectory(server)
add_subdirectory(utils/logger)

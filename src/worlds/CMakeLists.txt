# SPDX-License-Identifier: MIT

add_library(worlds STATIC worlds.cc)
target_link_libraries(worlds PRIVATE nbtpullparser)

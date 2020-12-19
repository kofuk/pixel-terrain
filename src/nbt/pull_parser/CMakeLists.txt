# SPDX-License-Identifier: MIT

set(PULL_PARSER_SRCS nbt_pull_parser.cc)

add_library(nbtpullparser STATIC ${PULL_PARSER_SRCS})

add_boost_test(nbt_pull_parser_test _nbt_pull_parser_test nbt_pull_parser_test.cc)
if(TARGET nbt_pull_parser_test)
  target_link_libraries(nbt_pull_parser_test nbtpullparser)
endif()

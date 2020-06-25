add_library(nbt_pull_parser STATIC nbt_pull_parser.cc)

add_boost_test(nbt_pull_parser_test _nbt_pull_parser_test nbt_pull_parser_test.cc)
if(TARGET nbt_pull_parser_test)
  target_link_libraries(nbt_pull_parser_test nbt_pull_parser)
endif()

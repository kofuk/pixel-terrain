target_sources(pixel-terrain PRIVATE chunk.cc region.cc nbt.cc utils.cc)

add_boost_test(nbt_pull_parser_test _nbt_pull_parser_test nbt_pull_parser.cc nbt_pull_parser_test.cc)

add_executable(nbt2xml nbt_to_xml.cc nbt_pull_parser.cc)
install(TARGETS nbt2xml DESTINATION bin)

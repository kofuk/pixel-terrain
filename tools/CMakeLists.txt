add_executable(block_data_generator block_data_generator.cc)

add_boost_test(block_data_generator_test blocks_generator_test block_data_generator.cc)
if(TARGET block_data_generator_test)
  target_compile_definitions(block_data_generator_test PRIVATE -DTEST)
endif()

add_subdirectory(editor-aux)

target_sources(mcmap PRIVATE blocks.cc color.cc generator.cc worker.cc PNG.cc)

add_executable(color_test color_test.cc color.cc)
target_include_directories(color_test PRIVATE ${Boost_INLUDE_DIRS})
target_compile_definitions(color_test PRIVATE "BOOST_TEST_DYN_LINK=1")
target_compile_definitions(color_test PRIVATE "BOOST_TEST_MAIN")
target_link_libraries(color_test ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})

add_test(NAME imagegen_color COMMAND color_test)

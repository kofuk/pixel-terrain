target_sources(mcmap PRIVATE server.cc writer_unix.cc)

add_executable(writer_unix_test writer_unix_test.cc writer_unix.cc)
target_include_directories(writer_unix_test PRIVATE ${Boost_INLUDE_DIRS})
target_compile_definitions(writer_unix_test PRIVATE "BOOST_TEST_DYN_LINK=1")
target_compile_definitions(writer_unix_test PRIVATE "BOOST_TEST_MAIN")
target_link_libraries(writer_unix_test ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})

add_executable(writer_string_test writer_string_test.cc writer_string.cc)
target_include_directories(writer_string_test PRIVATE ${Boost_INLUDE_DIRS})
target_compile_definitions(writer_string_test PRIVATE "BOOST_TEST_DYN_LINK=1")
target_compile_definitions(writer_string_test PRIVATE "BOOST_TEST_MAIN")
target_link_libraries(writer_string_test ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})

add_executable(request_test request_test.cc)
target_include_directories(request_test PRIVATE ${Boost_INLUDE_DIRS})
target_compile_definitions(request_test PRIVATE "BOOST_TEST_DYN_LINK=1")
target_compile_definitions(request_test PRIVATE "BOOST_TEST_MAIN")
target_link_libraries(request_test ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})

add_test(NAME blockserver_writer_unix COMMAND writer_unix_test)
add_test(NAME blockserver_writer_string COMMAND writer_string_test)
add_test(NAME blockserver_request COMMAND request_test)

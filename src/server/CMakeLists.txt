add_library(block_server STATIC request.cc server.cc writer_string.cc)
target_include_directories(block_server PRIVATE SYSTEM ${PROJECT_SOURCE_DIR}/external/include)
target_link_libraries(block_server INTERFACE ${ZLIB_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} optlib)
target_link_libraries(block_server PRIVATE logger region_parser)

if(MSVC)
  target_sources(block_server PRIVATE reader_generic.cc server_generic.cc writer_generic.cc)
else()
  target_sources(block_server PRIVATE reader_unix.cc server_unix_socket.cc writer_unix.cc)

  add_boost_test(writer_unix_test blockserver_writer_unix writer_unix_test.cc)
  if(TARGET writer_unix_test)
    target_link_libraries(writer_unix_test block_server)
  endif()
endif()

add_boost_test(writer_string_test blockserver_writer_string writer_string_test.cc)
if(TARGET writer_string_test)
  target_link_libraries(writer_string_test block_server)
endif()

add_boost_test(request_test blockserver_request request_test.cc)
if(TARGET request_test)
  target_link_libraries(request_test block_server)
endif()
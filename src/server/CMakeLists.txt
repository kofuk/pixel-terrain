# SPDX-License-Identifier: MIT

set(SERVER_SRCS
  request.cc
  server.cc
  writer_string.cc
  reader_unix.cc
  server_unix_socket.cc
  writer_unix.cc)

add_library(pixtserver STATIC ${SERVER_SRCS})
target_link_libraries(pixtserver INTERFACE ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(pixtserver PRIVATE logger mcregion)

add_boost_test(writer_unix_test blockserver_writer_unix writer_unix_test.cc)
if(TARGET writer_unix_test)
  target_link_libraries(writer_unix_test pixtserver)
endif()

add_boost_test(writer_string_test blockserver_writer_string writer_string_test.cc)
if(TARGET writer_string_test)
  target_link_libraries(writer_string_test pixtserver)
endif()

add_boost_test(request_test blockserver_request request_test.cc)
if(TARGET request_test)
  target_link_libraries(request_test pixtserver)
endif()

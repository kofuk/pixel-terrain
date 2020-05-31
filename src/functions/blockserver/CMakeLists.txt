target_sources(mcmap PRIVATE reader_unix.cc request.cc server.cc server_unix_socket.cc writer_unix.cc)

add_boost_test(writer_unix_test blockserver_writer_unix writer_unix_test.cc writer_unix.cc)
add_boost_test(writer_string_test blockserver_writer_string writer_string_test.cc writer_string.cc)
add_boost_test(request_test blockserver_request request_test.cc request.cc)

# Copyright (c) 2020 Koki Fukuda
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

add_library(block_server STATIC request.cc server.cc writer_string.cc)
target_link_libraries(block_server INTERFACE ${CMAKE_THREAD_LIBS_INIT})
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

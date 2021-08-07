srcs = [
  'request.cc',
  'server.cc',
  'writer_string.cc',
  'reader_unix.cc',
  'server_unix_socket.cc',
  'writer_unix.cc'
]

server_lib = static_library(
  'pixtserver', srcs,
  include_directories : project_inc,
  link_with : logger_lib)

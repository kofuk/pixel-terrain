# SPDX-License-Identifier: MIT

add_executable(threaded_worker_test EXCLUDE_FROM_ALL
  threaded_worker_test.cc)
target_link_libraries(threaded_worker_test PUBLIC ${CMAKE_THREAD_LIBS_INIT})

add_custom_target(run_threaded_worker_test
  DEPENDS threaded_worker_test
  COMMENT "Running threaded_worker_test..."
  COMMAND ${CMAKE_CURRENT_BINARY_DIR}/threaded_worker_test
  VERBATIM)

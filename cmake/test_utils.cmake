enable_testing()
find_package(Boost COMPONENTS unit_test_framework)

add_custom_target(test_executable)

function(add_boost_test target_name test_name)
  if(NOT Boost_FOUND)
    return()
  endif()

  add_executable(${target_name} ${ARGN})
  target_include_directories(${target_name} PRIVATE ${Boost_INLUDE_DIRS})
  target_compile_definitions(${target_name} PRIVATE "BOOST_TEST_DYN_LINK=1")
  target_compile_definitions(${target_name} PRIVATE "BOOST_TEST_MAIN")
  target_link_libraries(${target_name} ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})

  add_dependencies(test_executable ${target_name})

  add_test(NAME ${test_name} COMMAND ${target_name})
endfunction()

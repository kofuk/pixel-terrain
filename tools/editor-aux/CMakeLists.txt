add_executable(editor-aux EXCLUDE_FROM_ALL main.cc)

add_boost_test(editor_aux_test tools_editor_aux main.cc)
if(Boost_FOUND)
  target_compile_definitions(editor_aux_test PRIVATE -DTEST)
endif()

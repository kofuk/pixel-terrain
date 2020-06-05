add_executable(mcmap main.cc)
add_dependencies(mcmap block_colors_data)
target_include_directories(mcmap PRIVATE SYSTEM ${ZLIB_INCLUDE_DIRS} ${PNG_INCLUE_DIRS})
target_include_directories(mcmap PRIVATE ${CMAKE_BINARY_DIR})
target_link_libraries(mcmap ${ZLIB_LIBRARIES} ${PNG_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

add_subdirectory(functions)
add_subdirectory(logger)
add_subdirectory(nbt)

install(TARGETS mcmap DESTINATION bin)

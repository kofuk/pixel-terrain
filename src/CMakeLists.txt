add_executable(pixel-terrain main.cc)
add_dependencies(pixel-terrain block_colors_data optlib)
if(ZLIB_USE_EXTERNAL_PROJECT)
  add_dependencies(pixel-terrain zlib)
endif()
if(PNG_USE_EXTERNAL_PROJECT)
  add_dependencies(pixel-terrain png)
endif()
target_include_directories(pixel-terrain PRIVATE SYSTEM ${ZLIB_INCLUDE_DIRS} ${PNG_INCLUE_DIRS})
target_include_directories(pixel-terrain PRIVATE ${CMAKE_BINARY_DIR})
target_include_directories(pixel-terrain PRIVATE SYSTEM ${PROJECT_SOURCE_DIR}/external/include)
target_link_libraries(pixel-terrain ${ZLIB_LIBRARIES} ${PNG_LIBRARIES}
  ${CMAKE_THREAD_LIBS_INIT} optlib)

add_subdirectory(functions)
add_subdirectory(logger)
add_subdirectory(nbt)

install(TARGETS pixel-terrain DESTINATION bin)

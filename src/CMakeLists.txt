# SPDX-License-Identifier: MIT

include_directories(SYSTEM ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(terrain2png terrain2png.cc)
add_dependencies(terrain2png block_colors_data)
target_link_libraries(terrain2png PRIVATE
  mcregion
  logger
  pixtimage
  ${REGETOPT_MOD_NAME})
install(TARGETS terrain2png DESTINATION bin)

add_executable(blockd blockd.cc)
target_include_directories(blockd PRIVATE SYSTEM ${PROJECT_SOURCE_DIR}/external/include)
target_link_libraries(blockd PRIVATE
  mcregion
  logger
  pixtserver
  ${REGETOPT_MOD_NAME})
install(TARGETS blockd DESTINATION bin)

add_executable(nbt2xml nbt2xml.cc)
target_include_directories(nbt2xml PRIVATE SYSTEM ${PROJECT_SOURCE_DIR}/external/include)
target_link_libraries(nbt2xml PRIVATE
  nbtpullparser
  ${REGETOPT_MOD_NAME})
install(TARGETS nbt2xml DESTINATION bin)

add_executable(dumpnbt dumpnbt.cc)
target_include_directories(dumpnbt PRIVATE SYSTEM ${PROJECT_SOURCE_DIR}/external/include)
target_link_libraries(dumpnbt PRIVATE
  mcregion
  ${REGETOPT_MOD_NAME})
install(TARGETS dumpnbt DESTINATION bin)

add_subdirectory(graphics)
add_subdirectory(image)
add_subdirectory(nbt)
add_subdirectory(server)
add_subdirectory(logger)

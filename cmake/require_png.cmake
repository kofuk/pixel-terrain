find_package(PNG)
if(NOT PNG_FOUND)
  include(ExternalProject)
  ExternalProject_Add(png_project
    URL https://download.sourceforge.net/libpng/libpng-1.6.37.tar.xz
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PROJECT_SOURCE_DIR}/external -DCMAKE_BUILD_TYPE=Release)
  add_library(png IMPORTED STATIC)
  if(ZLIB_USE_EXTERNAL_PROJECT)
    add_dependencies(png zlib)
  endif()
  set_target_properties(png PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${ZLIB_INCLUDE_DIRS}
    INTERFACE_LINK_LIBRARIES ${ZLIB_LIBRARIES})

  if(MSVC)
    set_target_properties(png PROPERTIES
      IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/external/lib/png.lib)
  else()
    set_target_properties(png PROPERTIES
      IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/external/lib/libpng.a)
  endif()
  set(PNG_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/external/include)
  set(PNG_LIBRARIES png)
  set(PNG_USE_EXTERNAL_PROJECT YES)
else()
  set(PNG_USE_EXTERNAL_PROJECT NO)
endif()

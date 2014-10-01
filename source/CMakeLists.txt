Cmake_minimum_required(VERSION 2.8)
project(scigma)

enable_language(Fortran)
add_subdirectory(external)

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set(CMAKE_MACOSX_RPATH OFF)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Werror -Weverything -Wno-c++98-compat-pedantic -Wno-documentation -Wno-unknown-pragmas")
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wpedantic -Wextra -Werror -Wno-unknown-pragmas")	
endif()

add_subdirectory(num)
add_subdirectory(gui)
add_subdirectory(dat)

include_directories(external/tinythread)
add_library(scigma SHARED log.cpp pythonwrapper.cpp $<TARGET_OBJECTS:guiobjects> $<TARGET_OBJECTS:numobjects> $<TARGET_OBJECTS:datobjects>)
target_link_libraries(scigma lapack odessa tinythread libpng zlib glew glfw anttweakbar ${GLFW_LIBRARIES})

Cmake_minimum_required(VERSION 3.0)
project(scigma)

enable_language(Fortran)

add_subdirectory(external)

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
set(CMAKE_MACOSX_RPATH OFF)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 \
-Werror \
-Weverything \
-Wno-c++98-compat-pedantic \
-Wno-documentation \
-Wno-documentation-unknown-command \
-Wno-unknown-pragmas \
-Wno-unknown-warning-option \
-Wno-shadow-field \
-Wno-reserved-id-macro \
-Wno-zero-as-null-pointer-constant \
-Wno-disabled-macro-expansion \
-Wno-comma")
else()
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread \
-Wall \
-Wpedantic \
-Wextra \
-Werror \
-Wno-unknown-pragmas")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -pthread")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pthread")
endif()

add_subdirectory(common)
add_subdirectory(num)
add_subdirectory(gui)
add_subdirectory(dat)

file(WRITE empty.cpp "")
add_library(scigma SHARED empty.cpp) 

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
target_link_libraries(scigma "-Wl,-force_load"
  			gui "-Wl,-force_load" num "-Wl,-force_load" dat "-Wl,-force_load" common
			odessa auto lapack tinythread
			libpng zlib glew glfw
			anttweakbar ${GLFW_LIBRARIES})
else()
target_link_libraries(scigma "-Wl,--whole-archive"
			gui num dat common "-Wl,--no-whole-archive"	
			odessa auto lapack tinythread
			libpng zlib glew glfw
			anttweakbar ${GLFW_LIBRARIES})
endif()

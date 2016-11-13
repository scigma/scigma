set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -Wno-documentation -Wno-unknown-pragmas")	
include_directories(../../external/tinythread)
include_directories(../../external/loki)
include_directories(../../external/catch)
add_executable(datutest datutest.cpp
		     wave_test.cpp
		     mesh_test.cpp)

add_executable(datvtest datvtest.cpp)
		     
target_link_libraries(datutest dat common tinythread)
target_link_libraries(datvtest dat common tinythread)

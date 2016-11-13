set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -Wno-documentation -Wno-unknown-pragmas")	
include_directories(../../external/catch)
add_executable(guiutest guiutest.cpp
		     application_test.cpp
		     glbuffer_test.cpp)

add_executable(guivtest guivtest.cpp)
		     
target_link_libraries(guiutest common gui tinythread)
target_link_libraries(guivtest common gui tinythread)

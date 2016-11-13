set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -Wno-documentation -Wno-unknown-pragmas")	
include_directories(../../external/catch)
add_executable(commonutest commonutest.cpp
		     log_test.cpp
		     blob_test.cpp
		     pythonobject_test.cpp)

add_executable(commonvtest commonvtest.cpp)
		     
target_link_libraries(commonutest common tinythread)
target_link_libraries(commonvtest common tinythread)

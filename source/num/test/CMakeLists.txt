set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -Wno-documentation -Wno-unknown-pragmas")	
include_directories(../../external/catch)
add_executable(numutest numutest.cpp
		     internalequationsystem_test.cpp
		     externalequationsystem_test.cpp
		     externalequationsystem_test_p.cpp
		     externalequationsystem_test_t.cpp
		     externalequationsystem_test_pt.cpp				
		     externalequationsystem_test_shared.cpp
		     odessa_test.cpp
		     integrationstepper_test.cpp
		     mapstepper_test.cpp)

add_executable(numvtest numvtest.cpp externalequationsystem_test_shared.cpp)
		     
target_link_libraries(numutest num common odessa lapack tinythread)
target_link_libraries(numvtest num common tinythread)

include_directories(../external/tinythread)
add_library(common STATIC log.cpp blob.cpp logwrapper.cpp
		   	  blobwrapper.cpp util.cpp)
set_target_properties(common PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
add_subdirectory(test)

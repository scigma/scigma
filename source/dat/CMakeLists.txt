include_directories(../external/tinythread)
include_directories(../external/loki)
include_directories(../external/glew)
include_directories(../external/glfw-3.1.2/include/GLFW)

add_library(dat STATIC mesh.cpp datwrapper.cpp)
set_target_properties(dat PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
target_link_libraries(dat common tinythread libpng zlib glew glfw anttweakbar ${GLFW_LIBRARIES})

add_subdirectory(test)

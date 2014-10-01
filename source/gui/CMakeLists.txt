if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  add_definitions(-DSCIGMA_USE_OPENGL_3_2)
endif()
include_directories(../external/tinythread)
include_directories(../external/glew)
include_directories(../external/glfw3/include/GLFW)
include_directories(../external/loki)
include_directories(../external/lpng1610)
include_directories(../external/anttweakbar)
add_library(guiobjects OBJECT application.cpp glwindow.cpp font.cpp viewingvolume.cpp viewingarea.cpp glcontext.cpp navigator.cpp graph.cpp curve.cpp picker.cpp glutil.cpp drawable.cpp console.cpp ruler.cpp cosy.cpp atwpanel.cpp marker.cpp screentext.cpp spacetext.cpp pythonwrapper.cpp)
set_target_properties(guiobjects PROPERTIES POSITION_INDEPENDENT_CODE TRUE)

# this is necessary because libpngconf.h is generated in the libpng build;
# without the following line, scigma will be compiled before libpng and
# libpngconf.h will not be found
add_dependencies(guiobjects libpng)

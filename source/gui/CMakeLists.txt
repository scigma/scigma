if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
   add_definitions(-DSCIGMA_USE_OPENGL_3_2)
endif()
include_directories(../external/tinythread)
include_directories(../external/glew)
include_directories(../external/glfw-3.1.2/include/GLFW)
include_directories(../external/loki)
include_directories(../external/lpng1610)
include_directories(../external/anttweakbar)

add_library(gui STATIC application.cpp glutil.cpp glcontext.cpp
		viewingarea.cpp viewingvolume.cpp glwindow.cpp
		font.cpp marker.cpp screentext.cpp spacetext.cpp
		ruler.cpp picker.cpp drawable.cpp navigator.cpp
		cosy.cpp graph.cpp bundle.cpp sheet.cpp
		console.cpp atwpanel.cpp mouse.cpp
		applicationwrapper.cpp glwindowwrapper.cpp
		cosywrapper.cpp consolewrapper.cpp pickerwrapper.cpp
		navigatorwrapper.cpp mousewrapper.cpp atwpanelwrapper.cpp graphwrapper.cpp)
set_target_properties(gui PROPERTIES POSITION_INDEPENDENT_CODE TRUE)

# libpng must be built before gui, because it creates a header needed by gui
# not the right way to do this, but it seems to work
target_link_libraries(gui libpng)

add_subdirectory(test)

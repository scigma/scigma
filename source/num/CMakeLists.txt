enable_language(Fortran)
include_directories(../external/tinythread)
include_directories(../external/loki)
include_directories(../external/glew)
include_directories(../external/glfw-3.1.2/include/GLFW)

add_library(num STATIC function.cpp operator.cpp parser.cpp
		equationsystem.cpp internalequationsystem.cpp externalequationsystem.cpp
		odessa.cpp
		stepper.cpp integrationstepper.cpp mapstepper.cpp auto.cpp
		poincarestepper.cpp mapmanifoldstepper.cpp
		newton.cpp eigen.cpp function.cpp operator.cpp task.cpp
		sweeping.cpp
		guessingwrapper.cpp
		iterationwrapper.cpp
		continuationwrapper.cpp
		equationsystemwrapper.cpp
		manifoldwrapper.cpp
		sweepingwrapper.cpp
		taskwrapper.cpp) 
set_target_properties(num PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
target_link_libraries(num tinythread common odessa auto)
add_subdirectory(test)

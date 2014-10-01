enable_language(Fortran)
file(GLOB lapackfiles "*.f")
add_library(lapack STATIC ${lapackfiles})
set_target_properties(lapack PROPERTIES POSITION_INDEPENDENT_CODE TRUE)

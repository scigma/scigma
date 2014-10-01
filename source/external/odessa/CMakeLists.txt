enable_language(Fortran)
add_library(odessa STATIC odessa.f)
set_target_properties(odessa PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
target_link_libraries(odessa lapack)
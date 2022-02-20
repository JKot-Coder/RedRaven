
add_subdirectory(rfx)
add_subdirectory(tests)

#================================ Dependencies ===============================#
#================================== cxxopts ==================================#
FetchContent_Declare(
  cxxopts
  GIT_REPOSITORY https://github.com/jarro2783/cxxopts
  GIT_TAG        v3.0.0
)
FetchContent_MakeAvailable(cxxopts)
set_property(TARGET cxxopts PROPERTY FOLDER "Dependencies")
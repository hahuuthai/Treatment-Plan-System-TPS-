# CMakeList.txt : CMake project for CTViewerApp, include source and define
# project specific logic here.
#

# Add source to this project's executable.
add_executable (CTViewerApp "CTViewerApp.cpp" "CTViewerApp.h")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET CTViewerApp PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.

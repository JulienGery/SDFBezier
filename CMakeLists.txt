cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "-O2")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(EXPORT_COMPILE_COMMANDS ON)

project(SDFBezier VERSION 0.1)

add_executable(SDFBezier main.cpp)
add_subdirectory(Mylib)
add_subdirectory(TMP)

target_link_libraries(SDFBezier PUBLIC Renderer)
target_link_libraries(SDFBezier PUBLIC Glyph)
target_link_libraries(SDFBezier PUBLIC msdfgen)

target_include_directories(SDFBezier PUBLIC "${PROJECT_BINARY_DIR}" "${PROJECT_SOURCE_DIR}/Mylib" "include" "${PROJECT_SOURCE_DIR}/TMP" )

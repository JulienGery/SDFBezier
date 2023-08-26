find_package(Freetype REQUIRED)
find_package(glm REQUIRED)
find_package(Vulkan REQUIRED)
set(CMAKE_CXX_FLAGS "-O2")

add_library(Renderer Renderer.h Renderer.cpp)
add_library(Glyph Glyph.h Glyph.cpp Bezier.cpp Contour.cpp)

include_directories(Renderer PUBLIC include)
include_directories(Glyph PUBLIC include)

target_link_libraries(Renderer PUBLIC vulkan glm)
target_link_libraries(Glyph PUBLIC freetype glm)

cmake_minimum_required(VERSION 3.24)
project(example VERSION 0.0.1)

include(cmake/prelude.cmake)
include(cmake/mold.cmake)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_COLOR_DIAGNOSTICS ON) # You might want to enable this (CMake 3.24+)

find_package(fmt REQUIRED)
find_package(glbinding REQUIRED)
find_package(glad REQUIRED)
find_package(glfw3 3.3 REQUIRED)
find_package(glm REQUIRED)
find_package(VulkanHeaders REQUIRED)
find_package(VulkanLoader REQUIRED)
find_package(linmath.h REQUIRED)
find_package(stb REQUIRED)
find_package(tinycthread REQUIRED)

add_subdirectory(lib/getopt)

# the library
option(GLFW_CPP_VULKAN_SUPPORT "vulkan support" ON)
add_subdirectory(lib/glfw-cpp)

# the examples
add_subdirectory(source/classic)
add_subdirectory(source/new)

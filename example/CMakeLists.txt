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
find_package(glm REQUIRED)

add_subdirectory(lib/glfw-cpp)

add_executable(example source/example.cpp)
target_include_directories(example PRIVATE source)
target_compile_options(example PRIVATE -Wall -Wextra -Wconversion)
target_link_libraries(example PRIVATE fmt::fmt glbinding::glbinding glm::glm
                                      glfw-cpp)

# sanitizer
target_compile_options(example PRIVATE -fsanitize=thread)
target_link_options(example PRIVATE -fsanitize=thread)

# copy assets to build directory
add_custom_command(
  TARGET example
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/asset
          $<TARGET_FILE_DIR:example>/asset)

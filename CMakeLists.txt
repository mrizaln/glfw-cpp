cmake_minimum_required(VERSION 3.12)

project(
  glfw-cpp
  VERSION 0.0.0
  LANGUAGES CXX
  DESCRIPTION "C++ GLFW wrapper with multi-window and multithreading in mind")

find_package(glfw3 3.4 REQUIRED)

set(glfw-cpp_SOURCES src/window.cpp src/window_manager.cpp src/context.cpp)
add_library(glfw-cpp STATIC ${glfw-cpp_SOURCES})
target_include_directories(glfw-cpp PUBLIC include)
target_link_libraries(glfw-cpp PUBLIC glfw)
target_compile_features(glfw-cpp INTERFACE cxx_std_20)
set_target_properties(glfw-cpp PROPERTIES CXX_EXTENSIONS OFF)

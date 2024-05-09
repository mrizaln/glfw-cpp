cmake_minimum_required(VERSION 3.12)

project(
  glfw-cpp
  VERSION 0.4.0
  LANGUAGES CXX
  DESCRIPTION "C++ GLFW wrapper with multi-window and multithreading in mind")

# Enable Vulkan support (requires Vulkan SDK)
option(GLFW_CPP_VULKAN_SUPPORT "Enable Vulkan support" OFF)

find_package(glfw3 3.3 REQUIRED)

set(glfw-cpp_SOURCES source/window.cpp source/window_manager.cpp
                     source/instance.cpp source/monitor.cpp source/input.cpp)

add_library(glfw-cpp STATIC ${glfw-cpp_SOURCES})
target_include_directories(glfw-cpp PUBLIC include source)
target_compile_features(glfw-cpp INTERFACE cxx_std_20)
set_target_properties(glfw-cpp PROPERTIES CXX_EXTENSIONS OFF)
target_link_libraries(glfw-cpp PUBLIC glfw)

if(GLFW_CPP_VULKAN_SUPPORT)
  find_package(Vulkan REQUIRED)

  target_sources(glfw-cpp PRIVATE source/vulkan.cpp)
  target_compile_definitions(glfw-cpp PUBLIC GLFW_CPP_VULKAN_SUPPORT)
  target_link_libraries(glfw-cpp PUBLIC Vulkan::Vulkan)
endif()

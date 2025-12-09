cmake_minimum_required(VERSION 3.24)
project(example VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT EMSCRIPTEN)
  find_package(VulkanHeaders REQUIRED)
  find_package(VulkanLoader REQUIRED)
  find_package(glfw3 3.4 REQUIRED)

  include(cmake/fetched-libs.cmake)

  add_subdirectory(source/classic)
  add_subdirectory(source/new)
else()
  add_subdirectory(source/emscripten)
endif()

cmake_minimum_required(VERSION 3.12)

project(
  glfw-cpp
  VERSION 0.12.2
  LANGUAGES CXX
  DESCRIPTION "C++ GLFW wrapper with multi-window and multithreading in mind"
)

string(
  COMPARE EQUAL
  "${CMAKE_CURRENT_SOURCE_DIR}"
  "${CMAKE_SOURCE_DIR}"
  GLFW_CPP_STANDALONE
)

option(GLFW_CPP_BUILD_EXAMPLES "Build example programs" ${GLFW_CPP_STANDALONE})
option(GLFW_CPP_BUILD_TESTS "Build test programs" ${GLFW_CPP_STANDALONE})

set(
  GLFW_CPP_SOURCES
  source/window.cpp
  source/instance.cpp
  source/monitor.cpp
  source/input.cpp
  source/event.cpp
)

add_library(glfw-cpp STATIC ${GLFW_CPP_SOURCES})
target_include_directories(glfw-cpp PUBLIC include)
target_compile_features(glfw-cpp PRIVATE cxx_std_20)
set_target_properties(glfw-cpp PROPERTIES CXX_EXTENSIONS OFF)

if(EMSCRIPTEN)
  target_sources(glfw-cpp PRIVATE source/emscripten.cpp)

  target_compile_options(glfw-cpp PUBLIC "--use-port=contrib.glfw3")
  target_link_options(glfw-cpp PUBLIC "--use-port=contrib.glfw3")
else()
  target_sources(glfw-cpp PRIVATE source/vulkan.cpp)

  find_package(glfw3 3.4 REQUIRED)
  target_link_libraries(glfw-cpp PUBLIC glfw)
endif()

if(GLFW_CPP_BUILD_EXAMPLES)
  add_subdirectory(example)
endif()

if(GLFW_CPP_BUILD_TESTS AND NOT EMSCRIPTEN)
  add_subdirectory(test)
endif()

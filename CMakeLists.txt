cmake_minimum_required(VERSION 3.12)

project(
    glfw-cpp
    VERSION 0.12.0
    LANGUAGES CXX
    DESCRIPTION "C++ GLFW wrapper with multi-window and multithreading in mind"
)

option(GLFW_CPP_EMSCRIPTEN_PORT "Use emscripten port" OFF)

set(GLFW_CPP_SOURCES
    source/window.cpp
    source/instance.cpp
    source/monitor.cpp
    source/input.cpp
    source/event.cpp
    source/vulkan.cpp
)

add_library(glfw-cpp STATIC ${GLFW_CPP_SOURCES})
target_include_directories(glfw-cpp PUBLIC include)
target_compile_features(glfw-cpp PRIVATE cxx_std_20)
set_target_properties(glfw-cpp PROPERTIES CXX_EXTENSIONS OFF)

if(GLFW_CPP_EMSCRIPTEN_PORT)
    if(NOT EMSCRIPTEN)
        message(FATAL_ERROR "Emscripten port enabled but not using emscripten")
    else()
        message(STATUS "glfw-cpp: GLFW_CPP_EMSCRIPTEN_PORT option set - emscripten port enabled.")
    endif()

    target_compile_options(glfw-cpp PUBLIC "--use-port=contrib.glfw3")
    target_link_options(glfw-cpp PUBLIC "--use-port=contrib.glfw3")
else()
    find_package(glfw3 3.4 REQUIRED)
    target_link_libraries(glfw-cpp PUBLIC glfw)
endif()

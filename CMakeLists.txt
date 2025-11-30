cmake_minimum_required(VERSION 3.12)

project(
    glfw-cpp
    VERSION 0.12.0
    LANGUAGES CXX
    DESCRIPTION "C++ GLFW wrapper with multi-window and multithreading in mind"
)

# Enable Vulkan support (requires Vulkan SDK)
option(GLFW_CPP_VULKAN_SUPPORT "Enable Vulkan support" OFF)
option(GLFW_CPP_EMSCRIPTEN_PORT "Use emscripten port" OFF)

if(GLFW_CPP_VULKAN_SUPPORT AND GLFW_CPP_EMSCRIPTEN_PORT)
    message(FATAL_ERROR "Vulkan support can't be enabled together with emscripten")
endif()

if(GLFW_CPP_EMSCRIPTEN_PORT)
    if(NOT EMSCRIPTEN)
        message(FATAL_ERROR "Emscripten port enabled but not using emscripten")
    else()
        message(STATUS "glfw-cpp: GLFW_CPP_EMSCRIPTEN_PORT option set - emscripten port enabled.")
    endif()
else()
    find_package(glfw3 3.3 REQUIRED)
endif()

set(glfw-cpp_SOURCES
    source/window.cpp
    source/instance.cpp
    source/monitor.cpp
    source/input.cpp
    source/event.cpp
)

add_library(glfw-cpp STATIC ${glfw-cpp_SOURCES})
target_include_directories(glfw-cpp PUBLIC include)
target_compile_features(glfw-cpp PRIVATE cxx_std_20)
set_target_properties(glfw-cpp PROPERTIES CXX_EXTENSIONS OFF)
target_link_libraries(glfw-cpp PUBLIC glfw)

if(GLFW_CPP_EMSCRIPTEN_PORT)
    target_compile_options(glfw-cpp PUBLIC "--use-port=contrib.glfw3")
    target_link_options(glfw-cpp PUBLIC "--use-port=contrib.glfw3")
endif()

if(GLFW_CPP_VULKAN_SUPPORT)
    message(STATUS "glfw-cpp: GLFW_CPP_VULKAN_SUPPORT option set - vulkan support enabled.")

    find_package(Vulkan REQUIRED)

    target_sources(glfw-cpp PRIVATE source/vulkan.cpp)
    target_compile_definitions(glfw-cpp PUBLIC GLFW_CPP_VULKAN_SUPPORT)
    target_link_libraries(glfw-cpp PUBLIC Vulkan::Vulkan)
endif()

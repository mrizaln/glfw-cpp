cmake_minimum_required(VERSION 3.24)
project(example VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT EMSCRIPTEN)
    include(cmake/prelude.cmake)
    include(cmake/mold.cmake)

    find_package(VulkanHeaders REQUIRED)
    find_package(VulkanLoader REQUIRED)
    find_package(fmt REQUIRED)
    find_package(glbinding REQUIRED)
    find_package(glfw3 3.4 REQUIRED)
    find_package(glm REQUIRED)
    find_package(imgui REQUIRED)
    find_package(khrplatform REQUIRED)
    find_package(linmath.h REQUIRED)
    find_package(stb REQUIRED)
    find_package(tinycthread REQUIRED)

    include(cmake/imgui-bindings.cmake) # imgui bindings
    add_subdirectory(lib/getopt)

    # the library
    option(GLFW_CPP_VULKAN_SUPPORT "vulkan support" ON)
    add_subdirectory(lib/glfw-cpp)

    # the examples
    add_subdirectory(source/classic)
    add_subdirectory(source/new)
else()
    # the library
    option(GLFW_CPP_EMSCRIPTEN_PORT "emscripten port" ON)
    add_subdirectory(lib/glfw-cpp)

    # the example
    add_subdirectory(source/emscripten)
endif()

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

option(GLFW_CPP_VULKAN_SUPPORT "vulkan support" ON)
add_subdirectory(lib/glfw-cpp)

function(create_an_executable NAME)
  cmake_parse_arguments(ARGS "DO_SANITIZE" "" "LIBS" ${ARGN})

  add_executable(${NAME} source/${NAME}.cpp)
  target_include_directories(${NAME} PRIVATE source)
  target_compile_options(${NAME} PRIVATE -Wall -Wextra -Wconversion)
  target_link_libraries(${NAME} PRIVATE ${ARGS_LIBS})

  if(ARGS_DO_SANITIZE)
    target_compile_options(${NAME} PRIVATE -fsanitize=thread)
    target_link_options(${NAME} PRIVATE -fsanitize=thread)
  endif()
endfunction()

# cmake-format: off
create_an_executable(single
  DO_SANITIZE
  LIBS fmt::fmt glad::glad glfw-cpp)

create_an_executable(multi
  DO_SANITIZE
  LIBS fmt::fmt glbinding::glbinding glm::glm glfw-cpp)

create_an_executable(multi_multi_manager
  DO_SANITIZE
  LIBS fmt::fmt glad::glad glfw-cpp)

create_an_executable(monitor
  DO_SANITIZE
  LIBS fmt::fmt glfw-cpp)

# even though the vulkan example does not have any multithreading, it fails with DEADLYSIGNAL on my machine...
create_an_executable(vulkan
  # DO_SANITIZE
  LIBS fmt::fmt vulkan-headers::vulkan-headers Vulkan::Loader glfw-cpp)
# cmake-format: on

# copy assets to build directory
add_custom_command(
  TARGET multi
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/asset
          $<TARGET_FILE_DIR:multi>/asset)

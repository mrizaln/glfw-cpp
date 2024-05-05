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
find_package(glm REQUIRED)
find_package(glfw3 3.4 REQUIRED)

add_subdirectory(lib/glfw-cpp)

add_executable(single source/single.cpp)
target_include_directories(single PRIVATE source)
target_compile_options(single PRIVATE -Wall -Wextra -Wconversion)
target_link_libraries(single PRIVATE fmt::fmt glad::glad glfw-cpp)

add_executable(multi source/multi.cpp)
target_include_directories(multi PRIVATE source)
target_compile_options(multi PRIVATE -Wall -Wextra -Wconversion)
target_link_libraries(multi PRIVATE fmt::fmt glbinding::glbinding glm::glm
                                    glfw-cpp)

add_executable(multi_multi_manager source/multi_multi_manager.cpp)
target_include_directories(multi_multi_manager PRIVATE source)
target_compile_options(multi_multi_manager PRIVATE -Wall -Wextra -Wconversion)
target_link_libraries(multi_multi_manager PRIVATE fmt::fmt glad::glad glfw-cpp)

add_executable(monitor source/monitor.cpp)
target_include_directories(monitor PRIVATE source)
target_compile_options(monitor PRIVATE -Wall -Wextra -Wconversion)
target_link_libraries(monitor PRIVATE fmt::fmt glfw-cpp)

function(do_sanitize_on TARGET)
  target_compile_options(${TARGET} PRIVATE -fsanitize=thread)
  target_link_options(${TARGET} PRIVATE -fsanitize=thread)
endfunction()

do_sanitize_on(single)
do_sanitize_on(multi)
do_sanitize_on(multi_multi_manager)
do_sanitize_on(monitor)

# copy assets to build directory
add_custom_command(
  TARGET multi
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/asset
          $<TARGET_FILE_DIR:multi>/asset)

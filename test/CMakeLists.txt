cmake_minimum_required(VERSION 3.16)
project(glfw-cpp-test VERSION 0.0.0)

include(cmake/prelude.cmake)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(fmt REQUIRED)
find_package(ut REQUIRED)

add_subdirectory(lib/glfw-cpp)

function(make_test NAME)
    add_executable(${NAME} source/${NAME}.cpp)
    target_link_libraries(${NAME} PRIVATE glfw-cpp Boost::ut fmt::fmt)
    target_compile_options(${NAME} PRIVATE -Wall -Wextra -Wconversion)

    target_compile_options(${NAME} PRIVATE -fsanitize=address,leak,undefined)
    target_link_options(${NAME} PRIVATE -fsanitize=address,leak,undefined)

    add_test(NAME ${NAME} COMMAND $<TARGET_FILE:${NAME}>)
    add_custom_command(TARGET ${NAME} POST_BUILD COMMAND $<TARGET_FILE:${NAME}>)
endfunction()

enable_testing()

make_test(input_test)

project(getopt LANGUAGES C)

add_library(getopt STATIC getopt.c)
target_include_directories(getopt PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

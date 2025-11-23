add_executable(emscripten main.cpp)
target_link_libraries(emscripten PRIVATE glfw-cpp)
target_compile_options(emscripten PRIVATE -Wall -Wextra -Wconversion)
set_target_properties(emscripten PROPERTIES SUFFIX ".html")
set_target_properties(emscripten PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

target_link_options(emscripten PRIVATE --shell-file "${CMAKE_CURRENT_SOURCE_DIR}/shell.html")
target_link_options(emscripten PRIVATE -sRUNTIME_DEBUG -sNO_DISABLE_EXCEPTION_CATCHING)

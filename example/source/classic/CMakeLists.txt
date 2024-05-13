function(create_an_executable NAME)
  add_executable(${NAME} ${NAME}.cpp)
  target_compile_options(${NAME} PRIVATE -Wall -Wextra -Wconversion)
  target_link_libraries(
    ${NAME} PRIVATE glad::glad linmath.h::linmath.h glfw-cpp stb::stb
                    tinycthread::tinycthread getopt)
  set_target_properties(${NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY
                                           ${CMAKE_BINARY_DIR})
endfunction()

# examples are modified from glfw's original examples on version 3.4
create_an_executable(boing)
create_an_executable(gears)
create_an_executable(heightmap)
create_an_executable(offscreen)
create_an_executable(particles)
create_an_executable(sharing)
create_an_executable(splitview)
create_an_executable(triangle-opengl)
create_an_executable(triangle-opengles)
create_an_executable(wave)
create_an_executable(windows)

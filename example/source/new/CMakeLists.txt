function(create_an_executable NAME)
  cmake_parse_arguments(ARGS "DO_SANITIZE" "" "LIBS" ${ARGN})

  add_executable(${NAME} ${NAME}.cpp)
  target_include_directories(${NAME} PRIVATE source/new)
  target_compile_options(${NAME} PRIVATE -Wall -Wextra -Wconversion)
  target_link_libraries(${NAME} PRIVATE ${ARGS_LIBS})
  set_target_properties(${NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

  if(ARGS_DO_SANITIZE)
    target_compile_options(${NAME} PRIVATE -fsanitize=thread)
    target_link_options(${NAME} PRIVATE -fsanitize=thread)
  endif()
endfunction()

# cmake-format: off
create_an_executable(single
  DO_SANITIZE
  LIBS fmt::fmt glad::glad glfw-cpp khrplatform::khrplatform)

create_an_executable(multi
  DO_SANITIZE
  LIBS fmt::fmt glbinding::glbinding glm::glm glfw-cpp khrplatform::khrplatform)

create_an_executable(multi_single_thread
  DO_SANITIZE
  LIBS fmt::fmt glbinding::glbinding glfw-cpp khrplatform::khrplatform)

create_an_executable(multi_multi_manager
  DO_SANITIZE
  LIBS fmt::fmt glad::glad glfw-cpp khrplatform::khrplatform)

create_an_executable(monitor
  DO_SANITIZE
  LIBS fmt::fmt glfw-cpp)

create_an_executable(vulkan
  # DO_SANITIZE
  LIBS fmt::fmt vulkan-headers::vulkan-headers Vulkan::Loader glfw-cpp)

create_an_executable(imgui
  DO_SANITIZE
  LIBS
    fmt::fmt glbinding::glbinding glm::glm glfw-cpp khrplatform::khrplatform
    imgui::imgui imgui::bindings::glfw imgui::bindings::opengl3)
# cmake-format: on

# copy assets to build directory
add_custom_command(
  TARGET multi
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/asset
          $<TARGET_FILE_DIR:multi>/asset)

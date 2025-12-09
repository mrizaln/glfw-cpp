function(create_an_executable NAME)
  cmake_parse_arguments(ARGS "DO_SANITIZE" "" "LIBS" ${ARGN})

  add_executable(${NAME} ${NAME}.cpp)
  target_include_directories(${NAME} PRIVATE source/new)
  target_compile_options(${NAME} PRIVATE -Wall -Wextra -Wconversion)
  target_link_libraries(${NAME} PRIVATE ${ARGS_LIBS})
  set_target_properties(
    ${NAME}
    PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
  )

  if(ARGS_DO_SANITIZE)
    target_compile_options(${NAME} PRIVATE -fsanitize=thread)
    target_link_options(${NAME} PRIVATE -fsanitize=thread)
  endif()
endfunction()

create_an_executable(single LIBS glbinding glfw-cpp)
create_an_executable(multi_single_thread LIBS glbinding glfw-cpp)
create_an_executable(multi_multi_thread LIBS glbinding glfw-cpp)
create_an_executable(monitor LIBS glfw-cpp)
create_an_executable(imgui LIBS glbinding glm glfw-cpp dear_imgui)
create_an_executable(vulkan LIBS Vulkan::Headers Vulkan::Loader glfw-cpp)

# copy assets to build directory
add_custom_command(
  TARGET vulkan
  POST_BUILD
  COMMAND
    ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/../../asset
    $<TARGET_FILE_DIR:vulkan>/asset
)

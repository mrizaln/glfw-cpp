function(make_test NAME)
    add_executable(${NAME} source/${NAME}.cpp)
    target_link_libraries(${NAME} PRIVATE glfw-cpp Boost::ut)
    target_compile_options(${NAME} PRIVATE -Wall -Wextra -Wconversion)

    target_compile_options(${NAME} PRIVATE -fsanitize=address,leak,undefined)
    target_link_options(${NAME} PRIVATE -fsanitize=address,leak,undefined)

    add_test(NAME ${NAME} COMMAND $<TARGET_FILE:${NAME}>)
    add_custom_command(TARGET ${NAME} POST_BUILD COMMAND $<TARGET_FILE:${NAME}>)
endfunction()

enable_testing()

include(cmake/fetched-libs.cmake)

make_test(input_test)

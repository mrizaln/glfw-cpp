#ifndef PRINT_WERS7D5VRFAWEI
#define PRINT_WERS7D5VRFAWEI

#include <cstdio>
#include <format>

template <typename... Args>
void print(std::FILE* stream, std::format_string<Args...> fmt, Args&&... args)
{
    auto string = std::format(fmt, std::forward<Args>(args)...);
    std::fprintf(stream, "%s", string.c_str());
}

template <typename... Args>
void println(std::FILE* stream, std::format_string<Args...> fmt, Args&&... args)
{
    print(stream, fmt, std::forward<Args>(args)...);
    std::putc('\n', stream);
}

template <typename... Args>
void print(std::format_string<Args...> fmt, Args&&... args)
{
    print(stdout, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
void println(std::format_string<Args...> fmt, Args&&... args)
{
    println(stdout, fmt, std::forward<Args>(args)...);
}

#endif /* PRINT_WERS7D5VRFAWEI */

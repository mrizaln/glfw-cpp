# glfw-cpp

A C++ wrapper for GLFW with RAII support and with multi-window and multithreading in mind

## TODO

- [ ] Add event queue mechanism in addition to callback on input handling per window ([see](https://github.com/glfw/gleq))
- [ ] Use better error types instead of just using `std::runtime_error`

## Dependencies

- C++20
- GLFW 3.4

You can use your system package manager or c++ package manager like conan (see [example](./example)) or even manually clone the [GLFW](https://github.com/glfw/glfw) repository and call `add_subdirectory` to add the GLFW dependency.

## Usage

### Setting up

You can clone this repository (or add as submodule) inside your project. I recommend using FetchContent though as it is easier to do.

```cmake
# If you are using FetchContent
include(FetchContent)
FetchContent_Declare(
  glfw-cpp
  GIT_REPOSITORY https://github.com/mrizaln/glfw-cpp
  GIT_TAG main)
FetchContent_MakeAvailable(glfw-cpp)

# # If you clone/submodule the repository
# add_subdirectory(path/to/the/cloned/repository)

add_executable(main main.cpp)
target_link_libraries(main PRIVATE glfw-cpp)  # you don't need to link to glfw here, glfw-cpp already link to it
```

### Example

Using this library is as simple as

```cpp
#include <glad/glad.h>
#include <glfw_cpp/glfw_cpp.hpp>

#include <cmath>

namespace glfw = glfw_cpp;

int main()
{
    glfw::Context context{
        glfw::Context::Hint{
            .m_major   = 3,
            .m_minor   = 3,
            .m_profile = glfw::Context::Profile::CORE,
        },
        [](auto handle [[maybe_unused]], auto proc) {
            if (gladLoadGLLoader((GLADloadproc)proc) == 0) {
                throw std::runtime_error{ "Failed to initialize glad" };
            };
        },
    };

    glfw::WindowManager windowManager{ context };

    auto window = windowManager.createWindow("Learn glfw-cpp", 800, 600);    // NOLINT
    window.addKeyEventHandler(
        GLFW_KEY_ESCAPE, 0, glfw::Window::KeyActionType::CALLBACK, [](auto& w) { w.requestClose(); }
    );

    window.run([&, elapsed = 0.0F]() mutable {
        elapsed += (float)window.deltaTime();

        // funny color cycle
        const auto r = (std::sin(23.0F / 8.0F * elapsed) + 1.0F) * 0.1F + 0.4F;
        const auto g = (std::cos(13.0F / 8.0F * elapsed) + 1.0F) * 0.2F + 0.3F;
        const auto b = (std::sin(41.0F / 8.0F * elapsed) + 1.5F) * 0.2F;

        glClearColor(r, g, b, 1.0F);    // NOLINT
        glClear(GL_COLOR_BUFFER_BIT);

        const auto& prop = window.properties();
        glViewport(0, 0, prop.m_width, prop.m_height);

        windowManager.pollEvents();
    });
}
```

No manual cleanup necessary, the classes defined already using RAII pattern.

One caveat is that you need to make sure that `glfw_cpp::Context` outlive `glfw_cpp::WindowManager` and `glfw_cpp::WindowManager` outlive `glfw_cpp::Window`s in order for the program to be well defined and not crashing.

The above example is a single-threaded, one window example. For a multi-window and multithreaded example, you can see it in the [example](./example) directory (I also use a different OpenGL loader library there).

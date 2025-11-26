# glfw-cpp

A C++ wrapper for GLFW with RAII support and with multi-window and multithreading in mind.

This wrapper is customized for my needs and isn't a direct one-to-one port of the GLFW C API to C++. I primarily use it to simplify window management for my graphics projects. You can see my projects here::

- [game-of-life](https://github.com/mrizaln/game-of-life)

## Features

- RAII handles
- Easy multi-threading (one window/context per thread)
- Vulkan support
- Emscripten support
- No callback

## Dependencies

- C++20
- GLFW 3.3+

You can use your system package manager or c++ package manager like Conan (see [example](./example)) or even manually clone the [GLFW](https://github.com/glfw/glfw) repository and call `add_subdirectory` to add the GLFW dependency.

## Usage

### Setting up

You can clone this repository (or add as submodule) inside your project. I recommend using FetchContent though as it is easier to do.

```cmake
# If you are using FetchContent
# -----------------------------
include(FetchContent)
FetchContent_Declare(
  glfw-cpp
  GIT_REPOSITORY https://github.com/mrizaln/glfw-cpp
  GIT_TAG v0.11.0)

option(GLFW_CPP_VULKAN_SUPPORT "vulkan support" ON)        # enable Vulkan support (requires Vulkan loader and headers)
# option(GLFW_CPP_EMSCRIPTEN_PORT "emscripten port" ON)    # enable GLFW port for emscripten (mutually exclusive with above)

FetchContent_MakeAvailable(glfw-cpp)

# # If you clone/submodule the repository instead do this
# add_subdirectory(path/to/the/cloned/repository)

add_executable(main main.cpp)
target_link_libraries(main PRIVATE glfw-cpp ...)  # you don't need to link to glfw here, glfw-cpp already link to it
```

### Option

This library supports Vulkan, just set `GLFW_CPP_VULKAN_SUPPORT` before adding this repository to the project to enable it. Note that it requires Vulkan loader and the headers to compile.

This library also supports web/wasm platform. The support is possible thanks to the [GLFW port for emscripten](https://github.com/pongasoft/emscripten-glfw) made by [pongasoft](https://github.com/pongasoft). To enable the port set the `GLFW_CPP_EMSCRIPTEN_PORT` to `ON` before adding this repository to your project.

### Example

> [multi_multi_thread.cpp](./example/source/new/multi_multi_thread.cpp)

```cpp
#include <glbinding/gl/gl.h>
#include <glbinding/glbinding.h>
#include <glfw_cpp/glfw_cpp.hpp>

#include <cmath>
#include <cstdlib>
#include <ctime>
#include <thread>

using namespace gl;    // from <glbinding/gl/g.h>

void window_thread(glfw_cpp::Window&& window)
{
    glfw_cpp::make_current(window.handle());
    glbinding::initialize(glfw_cpp::get_proc_address);

    auto multiplier = 1.0f / static_cast<float>(std::rand() % 10 + 1);
    auto elapsed    = 0.0f;

    while (not window.should_close()) {
        // swap events enqueued by glfw_cpp::poll_events()
        window.swap_events().visit(glfw_cpp::event::Overload{
            [&](const glfw_cpp::event::KeyPressed& e) {
                if (e.key == glfw_cpp::KeyCode::Q and e.state == glfw_cpp::KeyState::Press) {
                    window.request_close();
                }
            },
            [&](const glfw_cpp::event::FramebufferResized& e) { glViewport(0, 0, e.width, e.height); },
            [&](const auto&) { /* do nothing */ },
        });

        elapsed += static_cast<float>(window.delta_time());

        const auto r = (std::sin(multiplier * 23.0f / 8.0f * elapsed) + 1.0f) * 0.1f + 0.4f;
        const auto g = (std::cos(multiplier * 13.0f / 8.0f * elapsed) + 1.0f) * 0.2f + 0.3f;
        const auto b = (std::sin(multiplier * 41.0f / 8.0f * elapsed) + 1.5f) * 0.2f;

        glClearColor(r, g, b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        window.swap_buffers();
    };
}

int main()
{
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    auto glfw = glfw_cpp::init({});

    glfw->apply_hint({
        .api = glfw_cpp::api::OpenGL{
            .version_major = 3,
            .version_minor = 3,
            .profile       = glfw_cpp::gl::Profile::Core,
        },
    });

    auto window1 = glfw->create_window(800, 600, "Hello glfw-cpp 1");
    auto window2 = glfw->create_window(800, 600, "Hello glfw-cpp 2");
    auto window3 = glfw->create_window(800, 600, "Hello glfw-cpp 3");
    auto window4 = glfw->create_window(800, 600, "Hello glfw-cpp 4");

    auto thread1 = std::jthread{ window_thread, std::move(window1) };
    auto thread2 = std::jthread{ window_thread, std::move(window2) };
    auto thread3 = std::jthread{ window_thread, std::move(window3) };
    auto thread4 = std::jthread{ window_thread, std::move(window4) };

    while (glfw->has_window_opened()) {
        using glfw_cpp::operator""_fps;
        glfw->poll_events(120_fps);   // automatically queue events to its windows
    }
}
```

No manual cleanup necessary!

The above example is multi threaded with one context and one window per thread. For emscripten example, see [this one](./example/source/emscripten/main.cpp). For other examples, head to [example](./example) directory.

## Documentation

The project is documented using Doxygen. There is a Doxygen configuration in [docs](./docs/Doxygen) that can be used to generate a HTML documentation page.

From the root of the project, just run this command (require `doxygen` binary to be installed). The output of the HTML page is in `docs/doxygen/html`

```sh
doxygen docs/Doxygen
```

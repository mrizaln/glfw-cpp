# glfw-cpp

A C++ wrapper for GLFW with RAII support and with multi-window and multithreading in mind.

This wrapper is customized for my needs and isn't a direct one-to-one port of the GLFW C API to C++. I primarily use it to simplify window management for my graphics projects. You can see my projects here::

- [game-of-life](https://github.com/mrizaln/game-of-life)

## Features

- RAII handles
- Easy multi-window
- Vulkan support
- Emscripten support

## Dependencies

- C++20
- GLFW 3.3+

You can use your system package manager or c++ package manager like Conan (see [example](./example)) or even manually clone the [GLFW](https://github.com/glfw/glfw) repository and call `add_subdirectory` to add the GLFW dependency.

## Usage

### Setting up

You can clone this repository (or add as submodule) inside your project. I recommend using FetchContent though as it is easier to do.

```cmake
# If you are using FetchContent
include(FetchContent)
FetchContent_Declare(
  glfw-cpp
  GIT_REPOSITORY https://github.com/mrizaln/glfw-cpp
  GIT_TAG v0.11.0)

option(GLFW_CPP_VULKAN_SUPPORT "vulkan support" ON)       # enable Vulkan support (requires Vulkan loader and headers)
# option(GLFW_CPP_EMSCRIPTEN_PORT "emscripten port" ON)    # enable GLFW port for emscripten (mutually exclusive with above)
FetchContent_MakeAvailable(glfw-cpp)

# # If you clone/submodule the repository
# add_subdirectory(path/to/the/cloned/repository)

add_executable(main main.cpp)
target_link_libraries(main PRIVATE glfw-cpp)  # you don't need to link to glfw here, glfw-cpp already link to it
```

### Option

This library supports Vulkan, just set `GLFW_CPP_VULKAN_SUPPORT` before adding this repository to the project to enable it. Note that it requires Vulkan loader and the headers to compile.

This library also supports web/wasm platform. The support is possible thanks to the [GLFW port for emscripten](https://github.com/pongasoft/emscripten-glfw) made by [pongasoft](https://github.com/pongasoft). To enable the port set the `GLFW_CPP_EMSCRIPTEN_PORT` to `ON` before adding this repository to your project.

### Example

Using this library is as simple as

> [single.cpp](./example/source/new/single.cpp)

```cpp
#include <glad/glad.h>
#include <glfw_cpp/glfw_cpp.hpp>

#include <cmath>
#include <iostream>

int main()
{
    namespace api = glfw_cpp::api;

    // `init()` calls `glfwInit()` internally and returns an `Instance::Unique` that will call `glfwTerminate()`
    // on destruction. Note that the graphics API can't be changed later, this is a design choice.
    auto instance = glfw_cpp::init(api::OpenGL{
        .major   = 3,
        .minor   = 3,
        .profile = api::gl::Profile::Core,
        .loader  = [](api::gl::Context, api::gl::GetProc proc) { gladLoadGLLoader((GLADloadproc)proc); },
    });

    // `WindowManager` is responsible for managing windows (think of window group). The only way to construct
    // `Window` is through this `glfw_cpp::Instance::create_window_manager()` function which returns a
    // `std::shared_ptr<WindowManager>`. Each window created with this instance claims ownership of it.
    auto wm = instance->create_window_manager();

    // Graphics API hints are omitted from the `WindowHint` since it's already set at initialization. Only
    // other relevant hints are included.
    using F   = glfw_cpp::Flag;
    auto hint = glfw_cpp::Hint{ .flags = F::Default ^ F::Resizable };    // use default hint but not resizable

    auto window = wm->create_window(hint, "Learn glfw-cpp", 800, 600);

    // This member function is a helper for making an infinite loop until close is requested
    window.run([&, elapsed = 0.0F](const glfw_cpp::EventQueue& events) mutable {
        using K      = glfw_cpp::KeyCode;
        namespace ev = glfw_cpp::event;

        // Events are represented as std::variant-like structure which you can visit using the `visit()`
        // member function. glfw_cpp also provides a helper for struct for overload pattern like below.
        {
            events.visit(ev::Overload{
                [&](const ev::KeyPressed&         e) { if (e.key == K::Q) window.request_close();           },
                [&](const ev::FramebufferResized& e) { glViewport(0, 0, e.width, e.height);                 },
                [&](const auto&                   e) { std::cout << "event happened " << (void*)&e << '\n'; },  // catch-all case
            });
        }

        // `glfw_cpp::Window` keeps a copy of (almost) every properties of the window (like pressed keys) in
        // itself. You can query it directly.
        {
            const auto& keys = window.properties().key_state;

            if (keys.all_pressed({ K::H, K::J, K::L, K::K })) {
                std::cout << "HJKL key pressed all at once\n";
            }

            if (keys.is_pressed(K::LeftShift) && keys.any_pressed({ K::W, K::A, K::S, K::D })) {
                std::cout << "WASD key pressed with shift key being held\n";
            } else if (keys.any_pressed({ K::W, K::A, K::S, K::D })) {
                std::cout << "WASD key pressed\n";
            }
        }

        // Delta time is time between calls to `Window::display` which is after current lambda in the case of `Window::run`
        elapsed += static_cast<float>(window.delta_time());

        // OpenGL stuff
        {
            const float r = (std::sin(23.0F / 8.0F * elapsed) + 1.0F) * 0.1F + 0.4F;
            const float g = (std::cos(13.0F / 8.0F * elapsed) + 1.0F) * 0.2F + 0.3F;
            const float b = (std::sin(41.0F / 8.0F * elapsed) + 1.5F) * 0.2F;

            glClearColor(r, g, b, 1.0F);
            glClear(GL_COLOR_BUFFER_BIT);
        }

        // Poll events from the OS. This must be done from main thread.
        wm->poll_events();
    });
}
```

> Make sure that `glfw_cpp::Instance` outlive `glfw_cpp::WindowManager` and `glfw_cpp::Window`s in order for the program to be well defined and not crashing.

No manual cleanup necessary!

The above example is a single-threaded, one window example. For a multi-window and multithreaded example, you can see [here](./example/source/new/multi.cpp) or [here](./example/source/new/multi_multi_manager.cpp) (I also use a different OpenGL loader library there). For emscripten example, see [this one](./example/source/emscripten/main.cpp).

## Documentation

The project is documented using Doxygen. There is a Doxygen configuration in [docs](./docs/Doxygen) that can be used to generate a HTML documentation page.

From the root of the project, just run this command (require `doxygen` binary to be installed). The output of the HTML page is in `docs/doxygen/html`

```sh
cd docs
doxygen docs/Doxygen
```

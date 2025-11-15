# glfw-cpp

A C++ wrapper for GLFW with RAII support and with multi-window and multithreading in mind.

This wrapper is personalized and written to suit my needs. This is not a wrapper that is a one-to-one "port" of glfw C-API to C++ API. This repository will be updated iteratively as needed. I mainly use this repository to facilitate my graphics programming project that requires painless window management. You can see my projects here:

- [game-of-life](https://github.com/mrizaln/game-of-life)

## Dependencies

- C++20
- GLFW 3.3

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
  GIT_TAG v0.10.0)

# set this variable to ON to enable Vulkan support (requires Vulkan loader and headers)
option(GLFW_CPP_VULKAN_SUPPORT "vulkan support" ON)
FetchContent_MakeAvailable(glfw-cpp)

# # If you clone/submodule the repository
# add_subdirectory(path/to/the/cloned/repository)

add_executable(main main.cpp)
target_link_libraries(main PRIVATE glfw-cpp)  # you don't need to link to glfw here, glfw-cpp already link to it
```

### Option

This library supports Vulkan, just set `GLFW_CPP_VULKAN_SUPPORT` before adding this repository to the project to enable it. Note that it requires Vulkan loader and the headers to compile.

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

    // `glfw_cpp::init()` calls `glfwInit()` internally and returns an `glfw_cpp::Instance::Unique` that will
    // call `glfwTerminate()` on destruction. Note that the graphics API can't be changed later, this is a
    // design choice.
    auto instance = glfw_cpp::init(api::OpenGL{
        .major   = 3,
        .minor   = 3,
        .profile = api::gl::Profile::Core,
        .loader  = [](api::gl::Context, api::gl::GetProc proc) { gladLoadGLLoader((GLADloadproc)proc); },
    });

    // `WindowManager` is responsible for managing windows (think of window group). The only way to construct
    // `Window` is through this `glfw_cpp::Instance::create_window_manager()` function which returns a
    // `std::shared_ptr<WindowManager>`. Each window created with this instance claims ownership of it (hence
    // the shared_ptr).
    auto wm = instance->create_window_manager();

    // graphics API hints are omitted from the `WindowHint` since it's already set at initialization. Only
    // other relevant hints are included.
    using F   = glfw_cpp::Flag;
    auto hint = glfw_cpp::Hint{ .flags = F::Default ^ F::Resizable };    // use default hint but not resizable

    auto window = wm->create_window(hint, "Learn glfw-cpp", 800, 600);

    window.run([&, elapsed = 0.0F](const glfw_cpp::EventQueue& events) mutable {
        using K      = glfw_cpp::KeyCode;
        namespace ev = glfw_cpp::event;

        // handling events
        {
            // clang-format off
            events.visit(ev::Overload{
                [&](const ev::KeyPressed&         e) { if (e.key == K::Q) window.request_close();           },
                [&](const ev::FramebufferResized& e) { glViewport(0, 0, e.width, e.height);                 },
                [&](const auto&                   e) { std::cout << "event happened " << (void*)&e << '\n'; },  // catch-all case
            });
            // clang-format on
        }

        // `glfw_cpp::Window` keep a copy of (almost) every properties of the window (like pressed keys) in
        // itself. You can query it for continuous key input (for movement) for example.
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

        elapsed += static_cast<float>(window.delta_time());

        // funny color cycle
        const float r = (std::sin(23.0F / 8.0F * elapsed) + 1.0F) * 0.1F + 0.4F;
        const float g = (std::cos(13.0F / 8.0F * elapsed) + 1.0F) * 0.2F + 0.3F;
        const float b = (std::sin(41.0F / 8.0F * elapsed) + 1.5F) * 0.2F;

        glClearColor(r, g, b, 1.0F);
        glClear(GL_COLOR_BUFFER_BIT);

        // poll events from the OS
        wm->poll_events();
    });
}
```

No manual cleanup necessary, the classes defined already using RAII pattern.

One thing to keep in mind is that you need to make sure that `glfw_cpp::Instance::Unique` outlive `glfw_cpp::WindowManager` and `glfw_cpp::Window`s in order for the program to be well defined and not crashing.

The above example is a single-threaded, one window example. For a multi-window and multithreaded example, you can see [here](./example/source/new/multi.cpp) or [here](./example/source/new/multi_multi_manager.cpp) (I also use a different OpenGL loader library there).

## Documentation

The project is documented using Doxygen. There is a Doxygen configuration in [docs](./docs/Doxygen) that can be used to generate a HTML documentation page.

From the root of the project, just run this command (require `doxygen` binary to be installed). The output of the HTML page is in `docs/doxygen/html`

```sh
cd docs
doxygen docs/Doxygen
```

## TODO

- [x] Add event queue mechanism ~~in addition to callback~~ on input handling per window ([see](https://github.com/glfw/gleq))
- [x] ~~Handle GLFW internal error~~
  > - GLFW internal errors are mainly occurs from invalid enumeration passed into a function, invalid value for an enum passed into a function, and failure in maintaining invariants (like GLFW is initialized or not).
  > - This library is a C++ library so errors from passing invalid enumeration and/or invalid value for an enum is avoided by using the stricter type-system of C++.
  > - This library also is trying it's best in maintaining the invariants so hopefully the errors that surface from these are also avoided.
  > - That leaves out platform-related errors which I can report easily as exceptions.
- [x] Add the ability to handle window events in separate thread from Window and WindowManager
  > While the handling itself still in the thread `WindowManager` is, with the introduction of `IEventInterceptor`, user can intercept events polled by `WindowManager`before getting pushed into`EventQueue`inside`WindowManager`(you can control whether to continue forwarding the event to the underlying `Window` or not). The intercepted `Event` then can be sent to other thread for example.
- [x] Eliminate `glfw_cpp::WindowManager` move limitation
  > By making `WindowManager` be exclusively `shared_ptr`, this issue is completely eliminated.
- [x] Add documentation.
- [x] Add LICENSE

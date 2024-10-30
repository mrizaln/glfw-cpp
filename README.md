# glfw-cpp

A C++ wrapper for GLFW with RAII support and with multi-window and multithreading in mind.

This wrapper is personalized and written to suit my needs. This is not a wrapper that is a one-to-one "port" of glfw C-API to C++ API. This repository will be updated iteratively as needed. I mainly use this repository to facilitate my graphics programming project that requires painless window management. You can see my projects here:

- [game-of-life](https://github.com/mrizaln/game-of-life)

## Dependencies

- C++20
- GLFW 3.3

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

#include <iostream>
#include <cmath>

int main()
{
    // Graphics API can't be changed. You can recreate the glfw instance (basically resetting glfw)
    // to use other graphics API. This is a design choice.
    auto api = glfw_cpp::Api::OpenGL{
        .m_major   = 3,
        .m_minor   = 3,
        .m_profile = glfw_cpp::Api::OpenGL::Profile::CORE,
        .m_loader  = [](glfw_cpp::Api::GlContext /* context */, glfw_cpp::Api::GlGetProc proc) {
            // glad does not use the context, but other loader might be
            gladLoadGLLoader((GLADloadproc)proc);
        },
    };

    // All types are RAII, there's no need to manage them manually
    auto instance = glfw_cpp::init(api);
    auto wm       = instance->createWindowManager();
    auto window   = wm->createWindow({}, "Learn glfw-cpp", 800, 600);

    // window loop (v-sync enabled by default and polls events from WindowManager automatically)
    window.run([&, elapsed = 0.0F](const glfw_cpp::EventQueue& events) mutable {
        // events
        for (const auto& event : events) {
            using E = glfw_cpp::Event;
            using K = glfw_cpp::KeyCode;

            // using visitor pattern and overload set to visit each event
            event.visit(E::Overloaded{
                [&](const E::KeyPressed&         e) { if (e.m_key == K::Q) window.requestClose(); },
                [&](const E::FramebufferResized& e) { glViewport(0, 0, e.m_width, e.m_height);    },
                [&](const auto&) {},  // catch-all case
            });
        }

        // continuous key input (for movement for example)
        {
            using K          = glfw_cpp::KeyCode;
            const auto& keys = window.properties().m_keyState;

            if (keys.allPressed({ K::H, K::J, K::L, K::K })) {
                std::cout << "HJKL key pressed all at once\n";
            }

            if (keys.isPressed(K::LEFT_SHIFT) && keys.anyPressed({ K::W, K::A, K::S, K::D })) {
                std::cout << "WASD key pressed with shift key being held\n";
            } else if (keys.anyPressed({ K::W, K::A, K::S, K::D })) {
                std::cout << "WASD key pressed\n";
            }
        }

        elapsed += static_cast<float>(window.deltaTime());

        // funny color cycle
        const auto r = (std::sin(23.0F / 8.0F * elapsed) + 1.0F) * 0.1F + 0.4F;
        const auto g = (std::cos(13.0F / 8.0F * elapsed) + 1.0F) * 0.2F + 0.3F;
        const auto b = (std::sin(41.0F / 8.0F * elapsed) + 1.5F) * 0.2F;

        glClearColor(r, g, b, 1.0F);
        glClear(GL_COLOR_BUFFER_BIT);

        // WindowManager still need to poll the events from the OS though
        wm->pollEvents();
    });
}
```

No manual cleanup necessary, the classes defined already using RAII pattern.

One thing to keep in mind is that you need to make sure that `glfw_cpp::Instance::Handle` outlive `glfw_cpp::WindowManager` and `glfw_cpp::Window`s in order for the program to be well defined and not crashing.

The above example is a single-threaded, one window example. For a multi-window and multithreaded example, you can see [here](./example/source/new/multi.cpp) or [here](./example/source/new/multi_multi_manager.cpp) (I also use a different OpenGL loader library there).

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

# glfw-cpp

A C++ wrapper for GLFW with RAII support and with multi-window and multithreading in mind.

This wrapper is personalized and written to suit my needs. This is not a wrapper that is a one-to-one "port" of glfw C-API to C++ API. This repository will be updated iteratively as needed. I mainly use this repository to facilitate my graphics programming project that requires painless window management. You can see my projects here:

- [game-of-life](https://github.com/mrizaln/game-of-life)

## TODO

- [x] Add event queue mechanism ~~in addition to callback~~ on input handling per window ([see](https://github.com/glfw/gleq))
- [ ] Handle GLFW internal error
- [ ] Add the ability to handle window events in separate thread from Window and WindowManager
- [ ] Eliminate `glfw_cpp::WindowManager` move limitation

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

> [single.cpp](./example/source/single.cpp)

```cpp
#include <glad/glad.h>
#include <glfw_cpp/glfw_cpp.hpp>

#include <iostream>
#include <cmath>

namespace glfw = glfw_cpp;

int main()
{
    // init() calls glfwInit() internally and Instance::Handle will call glfwTerminate() on dtor.
    // Note that the graphics API can't be changed later, this is a design choice.
    glfw::Instance::Handle instance = glfw::init(glfw::Api::OpenGL{
        .m_major   = 3,
        .m_minor   = 3,
        .m_profile = glfw::Api::OpenGL::Profile::CORE,
        .m_loader  = [](glfw::Api::GlContext /* handle */,
                       glfw::Api::GlGetProc proc) { gladLoadGLLoader((GLADloadproc)proc); },
    });

    // WindowManager is responsible for managing windows (think of window group)
    glfw::WindowManager wm = instance->createWindowManager();

    // graphics API hints are omitted from the WindowHint, only other relevant hints are included.
    glfw::WindowHint hint = {};    // use default hint

    glfw::Window window = wm.createWindow(hint, "Learn glfw-cpp", 800, 600);

    window.run([&, elapsed = 0.0F](std::vector<glfw::Event>&& events) mutable {
        // events
        for (const glfw::Event& event : events) {
            if (auto* e = event.getIf<glfw::Event::KeyPressed>()) {
                if (e->m_key == glfw::KeyCode::Q) {
                    window.requestClose();
                }
            } else if (auto* e = event.getIf<glfw::Event::FramebufferResized>()) {
                glViewport(0, 0, e->m_width, e->m_height);
            }
        }

        // continuous key input (for movement for example)
        {
            using K          = glfw::KeyCode;
            const auto& keys = window.properties().m_keyState;

            if (keys.allPressed({ K::H, K::J, K::L, K::K })) {
                std::cout << "HJKL key pressed all at once";
            }

            if (keys.isPressed(K::LEFT_SHIFT) && keys.anyPressed({ K::W, K::A, K::S, K::D })) {
                std::cout << "WASD key pressed with shift key being held";
            } else if (keys.anyPressed({ K::W, K::A, K::S, K::D })) {
                std::cout << "WASD key pressed";
            }
        }

        elapsed += static_cast<float>(window.deltaTime());

        // funny color cycle
        const float r = (std::sin(23.0F / 8.0F * elapsed) + 1.0F) * 0.1F + 0.4F;
        const float g = (std::cos(13.0F / 8.0F * elapsed) + 1.0F) * 0.2F + 0.3F;
        const float b = (std::sin(41.0F / 8.0F * elapsed) + 1.5F) * 0.2F;

        glClearColor(r, g, b, 1.0F);
        glClear(GL_COLOR_BUFFER_BIT);

        wm.pollEvents();
    });
}
```

No manual cleanup necessary, the classes defined already using RAII pattern.

One thing to keep in mind is that you need to make sure that `glfw_cpp::Instance::Handle` outlive `glfw_cpp::WindowManager` and `glfw_cpp::WindowManager` outlive `glfw_cpp::Window`s in order for the program to be well defined and not crashing.

The above example is a single-threaded, one window example. For a multi-window and multithreaded example, you can see [here](./example/source/multi.cpp) or [here](./example/source/multi_multi_manager.cpp) directory (I also use a different OpenGL loader library there).

## Limitation

Other limitation is that `glfw_cpp::WindowManager` should not be moved after it created `glfw_cpp::Window`s since each window created from it has a pointer to the `glfw_cpp::WindowManager`. If you are doing that it may leads to undefined behavior (dereferencing a pointer to a destroyed `glfw_cpp::WindowManager` for example). If the `glfw_cpp::WindowManager` hasn't created any `glfw_cpp::Window`s (or they are already destroyed) it is okay to move it.

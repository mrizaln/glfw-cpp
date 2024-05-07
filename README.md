# glfw-cpp

A C++ wrapper for GLFW with RAII support and with multi-window and multithreading in mind.

This wrapper is personalized and written to suit my needs. This is not a wrapper that is a one-to-one "port" of glfw C-API to C++ API. This repository will be updated iteratively as needed. I mainly use this repository to facilitate my graphics programming project that requires painless window management. You can see my projects here:

- [game-of-life](https://github.com/mrizaln/game-of-life)

## TODO

- [x] Add event queue mechanism ~~in addition to callback~~ on input handling per window ([see](https://github.com/glfw/gleq))
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

#include <cmath>

namespace glfw = glfw_cpp;

int main()
{
    // Instance here refers to global state that glfw_cpp initializes in order to communicate with
    // the internals of GLFW. This class can only have one valid instance and throws when
    // instantiated again. The init function returns a RAII handle that automatically deinit the
    // Instance on destruction.
    glfw::Instance::Handle instance = glfw_cpp::init(glfw::Api::OpenGL{
        .m_major   = 3,
        .m_minor   = 3,
        .m_profile = glfw::Api::OpenGL::Profile::CORE,
        .m_loader =
            [](glfw::Api::GlContext /* handle */, glfw::Api::GlGetProc proc) {
                // why two arguments?
                // Some GL loader libraries need a handle to load the proc.
                // Case on point: glbinding (though it is not required, but reocommended)
                gladLoadGLLoader((GLADloadproc)proc);
            },
    });

    glfw::WindowManager wm     = instance->createWindowManager();
    glfw::WindowHint    hint   = {};    // use default hint
    glfw::Window        window = wm.createWindow(hint, "Learn glfw-cpp", 800, 600);

    window.run([&, elapsed = 0.0F](std::deque<glfw::Event>&& events) mutable {
        for (const glfw::Event& event : events) {
            if (auto* e = event.getIf<glfw::Event::KeyPressed>()) {
                if (e->m_key == glfw::KeyCode::Q) {
                    window.requestClose();
                }
            } else if (auto* e = event.getIf<glfw::Event::FramebufferResized>()) {
                glViewport(0, 0, e->m_width, e->m_height);
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

One caveat is that you need to make sure that `glfw_cpp::Instance::Handle` outlive `glfw_cpp::WindowManager` and `glfw_cpp::WindowManager` outlive `glfw_cpp::Window`s in order for the program to be well defined and not crashing.

The above example is a single-threaded, one window example. For a multi-window and multithreaded example, you can see [here](./example/source/multi.cpp) or [here](./example/source/multi_multi_manager.cpp) directory (I also use a different OpenGL loader library there).

# Building

## New and classic

I use [Conan](https://docs.conan.io/2/installation.html) to install the dependencies for this example. Make sure you have it installed and is configured properly.

- Install the dependencies

  ```sh
  conan install . --build missing -s build_type=Debug
  ```

- Build

  ```sh
  cmake --preset conan-debug                              # may be conan-default on windows
  cmake --build --preset conan-debug
  ```

- Run the executable

  ```sh
  ./build/Debug/<example>
  ```

## Emscripten

Make sure you have the sdk installed and is active for the current shell.

The emscripten example is built separately since it runs very differently from usual C++ code.

No manual dependencies installation is needed for this one as `glfw-cpp` use the `contrib.glfw3` port for emscripten which will be installed automatically.

- Build

  ```sh
  emcmake cmake -S . -B build/Emscripten
  cmake --build build
  ```

- View the result in browser

  ```sh
  firefox ./build/Emscripten/emscripten.html
  ```

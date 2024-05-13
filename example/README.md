# Building

Make sure you have conan installed and properly configured

- Install the dependencies

  ```
  conan install . -of build/debug/ --build missing -s build_type=Debug
  ```

- Configure the project

  ```
  cmake --preset conan-debug
  ```

- Build the project

  ```
  cmake --build --preset conan-debug
  ```

- Run the executable

  ```
  ./build/debug/<example>
  ```

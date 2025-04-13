# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Changed

- Move Event variants into `glfw_cpp::event` namespace.
- Drop `m_*` prefix from public facing fields name.

## [0.10.0] - 2025-03-18

### Added

- New `dev` branch for active development.
- New `ModifierKey::test_any` and `ModifierKey::test_all` member functions.
- Proper unit tests for inputs functionalities.
- New changelog file.
- Error handling for `GLFW_INVALID_VALUE` and `GLFW_INVALID_ENUM` adding `InvalidValue` and `InvalidEnum` class respectively.

### Fixed

- Fix off-by-one error on `pressed_buttons` and `released_button` member functions for both `KeyStateRecord` and `MouseButtonStateRecord`.
- Incorrect logger formatting for internal error.
- Missing `error.hpp` include in `glfw_cpp.hpp` header.

### Changed

- Replace variadics functions to single parameter `std::initializer_list` and `std::span` for `ModifierKey` functions.
- Make ModifierKey default constructible.
- Make ModifierKey constructor explicit.
- Make `ModifierKey::test` function take only single parameter.
- Make `set_value`, `set`, and `unset` member functions of `KeyStateRecord` and `MouseButtonStateRecord` return its instance to allow chaining.
- Make `IEventInterceptor` and its derivatives a class.
- Promote the severity of internal error from Error to Critical.
- Add `ErrorCode` enumeration as an alternative type safe GLFW error code.
- Add `ErrorCode` information inside the `glfw_cpp::Error` class that can be queried with `code()` member function.

## [0.9.0] - 2025-01-26

### Added

- New `Window::toggle_vsync` and `Window::toggle_capture_mouse` member functions.
- Extra functionality for ImGui binding for GLFW.

### Changed

- Make `Window::enqueue_task` takes a `std::function<void(Window&)>` instead of `std::function<void()>`.
- Make `IEventInterceptor` abstract class destructor public virtual.
- Add `IEventInterceptor*` as first argument for `Instance::create_window_manager` function

## [0.8.0] - 2025-01-17

### Changed

- Update code style from mainly `camelCase` and `PascalCase` to `snake_case` and `PascalCase` for variables/functions and types respectively.

## [0.7.1] - 2025-01-06

### Added

- Ability to peek last events through `Window::events` function without polling.
- New fields for `Window` events that represent changes/difference.
- Ability to directly visit all `Events` in `EventQueue` using `EventQueue::visit` member function.

## [0.7.0] - 2024-10-31

### Added

- New `Monitor` entry for `Window`'s `Properties`.
- New `IEventInterceptor` abstract class for intercepting events polled by `WindowManager`.
- New `DefaultEventInterceptor` concrete class derived from `IEventInterceptor` that does nothing in its interception.
- Proper Doxygen documentation.
- Proper GLFW error handling.
- Add LICENSE.

### Fixed

- Make enumerations PascalCase instead of UPPERCASE to fix naming issue on Windows platform.

### Changed

- Update the event handling mechanism to be compatible with the act of intercepting them.
- Update the underlying value for enums and wrapper class related to input.
- Make `WindowManager` created from `Instance::create_window_manager` a `std::shared_ptr`.
- Update README.md.
- Update examples.

## [0.6.0] - 2024-05-28

### Added

- New delta fields for `CursorMoved` event.
- New `EventQueue` as an abstraction to `std::vector<Event>` using circular buffer internally.
- Ability to destroy `Window` instance and reset to uninitialized state using `Window::destroy` function.
- Minimal error handling.

### Fixed

- Data race when exchanging tasks queue.
- Missing initializer for `FramebufferSize` at `Window` creation.
- Example files paths pointing to nonexsistent files.

### Changed

- Update examples.

## [0.5.0] - 2024-05-13

### Added

- New `Attribute` as fields on `Window`'s `Properties`
- New APIs for `Window` manipulation: `iconify`, `restore`, `maximize`, `show`, `hide`, and `focus`.
- Clipboard and time API from the underlying GLFW library.
- Ability to lock aspect ratio just like in GLFW.
- New forward compat field to OpenGL API.
- New `FramebufferSize` field on `Window`'s `Properties`.
- New `Overloaded` helper struct for `std::variant` visitor.

### Fixed

- Missing default initializer for `Window`'s fields.

### Changed

- Make `Window`'s default constructor public.
- Name the unnamed structs on `Window`'s `Properties`.
- Move the `Window`'s callback set calls from `window.cpp` to `window_manager.cpp` just before the `Window` instance created.
- OpenGL API defaults to version 3.3.
- Reorder `Instance::LogLevel` enumerations with lower value represent lower severity.
- Update examples.

### Removed

- `GLFW.h` header include dependency on `input.hpp` .
- `vk::initVulkanLoader` function.

## [0.4.0] - 2024-05-09

### Added

- New APIs for using `Window` manually: `use`, `poll`, `display`, `shouldClose`.
- Vulkan usage example.

### Fixed

- Erroneous implementation of `WindowManager::hasWindowOpened` function.
- Vulkan include issue.

### Changed

- Update README.md.
- Lower the GLFW version requirement.

## [0.3.0] - 2024-05-08

### Added

- New `KeyStateRecord` struct as field on `Window`'s `Properties` for tracking which key is pressed at the last poll.
- New `MouseButtonStateRecord` struct as field on `Window`'s `Properties` for tracking which button is pressed at the last poll.

### Changed

- Update README.md.
- Update examples.

## [0.2.0] - 2024-05-07

### Added

- New `m_pos` and `m_dimension` fields on `Window`'s `Properties`.
- New `Monitor` struct as a wrapper to `GLFWmonitor*`.
- New `m_monitor` and `m_share` fields on `WindowHint`
- New events: `MonitorConnected`, `FileDropped`, `JoystickConnected`, `WindowMaximized`, and `WindowScaleChanged`.

### Fixed

- Missing includes and extra includes.

### Changed

- Rework the event handling mechanism from callback-based to poll-based.
- Update README.md.
- Update examples.
- Update window creation logic to include sharing and monitor data.
- Rename `Context` to `Instance` to avoid confusion with OpenGL context.

## [0.1.0] - 2024-05-02

### Added

- New `Context` class that wraps the entire GLFW instance in a RAII class instead of global state.
- New `Window` class wrapper for `GLFWwindow*`.
- New `WindowManager` class to group multiple `Window`s and poll events from OS.
- New Vulkan support.
- New examples code.
- New README.md.

[unreleased]: https://github.com/mrizaln/glfw-cpp/compare/v0.10.0...HEAD
[0.10.0]: https://github.com/mrizaln/glfw-cpp/compare/v0.9.0...v0.10.0
[0.9.0]: https://github.com/mrizaln/glfw-cpp/compare/v0.8.0...v0.9.0
[0.8.0]: https://github.com/mrizaln/glfw-cpp/compare/v0.7.1...v0.8.0
[0.7.1]: https://github.com/mrizaln/glfw-cpp/compare/v0.7.0...v0.7.1
[0.7.0]: https://github.com/mrizaln/glfw-cpp/compare/v0.6.0...v0.7.0
[0.6.0]: https://github.com/mrizaln/glfw-cpp/compare/v0.5.0...v0.6.0
[0.5.0]: https://github.com/mrizaln/glfw-cpp/compare/v0.4.0...v0.5.0
[0.4.0]: https://github.com/mrizaln/glfw-cpp/compare/v0.3.0...v0.4.0
[0.3.0]: https://github.com/mrizaln/glfw-cpp/compare/v0.2.0...v0.3.0
[0.2.0]: https://github.com/mrizaln/glfw-cpp/compare/v0.1.0...v0.2.0
[0.1.0]: https://github.com/mrizaln/glfw-cpp/releases/tag/v0.1.0

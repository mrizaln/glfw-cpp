# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [0.12.2] - 2026-01-06

### Added

- New `glfw_cpp/emscripten.hpp` header for extension functions for Emscripten.
- New `em` namespace for Emscripten extension functions.
- New `em::is_window_fullscreen` function for checking if window is fullscreen.
- New `em::request_fullscreen` function for requesting fullscreen.
- New `em::get_super_plus_key_timeout` function for getting `super + <other_key>` event timeout.
- New `em::set_super_plus_key_timeout` function for setting `super + <other_key>` event timeout.
- New `em::open_url` function for opening url.
- New `em::is_runtime_platform_apple` to check whether the Emscripten runtime is running on Apple platform.
- New `em::Result` enumeration that wraps `EMSCRIPTEN_RESULT` enumerators.
- New `hint::Platform::Emscripten` platform enumerator.
- New `hint::Emscripten` window creation hint.
- New `api::WebGL` client API window creation hint (only for Emscripten).
- New `imgui::ImguiHandle::process_events` for single event processing.

### Fixed

- Window context will always be set to `false` on Emscripten.
- Missing version check for Emscripten `make_current` workaround

## [0.12.1] - 2025-12-15

### Added

- New `Instance::platform` function to get the current platform used.
- New `platform_supported` function to check whether GLFW compiled with specified platform support.
- New `EventInterceptor` class combining `IEventInterceptor` and `DefaultEventInterceptor` class.
- New `get_proc_address_noexcept` function replacing noexcept `get_proc_address`.
- New `extension_supported_noexcept` function replacing noexcept `extension_supported`.
- New `vk::init_vulkan_loader` function that corresponds to `glfwInitVulkanLoader`.
- New `vk::get_instance_proc_address_noexcept` function for noexcept version of `vk::get_instance_proc_address`.

### Fixed

- Clarify GLFW version supported to be v3.4.
- Documentation for thrown exceptions refers to outdated definitions.
- Window creation throws on Wayland.
- Missing error checks on window attribute actions.

### Changed

- Examples now doesn't require Conan to be built.
- Vulkan support now can be used out of the box, without any linking required.
- Emscripten support is automatically detected now.
- `get_proc_address` can throw now.
- `extension_supported` can throw now.

### Removed

- Remove variable, `GLFW_CPP_VULKAN_SUPPORT`, for Vulkan support.
- Remove variable, `GLFW_CPP_EMSCRIPTEN_PORT`, for Emscripten port.
- Removed `IEventInterceptor` and `DefaultEventInterceptor`.
- Enumerator `ErrorCode::AlreadyBound` and `ErrorCode::EmptyLoader` since they are for pre-v0.12.0 errors.
- Error class `error::AlreadyBound` and `error::EmptyLoader` since they are for pre-v0.12.0 errors.
- Struct `event::Empty` since it never used.

## [0.12.0] - 2025-12-1

### Added

- New `Instance::create_window` function, replacing `WindowManager::create_window`.
- New `Instance::set_event_interceptor` function, replacing `WindowManager::set_event_interceptor`.
- New `Instance::has_window_opened` function, replacing `WindowManager::has_window_opened`.
- New `Instance::poll_events` function, replacing `WindowManager::poll_events`.
- New `Instance::wait_events` function, replacing `WindowManager::wait_events`.
- New `Instance::set_error_callback` function for setting error callback.
- New `Window::set_resizable` function that corresponds to setting resizable attribute.
- New `Window::set_floating` function that corresponds to setting floating attribute.
- New `Window::set_auto_iconify` function that corresponds to setting auto-iconify attribute.
- New `Window::set_focus_on_show` function that corresponds to setting focus-on-show attribute.
- New `Window::set_decorated` function that corresponds to setting decorated attribute.
- New `Window::set_mouse_passthrough` function that corresponds to setting decorated attribute.
- New `apply_hints`, `apply_hints_full`, and `apply_hints_default` for setting window creation hints.
- New `make_current` and `get_current` function for context management replacing `Window::bind` and `Window::unbind`
- New `get_proc_address` function for getting OpenGL functions (wrapping `glfwGetProcAddress` function).
- New `extension_supported` function for checking OpenGL extension function existence.
- New `event_name` function that gets the name of an event.
- New structs for window creation hints: `hint::Window`, `hint::Frambuffer`, `hint::Monitor`, `hint::Win32`, `hint::Cocoa`, `hint::Wayland`, `hint::X11`, `Hints`, `PartialHints`, `FullHints`.
- New enums for GLFW initialization hints: `hint::Platform`, `hint::AnglePlatform`, `hint::WaylandLibdecor`, `InitHints`.
- New enums for context hints: `gl::CreationApi`, `gl::Robustness`, `gl::ReleaseBehavior`.
- New window attributes: `decorated`, `transparent_framebuffer`, `mouse_passthrough`.
- Add GLFW constants for `GLFW_DONT_CARE` and `GLFW_ANY_POSITION` in its own namespace, `constants`.
- Emscripten support using [`contrib.glfw3`](https://github.com/pongasoft/emscripten-glfw) port.
- Emscripten example.

### Fixed

- Missing `#ifdef` block and enumeration for error codes.
- Queued tasks not flushed before destruction.
- Add missing `NoError` enumerator that represents `GLFW_NO_ERROR` on `ErrorCode` enum.

### Changed

- Headers guards now use a fixed pattern instead of using random letters.
- Change the signature for `init` to `void init(const InitHint&)` to allow for setting initialization hint.
- Window creation hints can be set like in `GLFW`, not passed in as argument each time to window creation function, through `apply_hints*` functions.
- Move `gl` namespace to outside of `api` namespace.
- Move `helper.hpp` and `imgui.hpp` headers to top level include.
- Move the window creation and events polling from `WindowManager` to `Instance`.
- Move error classes into its new `error` namespace: .
- ImGUI functionalities now lives in `imgui` namespace instead of `extra`.
- Remove `extra::ImguiInterceptor` class and replace the functionality with `imgui::ImguiHandle::process_events` function, making events handling explicit for ImGUI.
- Remove logger and replace it with error callback instead.
- Remove logger enumeration, `LogLevel`, since it is not necessary with the removal of general logging.
- Remove the constraint of having only one kind of client context API for all windows.
- Rename `Window::poll` to `Window::swap_events`.
- Rename `Window::display` to `Window::swap_buffers`.
- Rename `Window::is_vsync_enabled` to `Window::is_vsync` making it shorter.
- Separate window's `Attribute` from `Properties`.
- User-defined literal `_fps` now is `constexpr`.
- `ErrorCode` enumerators for glfw-cpp errors changed from `0x0002000X` to `0x000F000X`.

### Removed

- `WindowManager` class as it is not necessary (refer to [Issue #6](https://github.com/mrizaln/glfw-cpp/issues/6)).
- `Instance::create_window_manager` function.
- `Instance::set_logger` function.
- `Window::bind` and `Window::unbind` functions for context management.
- `Window::toggle_vsync` and `Window::toggle_capture_mouse` function since I deemed it redundant with the existence of `Window::is_vsync` and `Window::is_mouse_captured` function.
- `Flag` enum for window hints and its bitwise operators helper.

## [0.11.0] - 2025-10-21

### Added

- New `event` namespace.
- New `api` namespace.
- New bitwise operators for `Flag` enum class.
- Add variants API for `Api` the same as `Event`.
- ImGui example.

### Fixed

- Remove stray using declaration for `LogLevel` in `source/window_manager.cpp` file.
- Synchronization issue on `extra::ImguiInterceptor`.

### Changed

- Move Event variants into `event` namespace.
- Move Api variants into `api` namespace.
- Drop `m_*` prefix from public facing fields name.
- Extract nested structs and classes from `Window`, `WindowHint`, `Monitor`, `Instance`, and `Api`.
- Rename `WindowHint` to `Hint`.
- Rename `FlagBit` to `Flag` and make it an enum class.
- Rename `Event::holds` to `Event::is`.
- Make `Window` convertible to bool (explicit).

### Removed

- `ModifierKey::Bit::None` enumeration.

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
- Add `ErrorCode` information inside the `Error` class that can be queried with `code` member function.

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

[unreleased]: https://github.com/mrizaln/glfw-cpp/compare/v0.12.2...HEAD
[0.12.2]: https://github.com/mrizaln/glfw-cpp/compare/v0.12.1...v0.12.2
[0.12.1]: https://github.com/mrizaln/glfw-cpp/compare/v0.12.0...v0.12.1
[0.12.0]: https://github.com/mrizaln/glfw-cpp/compare/v0.11.0...v0.12.0
[0.11.0]: https://github.com/mrizaln/glfw-cpp/compare/v0.10.0...v0.11.0
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

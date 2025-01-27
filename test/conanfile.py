from conan import ConanFile
from conan.tools.cmake import cmake_layout


class Recipe(ConanFile):
    settings = ["os", "compiler", "build_type", "arch"]
    generators = ["CMakeToolchain", "CMakeDeps"]
    requires = [
        "fmt/11.0.2",
        "boost-ext-ut/1.1.9",
        "glbinding/3.3.0",
        "khrplatform/cci.20200529",
        "glfw/3.4",
        "vulkan-headers/1.3.268.0",
        "vulkan-loader/1.3.268.0",
    ]

    def layout(self):
        cmake_layout(self)

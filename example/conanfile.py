from conan import ConanFile
from conan.tools.cmake import cmake_layout, CMakeToolchain
from os import path


class Recipe(ConanFile):
    settings = ["os", "compiler", "build_type", "arch"]
    generators = ["CMakeDeps"]
    requires = [
        "fmt/10.2.1",
        "glad/0.1.36",
        "glbinding/3.3.0",
        "glfw/3.4",
        "glm/0.9.9.8",
        "imgui/1.90.5",
        "khrplatform/cci.20200529",
        "linmath.h/cci.20220619",
        "stb/cci.20230920",
        "tinycthread/cci.20161001",
        "vulkan-headers/1.3.268.0",
        "vulkan-loader/1.3.268.0",
    ]

    def layout(self):
        cmake_layout(self)

    def generate(self):
        tc = CMakeToolchain(self)
        tc.variables["IMGUI_BINDINGS_DIR"] = path.join(
            self.dependencies["imgui"].package_folder, "res", "bindings"
        )
        tc.generate()

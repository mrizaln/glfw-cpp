from conan import ConanFile


class Recipe(ConanFile):
    settings = ["os", "compiler", "build_type", "arch"]
    generators = ["CMakeToolchain", "CMakeDeps"]
    requires = [
        "fmt/10.2.1",
        "glfw/3.3.8",
        "glad/0.1.36",
        "glbinding/3.3.0",
        "glm/0.9.9.8",
        "vulkan-headers/1.3.268.0",
        "vulkan-loader/1.3.268.0",
        "linmath.h/cci.20220619",
        "stb/cci.20230920",
        "tinycthread/cci.20161001",
    ]

    def layout(self):
        self.folders.generators = "conan"

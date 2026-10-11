from conan import ConanFile
from conan.tools.cmake import CMakeToolchain


class LibreCAD3(ConanFile):
    settings = "os", "arch", "compiler", "build_type"
    generators = "CMakeDeps", "CMakeToolchain", "VirtualRunEnv"

    def configure(self):
        # Lua states cross DLL boundaries; separate static cores fail Lua 5.3's
        # identity check even when their version numbers match.
        if self.settings.os == "Windows":
            self.options["lua"].shared = True

    def requirements(self):
        self.requires("boost/1.82.0")
        self.requires("eigen/3.4.0")
        self.requires("freetype/2.13.0")
        self.requires("glew/2.2.0")
        self.requires("glfw/3.3.8")
        self.requires("glm/0.9.9.8")
        self.requires("gtest/1.14.0")
        self.requires("libcurl/8.15.0")
        self.requires("lua/5.3.5")
        self.requires("rapidjson/1.1.0")

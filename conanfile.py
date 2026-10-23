from conan import ConanFile
from conan.errors import ConanInvalidConfiguration
from conan.tools.cmake import CMakeToolchain


class LibreCAD3(ConanFile):
    settings = "os", "arch", "compiler", "build_type"
    generators = "CMakeDeps", "VirtualRunEnv"
    _cmake_options = (
        "WITH_QT_UI", "WITH_LUACMDINTERFACE", "WITH_UNITTESTS", "WITH_PYTHONSCRIPT",
        "WITH_LEGACY_OPENGL_RENDERER", "WITH_RHI_RENDERER",
        "WITH_RENDERING_UNITTESTS", "WITH_RENDERER_TESTS",
    )
    options = {name.lower(): [True, False] for name in _cmake_options}
    # Preserve the Windows package's historical host/test choices explicitly.
    # Qt (including RHI private headers/tools) and CPython remain external SDKs.
    default_options = {
        "with_qt_ui": True, "with_luacmdinterface": False,
        "with_unittests": True, "with_pythonscript": True,
        "with_legacy_opengl_renderer": True, "with_rhi_renderer": False,
        "with_rendering_unittests": False, "with_renderer_tests": False,
    }

    def validate(self):
        o = self.options
        if (o.with_qt_ui or o.with_luacmdinterface) and not o.with_legacy_opengl_renderer:
            raise ConanInvalidConfiguration("Current UI/CLI hosts require the legacy OpenGL renderer")
        if o.with_rendering_unittests and not (o.with_legacy_opengl_renderer and o.with_unittests):
            raise ConanInvalidConfiguration("Legacy rendering tests require legacy OpenGL and unit tests")
        if o.with_renderer_tests and not (o.with_rhi_renderer and o.with_unittests):
            raise ConanInvalidConfiguration("RHI tests require the RHI renderer and unit tests")

    def generate(self):
        tc = CMakeToolchain(self)
        for name in self._cmake_options:
            tc.variables["LC_CONAN_" + name] = bool(getattr(self.options, name.lower()))
        tc.generate()

    def requirements(self):
        self.requires("boost/1.82.0")
        self.requires("eigen/3.4.0")
        if self.options.with_legacy_opengl_renderer:
            self.requires("freetype/2.13.0")
            self.requires("glew/2.2.0")
            self.requires("glm/0.9.9.8")
            if self.options.with_luacmdinterface or self.options.with_rendering_unittests:
                self.requires("glfw/3.3.8")
        if self.options.with_unittests:
            self.requires("gtest/1.18.0")
        if self.options.with_luacmdinterface:
            self.requires("libcurl/8.23.0")
        self.requires("lua/5.3.5")
        self.requires("rapidjson/1.1.0")

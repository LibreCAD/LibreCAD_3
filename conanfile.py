from conan import ConanFile
from conan.tools.cmake import CMakeToolchain
import hashlib
import json
from pathlib import Path


class LibreCAD3(ConanFile):
    settings = "os", "arch", "compiler", "build_type"
    generators = "CMakeDeps", "VirtualRunEnv"

    def generate(self):
        # Keep notices from the exact selected host binaries, including their
        # transitive dependencies. Build-tool licenses are not runtime inputs.
        providers, contents = [], {}
        for dep in sorted(self.dependencies.host.values(),
                          key=lambda item: (item.ref.name, str(item.ref.version))):
            identity = dict(reference="{}/{}".format(dep.ref.name, dep.ref.version),
                            recipe_revision=str(dep.ref.revision),
                            package_id=str(dep.pref.package_id),
                            package_revision=str(dep.pref.revision))
            if any(not value or value == "None" for value in identity.values()):
                raise RuntimeError("Conan package notice identity is incomplete: " + identity["reference"])
            if not dep.package_folder:
                raise RuntimeError("Conan package notices require a cached binary: " + identity["reference"])
            package = Path(dep.package_folder)
            system_libs = {"opengl/system": ["opengl32"], "glu/system": ["glu32"]}.get(identity["reference"])
            if system_libs and str(dep.settings.os) == "Windows":
                # These wrappers only link Windows system DLLs and distribute
                # no library/header bytes. Never extend this to arbitrary
                # packages merely because their license directory is absent.
                if not package.is_dir() or package.is_symlink():
                    raise RuntimeError(identity["reference"] + " cached package directory is missing or linked")
                info = dep.cpp_info
                if info.system_libs != system_libs or any((info.libs, info.objects,
                        info.frameworks, info.includedirs, info.libdirs, info.bindirs, info.components)):
                    raise RuntimeError("Unexpected Windows system provider metadata: " + identity["reference"])
                for path in package.rglob("*"):
                    if path.is_symlink() or (path.is_file() and path.relative_to(package).as_posix()
                                            not in ("conaninfo.txt", "conanmanifest.txt")):
                        raise RuntimeError(identity["reference"] + " unexpectedly distributes package files")
                providers.append(dict(identity, files=[], system_os="Windows", system_libraries=system_libs))
                continue
            source = package / "licenses"
            if not source.is_dir() or source.is_symlink():
                raise RuntimeError("Conan package license directory is missing or linked: " + str(source))
            key = hashlib.sha256(json.dumps(identity, sort_keys=True).encode()).hexdigest()
            files = []
            for path in sorted(source.rglob("*")):
                if path.is_symlink():
                    raise RuntimeError("Conan license symlinks are not supported: " + str(path))
                if not path.is_file():
                    continue
                data = path.read_bytes()
                if not data.strip():
                    raise RuntimeError("Conan license file is empty: " + str(path))
                relative = (Path(key) / path.relative_to(source)).as_posix()
                contents[relative] = data
                files.append(dict(path=relative, sha256=hashlib.sha256(data).hexdigest()))
            if not files:
                raise RuntimeError("Conan package has zero license files: " + identity["reference"])
            providers.append(dict(identity, files=files))
        if not providers or not contents:
            raise RuntimeError("Conan provider notice graph is empty")
        manifest = json.dumps(dict(format=1, providers=providers), indent=2, sort_keys=True) + "\n"
        # A content-addressed directory cannot retain notices from an older
        # graph. Do not recursively erase a caller's generators directory.
        directory = (Path(self.generators_folder) / "librecad-licenses" /
                     hashlib.sha256(manifest.encode()).hexdigest())
        for relative, data in contents.items():
            destination = directory / relative
            destination.parent.mkdir(parents=True, exist_ok=True)
            destination.write_bytes(data)
        (directory / "manifest.json").write_text(manifest, encoding="utf-8")
        toolchain = CMakeToolchain(self)
        # CI invokes CMake directly, not via Conan's configure preset.
        toolchain.variables["LIBRECAD_DEPENDENCY_LICENSE_DIR"] = directory.as_posix()
        toolchain.generate()

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

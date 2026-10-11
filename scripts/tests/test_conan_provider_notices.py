"""Selected Conan graph notices: generation, installation and artifact audit."""

import ast
import hashlib
import importlib.util
import json
import os
import shutil
import subprocess
import tempfile
import unittest
from unittest.mock import patch
from pathlib import Path
from types import SimpleNamespace


ROOT = Path(__file__).resolve().parents[2]
spec = importlib.util.spec_from_file_location("notice_runtimeaudit", ROOT / "lcUIPy/_runtimeaudit.py")
audit = importlib.util.module_from_spec(spec)
spec.loader.exec_module(audit)


class ConanProviderNoticesTests(unittest.TestCase):
    def setUp(self):
        temporary = tempfile.TemporaryDirectory()
        self.addCleanup(temporary.cleanup)
        self.root = Path(temporary.name)
        self.generators = self.root / "generated Ω"
        self.generators.mkdir()
        self.dependencies = {}
        self.toolchain = SimpleNamespace(variables={}, generate=lambda: None)

    def dependency(self, name, version="1.2", files=None):
        folder = self.root / name
        for relative, content in (files or {"COPYING": b"original license\r\n"}).items():
            destination = folder / "licenses" / relative
            destination.parent.mkdir(parents=True, exist_ok=True)
            destination.write_bytes(content)
        dep = SimpleNamespace(package_folder=str(folder),
                              ref=SimpleNamespace(name=name, version=version, revision="recipe-revision"),
                              pref=SimpleNamespace(package_id="binary-id", revision="package-revision"))
        self.dependencies[name] = dep
        return dep

    def generate(self):
        recipe = next(node for node in ast.parse((ROOT / "conanfile.py").read_text()).body
                      if isinstance(node, ast.ClassDef) and node.name == "LibreCAD3")
        method = next((node for node in recipe.body if isinstance(node, ast.FunctionDef)
                       and node.name == "generate"), None)
        self.assertIsNotNone(method, "Selected Conan dependencies must stage their own notices")
        namespace = dict(Path=Path, hashlib=hashlib, json=json, shutil=shutil,
                         CMakeToolchain=lambda consumer: self.toolchain)
        exec(compile(ast.fix_missing_locations(ast.Module(body=[method], type_ignores=[])),
                     "conanfile.py", "exec"), namespace)
        namespace["generate"](SimpleNamespace(generators_folder=str(self.generators),
                                             dependencies=SimpleNamespace(host=self.dependencies)))
        return Path(self.toolchain.variables["LIBRECAD_DEPENDENCY_LICENSE_DIR"])

    def test_preserves_every_host_package_notice_and_identity(self):
        self.dependency("direct", files={"COPYING": b"all original bytes\x00\r\n",
                                          "third-party/NOTICE Ω": b"nested notice\n"})
        self.dependency("transitive")
        directory = self.generate()
        manifest = json.loads((directory / "manifest.json").read_text())
        self.assertEqual(1, manifest["format"])
        self.assertEqual(["direct/1.2", "transitive/1.2"],
                         [provider["reference"] for provider in manifest["providers"]])
        for provider in manifest["providers"]:
            self.assertEqual("recipe-revision", provider["recipe_revision"])
            self.assertEqual("binary-id", provider["package_id"])
            self.assertEqual("package-revision", provider["package_revision"])
            for entry in provider["files"]:
                copied = directory / entry["path"]
                self.assertEqual(entry["sha256"], hashlib.sha256(copied.read_bytes()).hexdigest())
        self.assertEqual((2, 3), audit.audit_conan_notices(directory))
        before = (directory / "manifest.json").read_bytes()
        self.assertEqual(directory, self.generate())
        self.assertEqual(before, (directory / "manifest.json").read_bytes())

    def test_missing_editable_empty_and_escaped_notices_fail_generation(self):
        for kind in ("missing", "editable", "empty", "escaped", "identity"):
            with self.subTest(kind=kind):
                self.dependencies.clear()
                dep = self.dependency(kind)
                license_path = Path(dep.package_folder) / "licenses/COPYING"
                if kind == "missing":
                    license_path.unlink()
                elif kind == "editable":
                    dep.package_folder = None
                elif kind == "empty":
                    license_path.write_bytes(b"")
                elif kind == "escaped":
                    outside = self.root / "outside-license"
                    outside.write_text("unselected provider")
                    license_path.unlink()
                    license_path.symlink_to(outside)
                else:
                    dep.pref.revision = None
                with self.assertRaisesRegex(RuntimeError, "notice|license|package"):
                    self.generate()

    def test_empty_graph_is_not_complete_provider_coverage(self):
        with self.assertRaisesRegex(RuntimeError, "empty|zero"):
            self.generate()

    def system_dependency(self, name="opengl"):
        folder = self.root / ("system-" + name)
        folder.mkdir(exist_ok=True)
        (folder / "conaninfo.txt").write_text("cache metadata")
        dep = SimpleNamespace(package_folder=str(folder),
                              ref=SimpleNamespace(name=name, version="system", revision="recipe-revision"),
                              pref=SimpleNamespace(package_id="binary-id", revision="package-revision"),
                              settings=SimpleNamespace(os="Windows"),
                              cpp_info=SimpleNamespace(system_libs=[{"opengl": "opengl32", "glu": "glu32"}[name]], libs=[], objects=[],
                                  frameworks=[], includedirs=[], libdirs=[], bindirs=[], components={}))
        self.dependencies[name] = dep
        return dep

    def test_records_empty_windows_opengl_system_wrapper_without_inventing_a_license(self):
        self.dependency("provider")
        self.system_dependency()
        directory = self.generate()
        manifest = json.loads((directory / "manifest.json").read_text())
        system = manifest["providers"][0]
        self.assertEqual("opengl/system", system["reference"])
        self.assertEqual([], system["files"])
        self.assertEqual("Windows", system["system_os"])
        self.assertEqual(["opengl32"], system["system_libraries"])
        self.assertEqual((2, 1), audit.audit_conan_notices(directory))

    def test_windows_glu_wrapper_requires_exact_system_library_and_no_payload(self):
        self.dependency("provider")
        dep = self.system_dependency("glu")
        directory = self.generate()
        manifest = json.loads((directory / "manifest.json").read_text())
        system = manifest["providers"][0]
        self.assertEqual("glu/system", system["reference"])
        self.assertEqual([], system["files"])
        self.assertEqual(["glu32"], system["system_libraries"])
        self.assertEqual((2, 1), audit.audit_conan_notices(directory))
        for kind in ("library", "platform", "payload", "include"):
            with self.subTest(kind=kind):
                dep.cpp_info.system_libs = ["opengl32"] if kind == "library" else ["glu32"]
                dep.settings.os = "Linux" if kind == "platform" else "Windows"
                dep.cpp_info.includedirs = ["include"] if kind == "include" else []
                payload = Path(dep.package_folder) / "glu32.dll"
                if kind == "payload":
                    payload.write_bytes(b"MZnot-system")
                with self.assertRaises(RuntimeError):
                    self.generate()
                if payload.exists():
                    payload.unlink()
        manifest["providers"][0]["system_libraries"] = ["opengl32"]
        (directory / "manifest.json").write_text(json.dumps(manifest))
        with self.assertRaisesRegex(RuntimeError, "system"):
            audit.audit_conan_notices(directory)

    def test_system_exemption_rejects_other_platforms_metadata_and_distributed_bytes(self):
        for kind in ("platform", "library", "include", "binary", "link", "unknown", "missing", "system-only"):
            with self.subTest(kind=kind):
                self.dependencies.clear()
                if kind != "system-only":
                    self.dependency("provider")
                dep = self.system_dependency()
                if kind == "platform":
                    dep.settings.os = "Linux"
                elif kind == "library":
                    dep.cpp_info.libs = ["GL"]
                elif kind == "include":
                    dep.cpp_info.includedirs = ["include"]
                elif kind in ("binary", "link"):
                    path = Path(dep.package_folder) / "opengl32.dll"
                    if kind == "binary":
                        path.write_bytes(b"MZnot-system")
                    else:
                        path.symlink_to(Path(dep.package_folder) / "conaninfo.txt")
                elif kind == "unknown":
                    dep.ref.name = "unrecognized"
                elif kind == "missing":
                    dep.package_folder += "-missing"
                with self.assertRaises(RuntimeError):
                    self.generate()
                # Do not carry the previous negative fixture into the next case.
                if kind in ("binary", "link"):
                    path.unlink()

    def test_installed_audit_rejects_arbitrary_system_notice_exemptions(self):
        self.dependency("provider")
        self.system_dependency()
        original = self.generate()
        for key, value in (("reference", "unknown/system"), ("system_os", "Linux"),
                           ("system_libraries", ["arbitrary.dll"]), ("files", [{"path": "outside"}])):
            with self.subTest(key=key):
                manifest = json.loads((original / "manifest.json").read_text())
                manifest["providers"][0][key] = value
                (original / "manifest.json").write_text(json.dumps(manifest))
                with self.assertRaisesRegex(RuntimeError, "system"):
                    audit.audit_conan_notices(original)
                self.generate()  # Restore the content-addressed inventory.

    def test_installed_audit_rejects_missing_tampered_unlisted_and_escaped_files(self):
        self.dependency("provider")
        original = self.generate()
        for kind in ("missing", "tampered", "unlisted", "escaped", "manifest", "empty",
                     "absolute", "parent", "backslash", "duplicate", "identity", "schema"):
            with self.subTest(kind=kind):
                installed = self.root / ("install " + kind)
                shutil.copytree(original, installed)
                manifest_path = installed / "manifest.json"
                manifest = json.loads(manifest_path.read_text())
                entry = manifest["providers"][0]["files"][0]
                file = installed / entry["path"]
                if kind == "missing":
                    file.unlink()
                elif kind == "tampered":
                    file.write_bytes(b"different license")
                elif kind == "unlisted":
                    (installed / "not-in-manifest").write_text("extra")
                elif kind == "escaped":
                    file.unlink()
                    file.symlink_to(self.root / "provider/licenses/COPYING")
                elif kind == "manifest":
                    manifest_path.unlink()
                elif kind == "empty":
                    manifest["providers"] = []
                elif kind == "absolute":
                    entry["path"] = "/outside"
                elif kind == "parent":
                    entry["path"] = "../outside"
                elif kind == "backslash":
                    entry["path"] = "..\\outside"
                elif kind == "duplicate":
                    manifest["providers"][0]["files"].append(dict(entry))
                elif kind == "identity":
                    manifest["providers"][0]["package_revision"] = "None"
                else:
                    manifest = []
                if kind not in ("missing", "tampered", "unlisted", "escaped", "manifest"):
                    manifest_path.write_text(json.dumps(manifest))
                with self.assertRaises((RuntimeError, OSError)):
                    audit.audit_conan_notices(installed)

    @unittest.skipUnless(shutil.which("cmake"), "real CMake notice install fixture")
    def test_common_install_retains_the_generated_inventory_and_requires_manifest(self):
        self.dependency("provider")
        notices = self.generate()
        block = (ROOT / "CMakeLists.txt").read_text().split(
            "# BEGIN Conan provider notices\n", 1)[1].split("# END Conan provider notices", 1)[0]
        defaults = (ROOT / "CMakeLists.txt").read_text().split(
            "include(GNUInstallDirs)\n", 1)[1].split("# BEGIN source license installation", 1)[0]
        source = self.root / "cmake"
        source.mkdir()
        (source / "CMakeLists.txt").write_text(
            'cmake_minimum_required(VERSION 3.28)\nproject(Notices LANGUAGES NONE)\n'
            'set(MSVC TRUE)\nset(CMAKE_INSTALL_DATADIR "")\n' + defaults +
            'set(LIBRECAD_DEPENDENCY_LICENSE_DIR "{}")\n'.format(notices.as_posix()) + block)
        build, prefix = self.root / "build", self.root / "installed Ω"
        commands = (["cmake", "-S", str(source), "-B", str(build)],
                    ["cmake", "--install", str(build), "--prefix", str(prefix)])
        for index, command in enumerate(commands):
            if index == 1:
                self.assertIn('${CMAKE_INSTALL_PREFIX}/share/librecad/licenses/providers/conan',
                              (build / "cmake_install.cmake").read_text())
            result = subprocess.run(command, text=True, capture_output=True, timeout=30)
            self.assertEqual(0, result.returncode, result.stdout + result.stderr)
        installed = prefix / "share/librecad/licenses/providers/conan"
        self.assertEqual((1, 1), audit.audit_conan_notices(installed))
        self.assertEqual((notices / "manifest.json").read_bytes(),
                         (installed / "manifest.json").read_bytes())
        (notices / "manifest.json").unlink()
        result = subprocess.run(["cmake", "-S", str(source), "-B", str(build)],
                                text=True, capture_output=True, timeout=30)
        self.assertNotEqual(0, result.returncode)
        self.assertIn("Conan provider notice manifest", result.stdout + result.stderr)

    def test_actual_embedded_smoke_requires_and_audits_installed_notices(self):
        self.dependency("provider")
        source = self.generate()
        data = self.root / "data"
        package = data / "lcUIPy"
        package.mkdir(parents=True)
        installed = data / "licenses/providers/conan"
        shutil.copytree(source, installed)
        code = (ROOT / "lcUI/python/pyruntimesmoke.cpp").read_text().split(
            "# BEGIN required Conan notices\n", 1)[1].split("# END required Conan notices", 1)[0]
        namespace = dict(os=os, _package_root=package.resolve())
        with patch.dict(os.environ, LIBRECAD_REQUIRE_CONAN_NOTICES="1"), \
                patch.dict("sys.modules", {"lcUIPy._runtimeaudit": audit}):
            exec(code, namespace)
            self.assertIn("1 providers, 1 files", namespace["_notice_diagnostics"])
            (installed / "manifest.json").unlink()
            with self.assertRaises(FileNotFoundError):
                exec(code, namespace)

    def test_windows_packaging_requires_notices_before_and_after_nsis(self):
        script = (ROOT / "scripts/windows-install/buildLibrecadAndCreatePackage.bat").read_text()
        workflow = (ROOT / ".github/workflows/main.yml").read_text()
        self.assertIn('set "LIBRECAD_REQUIRE_CONAN_NOTICES=1"', script)
        self.assertIn("$env:LIBRECAD_REQUIRE_CONAN_NOTICES = '1'", workflow)

    @unittest.skipUnless(shutil.which("conan") and shutil.which("cmake"),
                         "actual Conan/CMake local-cache integration")
    def test_actual_conan_graph_and_direct_toolchain_install(self):
        env = dict(os.environ, CONAN_HOME=str(self.root / "isolated-conan-cache"))
        def run(*arguments):
            result = subprocess.run(arguments, env=env, capture_output=True, text=True, timeout=90)
            self.assertEqual(0, result.returncode, result.stdout + result.stderr)
        run("conan", "profile", "detect", "--force")
        for name, requirement in (("notice-leaf", ""),
                                  ("notice-direct", 'requires = "notice-leaf/1.0"')):
            source = self.root / name
            source.mkdir()
            (source / "LICENSE.txt").write_bytes((name + " original license\r\n").encode())
            (source / "conanfile.py").write_text(
                'from conan import ConanFile\nfrom conan.tools.files import copy\n'
                'import os\nclass Notice(ConanFile):\n'
                '    name = "{}"\n    version = "1.0"\n'.format(name) +
                '    package_type = "header-library"\n    exports_sources = "LICENSE.txt"\n'
                '    {}\n'.format(requirement) +
                '    def package(self):\n'
                '        copy(self, "LICENSE.txt", self.source_folder, '
                'os.path.join(self.package_folder, "licenses"))\n')
            run("conan", "create", str(source), "--no-remote")
        consumer = self.root / "consumer"
        consumer.mkdir()
        (consumer / "conanfile.py").write_text(
            'import importlib.util\n'
            'spec = importlib.util.spec_from_file_location("librecad_recipe", {!r})\n'.format(str(ROOT / "conanfile.py")) +
            'module = importlib.util.module_from_spec(spec)\nspec.loader.exec_module(module)\n'
            'class NoticeConsumer(module.LibreCAD3):\n'
            '    def configure(self): pass\n'
            '    def requirements(self): self.requires("notice-direct/1.0")\n')
        output = self.root / "actual generated Ω"
        run("conan", "install", str(consumer), "--output-folder", str(output),
            "--build=never", "--no-remote")
        block = (ROOT / "CMakeLists.txt").read_text().split(
            "# BEGIN Conan provider notices\n", 1)[1].split("# END Conan provider notices", 1)[0]
        (consumer / "CMakeLists.txt").write_text(
            'cmake_minimum_required(VERSION 3.28)\nproject(Notices LANGUAGES NONE)\n'
            'set(CMAKE_INSTALL_DATADIR share)\n' + block)
        build, prefix = self.root / "actual build", self.root / "actual install Ω"
        run("cmake", "-S", str(consumer), "-B", str(build),
            "-DCMAKE_TOOLCHAIN_FILE=" + str(output / "conan_toolchain.cmake"))
        run("cmake", "--install", str(build), "--prefix", str(prefix))
        installed = prefix / "share/librecad/licenses/providers/conan"
        self.assertEqual((2, 2), audit.audit_conan_notices(installed))
        self.assertEqual(["notice-direct/1.0", "notice-leaf/1.0"],
                         [p["reference"] for p in json.loads((installed / "manifest.json").read_text())["providers"]])
        # Validate the actual Conan dependency interface for Windows' empty
        # system wrappers too. Only metadata/header packages are created; no
        # Windows compiler, SDK, dependency source build or remote is used.
        windows = ["-s:h", "os=Windows", "-s:h", "arch=x86_64", "-s:h", "compiler=msvc",
                   "-s:h", "compiler.version=194", "-s:h", "compiler.cppstd=17",
                   "-s:h", "compiler.runtime=dynamic"]
        for name, library in (("opengl", "opengl32"), ("glu", "glu32")):
            system = self.root / ("actual-" + name + "-system")
            system.mkdir()
            (system / "conanfile.py").write_text(
                'from conan import ConanFile\nclass System(ConanFile):\n'
                '    name = "{}"\n    version = "system"\n    settings = "os"\n'.format(name) +
                '    package_type = "shared-library"\n'
                '    def package_info(self):\n'
                '        self.cpp_info.includedirs = []\n        self.cpp_info.libdirs = []\n'
                '        self.cpp_info.bindirs = []\n        self.cpp_info.system_libs = ["{}"]\n'.format(library))
            run("conan", "create", str(system), "--no-remote", *windows)
        recipe = consumer / "conanfile.py"
        recipe.write_text(recipe.read_text().replace(
            'self.requires("notice-direct/1.0")',
            'self.requires("notice-direct/1.0"); self.requires("opengl/system"); self.requires("glu/system")'))
        win_output = self.root / "Windows generated"
        run("conan", "install", str(consumer), "--output-folder", str(win_output),
            "--build=never", "--no-remote", *windows)
        manifests = list((win_output / "librecad-licenses").glob("*/manifest.json"))
        self.assertEqual(1, len(manifests))
        self.assertEqual((4, 2), audit.audit_conan_notices(manifests[0].parent))
        self.assertEqual({"opengl/system": ["opengl32"], "glu/system": ["glu32"]},
                         {provider["reference"]: provider["system_libraries"]
                          for provider in json.loads(manifests[0].read_text())["providers"]
                          if "system_libraries" in provider})


if __name__ == "__main__":
    unittest.main()

#ifdef LC_WITH_PYTHONSCRIPT

#include "pyruntimesmoke.h"

#include "pypluginloader.h"
#include "pyeventhooks.h"

#include <lcpython.h>
#include <qt_keywords_push.h>
#include <pybind11/embed.h>
#include <qt_keywords_pop.h>

#include <cad/storage/documentimpl.h>
#include <cad/storage/storagemanagerimpl.h>
#include <cad/storage/undomanagerimpl.h>

#include <QDir>
#include <QFileInfo>

#ifdef LC_PYTHON_SMOKE_WITH_PERSISTENCE
#include <QTemporaryDir>
#endif

#include <iostream>
#include <memory>
#include <set>
#include <string>

namespace py = pybind11;

namespace lc {
namespace ui {
namespace python {

int runPythonRuntimeSmoke() {
    const QString packageRoot = resolvePythonPackageRoot();
    if (packageRoot.isEmpty()) {
        std::cerr << "Python smoke failed: could not locate lcUIPy beside the app, "
                     "in its install prefix, or in its app bundle\n";
        return 1;
    }
    const QDir packageDirectory = QFileInfo(packageRoot).dir();
    const QString stagedPythonRoot = packageDirectory.filePath("python");
    const QString stagedPythonLicense = packageDirectory.filePath(
        "licenses/python/LICENSE.txt");
    const QString packageParentDirectoryName =
        QFileInfo(packageDirectory.absolutePath()).dir().dirName();
    const bool installedShareLayout = packageDirectory.dirName() == "librecad" &&
        packageParentDirectoryName == "share";
    const bool appBundleResourcesLayout = packageDirectory.dirName() == "Resources";
    const bool stagedPythonExpected = installedShareLayout ||
        appBundleResourcesLayout || QDir(stagedPythonRoot).exists();

    try {
        lc::python::PythonInit::initialize();
        lc::python::LCPython python;
        auto ns = python.makeNamespace();
        ns.set("python_package_root", packageRoot.toStdString());
        ns.set("expected_staged_python_root",
               stagedPythonExpected ? stagedPythonRoot.toStdString() : std::string());
        ns.set("expected_staged_python_license",
               stagedPythonExpected ? stagedPythonLicense.toStdString() : std::string());

        auto storageManager = std::make_shared<lc::storage::StorageManagerImpl>();
        auto document = std::make_shared<lc::storage::DocumentImpl>(storageManager);
        auto undoManager = std::make_shared<lc::storage::UndoManagerImpl>(10);
        document->commitProcessEvent().connect<lc::storage::UndoManagerImpl,
            &lc::storage::UndoManagerImpl::on_CommitProcessEvent>(undoManager.get());
        python.setDocument(ns, document);
        ns.set("undo_manager", undoManager);

#ifdef LC_PYTHON_SMOKE_WITH_PERSISTENCE
        auto roundTripStorage = std::make_shared<lc::storage::StorageManagerImpl>();
        auto roundTripDocument = std::make_shared<lc::storage::DocumentImpl>(roundTripStorage);
        ns.set("round_trip_document", roundTripDocument);
        QTemporaryDir temporaryDirectory;
        if (!temporaryDirectory.isValid()) {
            std::cerr << "Python smoke failed: could not create a temporary directory\n";
            return 1;
        }
        const QString dxfPath = temporaryDirectory.filePath("python-runtime-smoke.dxf");
        ns.set("dxf_path", dxfPath.toStdString());
#endif

        const std::string importsAndProviderCheck = R"py(
import ctypes
import encodings
import gzip
import hashlib
import json
import math
import os
import pathlib
import sqlite3
import ssl
import sys
import zlib
import _ctypes
import _sqlite3
import _ssl

assert sys.flags.isolated == 1, f"isolated flag is {sys.flags.isolated}"
assert sys.flags.ignore_environment == 1, \
    f"ignore_environment flag is {sys.flags.ignore_environment}"
_package_root = pathlib.Path(python_package_root).resolve()
sys.path.insert(0, str(_package_root.parent))
_package_file = pathlib.Path(__import__('lcUIPy').__file__).resolve()
assert _package_file.is_relative_to(_package_root), \
    f"lcUIPy loaded from {_package_file}, expected under {_package_root}"
if expected_staged_python_root:
    _expected_prefix = pathlib.Path(expected_staged_python_root).resolve()
    _actual_prefix = pathlib.Path(sys.prefix).resolve()
    assert _expected_prefix.is_dir(), \
        f"expected staged Python runtime is missing: {_expected_prefix}"
    assert _actual_prefix == _expected_prefix, \
        f"Python selected {_actual_prefix}, expected staged runtime {_expected_prefix}"
    _expected_license = pathlib.Path(expected_staged_python_license).resolve()
    assert _expected_license.is_file() and _expected_license.stat().st_size > 0, \
        f"selected CPython license is missing from the staged package: {_expected_license}"
for _module in (
    ctypes, encodings, gzip, hashlib, json, math, os, pathlib, sqlite3, ssl,
    sys, zlib, _ctypes, _sqlite3, _ssl,
):
    _module_file = getattr(_module, '__file__', None)
    if _module_file is None:
        _module_spec = getattr(_module, '__spec__', None)
        _origin = getattr(_module_spec, 'origin', None)
        assert _module.__name__ in sys.builtin_module_names or \
            _origin in ('built-in', 'frozen'), \
            f"{_module.__name__} has no file, built-in, or frozen origin"
        continue
    _origin = pathlib.Path(_module_file).resolve()
    _prefix = pathlib.Path(sys.prefix).resolve()
    assert _origin.is_relative_to(_prefix), \
        f"{_module.__name__} loaded from {_origin}, outside Python prefix {_prefix}"
_windows_diagnostics = ""
if os.environ.get("LIBRECAD_REQUIRE_BUNDLED_PYTHON") == "1":
    assert expected_staged_python_root, \
        "cannot verify packaged libpython without an expected staged Python runtime"
    if sys.platform.startswith("linux"):
        _maps_path = pathlib.Path("/proc/self/maps")
        assert _maps_path.is_file(), \
            f"cannot inspect loaded libraries because {_maps_path} is unavailable"
        _libpython_paths = set()
        _mapped_library_paths = set()
        for _mapping in _maps_path.read_text().splitlines():
            _mapping_fields = _mapping.split(maxsplit=5)
            if len(_mapping_fields) != 6:
                continue
            _mapped_path = _mapping_fields[5].removesuffix(" (deleted)")
            if not _mapped_path.startswith("/"):
                continue
            _resolved_path = pathlib.Path(_mapped_path).resolve()
            _mapped_library_paths.add(_resolved_path)
            if "libpython" in _mapped_path:
                _libpython_paths.add(_resolved_path)
        assert _libpython_paths, "no dynamically loaded libpython was found in /proc/self/maps"
        _package_usr = pathlib.Path(expected_staged_python_root).resolve().parents[2]
        _external_libpython = [
            _path for _path in _libpython_paths if not _path.is_relative_to(_package_usr)
        ]
        assert not _external_libpython, \
            f"libpython loaded outside the packaged usr tree {_package_usr}: {_external_libpython}"
        if os.environ.get("LIBRECAD_REQUIRE_PYTHON_EXTENSION_CLOSURE") == "1":
            for _dependency_name in (
                "libssl.so", "libcrypto.so", "libsqlite3.so", "libffi.so", "libz.so"):
                _dependency_paths = {
                    _path for _path in _mapped_library_paths
                    if _path.name.startswith(_dependency_name)
                }
                assert _dependency_paths, \
                    f"Python extension dependency {_dependency_name} was not loaded"
                _external_dependency_paths = [
                    _path for _path in _dependency_paths
                    if not _path.is_relative_to(_package_usr)
                ]
                assert not _external_dependency_paths, \
                    f"Python extension dependency {_dependency_name} loaded outside the packaged usr tree " \
                    f"{_package_usr}: {_external_dependency_paths}"
    elif sys.platform == "win32":
        _kernel32 = ctypes.WinDLL("kernel32", use_last_error=True)
        _get_module_handle = _kernel32.GetModuleHandleW
        _get_module_handle.argtypes = (ctypes.c_wchar_p,)
        _get_module_handle.restype = ctypes.c_void_p
        _get_module_filename = _kernel32.GetModuleFileNameW
        _get_module_filename.argtypes = (
            ctypes.c_void_p, ctypes.POINTER(ctypes.c_wchar), ctypes.c_uint32)
        _get_module_filename.restype = ctypes.c_uint32
        _python_dll_name = (
            f"python{sys.version_info.major}{sys.version_info.minor}.dll")
        _python_dll_handle = _get_module_handle(_python_dll_name)
        assert _python_dll_handle, \
            f"loaded CPython runtime DLL {_python_dll_name} was not found"
        _python_dll_buffer = ctypes.create_unicode_buffer(32768)
        _python_dll_length = _get_module_filename(
            _python_dll_handle, _python_dll_buffer, len(_python_dll_buffer))
        assert 0 < _python_dll_length < len(_python_dll_buffer), \
            f"could not resolve the loaded CPython DLL path (Win32 error {ctypes.get_last_error()})"
        _python_dll_path = pathlib.Path(_python_dll_buffer.value).resolve()
        _package_root = pathlib.Path(expected_staged_python_root).resolve().parents[2]
        _expected_runtime_dir = (_package_root / "bin").resolve()
        assert _python_dll_path.is_relative_to(_expected_runtime_dir), \
            f"CPython loaded from {_python_dll_path}, expected under {_expected_runtime_dir}"
        if os.environ.get("LIBRECAD_REQUIRE_PYTHON_EXTENSION_CLOSURE") == "1":
            from lcUIPy._runtimeaudit import audit_windows_extensions
            _windows_extension_count = audit_windows_extensions(
                pathlib.Path(expected_staged_python_root).resolve())
            _windows_diagnostics = \
                f"; Windows extension closure: {_windows_extension_count} distinct files"
    else:
        raise AssertionError(
            f"bundled-libpython verification is unsupported on {sys.platform}")
_elf_diagnostics = ""
if os.environ.get("LIBRECAD_REQUIRE_FULL_ELF_CLOSURE") == "1":
    assert sys.platform.startswith("linux") and expected_staged_python_root and \
        os.environ.get("LIBRECAD_REQUIRE_BUNDLED_PYTHON") == "1", \
        "full ELF closure requires an installed glibc/Linux package"
    from lcUIPy._runtimeaudit import audit_elf_closure
    _elf_root = pathlib.Path(expected_staged_python_root).resolve().parents[3]
    assert _elf_root != pathlib.Path(_elf_root.anchor) and \
        (_elf_root / "usr/share/librecad/python").resolve() == \
        pathlib.Path(expected_staged_python_root).resolve() and \
        ((_elf_root / "AppRun").is_file() or (_elf_root / "meta/snap.yaml").is_file()), \
        "full ELF closure requires a recognized AppImage or Snap root"
    _elf_count = audit_elf_closure(_elf_root)
    _elf_diagnostics = f"; full payload ELF closure: {_elf_count} distinct files"
import lcgui
assert hasattr(lcgui, 'MainWindow'), "embedded lcgui module lacks MainWindow"
runtime_diagnostics = (
    f"Python runtime: version={sys.version.split()[0]}, prefix={sys.prefix}; "
    f"lcUIPy={_package_file}; module search paths={sys.path}{_elf_diagnostics}{_windows_diagnostics}"
)
)py";
        const std::string importError = python.runString(importsAndProviderCheck.c_str(), ns);
        if (!importError.empty()) {
            std::cerr << "Python smoke failed during runtime imports/provider checks:\n"
                      << importError << std::endl;
            return 1;
        }
        {
            py::gil_scoped_acquire gil;
            std::cout << py::cast<std::string>(ns.dict()["runtime_diagnostics"])
                      << std::endl;
        }

        installEventHooks();
        const QString pluginDirectory = QDir(packageRoot).filePath("plugins");
        const PythonPluginLoadReport pluginReport =
            loadPythonPlugins(pluginDirectory.toStdString(), "gui");
        if (!pluginReport.directoryFound || !pluginReport.failures.empty()) {
            std::cerr << "Python smoke failed loading plugins from "
                      << pluginDirectory.toStdString() << '\n';
            for (const auto& failure : pluginReport.failures) {
                std::cerr << "  " << failure << '\n';
            }
            return 1;
        }
        const std::set<std::string> loaded(pluginReport.loadedPlugins.begin(),
                                           pluginReport.loadedPlugins.end());
        if (loaded.find("gear") == loaded.end()) {
            std::cerr << "Python smoke failed: production plugin loader did not load gear\n";
            return 1;
        }

        std::string script = R"py(
import lc
assert 'PyGearOp' in lc.operation_registry, \
    f"gear plugin failed to register operation: {list(lc.operation_registry)}"

layer = lc.meta.Layer('runtime-smoke', lc.meta.MetaLineWidthByValue(1.0),
                      lc.Color(255, 255, 255), None, False)
line_builder = lc.builder.LineBuilder()
line_builder.setLayer(layer)
line_builder.setStartPoint(lc.geo.Coordinate(2.0, 3.0))
line_builder.setEndPoint(lc.geo.Coordinate(12.0, 23.0))
line = line_builder.build()
entity_builder = lc.operation.EntityBuilder.new(document)
entity_builder.appendEntity(line)
entity_builder.execute()
entities = document.entityContainer().asVector(32767)
assert len(entities) == 1 and isinstance(entities[0], lc.entity.Line), \
    f"entity insertion failed: {len(entities)} entities"
assert undo_manager.canUndo(), "document mutation was not recorded for undo"
undo_manager.undo()
assert len(document.entityContainer().asVector(32767)) == 0, "undo did not remove the line"
assert undo_manager.canRedo(), "undo did not enable redo"
undo_manager.redo()
entities = document.entityContainer().asVector(32767)
assert len(entities) == 1 and isinstance(entities[0], lc.entity.Line), \
    "redo did not restore the line"
)py";

#ifdef LC_PYTHON_SMOKE_WITH_PERSISTENCE
        script += R"py(
lc.persistence.File.save(
    document, dxf_path, lc.persistence.File.Type.LIBDXFRW_DXF_R2000)
assert pathlib.Path(dxf_path).is_file() and pathlib.Path(dxf_path).stat().st_size > 0, \
    "DXF save did not produce a non-empty file"
lc.persistence.File.open(
    round_trip_document, dxf_path, lc.persistence.File.Library.LIBDXFRW)
round_trip_entities = round_trip_document.entityContainer().asVector(32767)
assert len(round_trip_entities) == 1 and isinstance(round_trip_entities[0], lc.entity.Line), \
    f"DXF round-trip returned {len(round_trip_entities)} entities"
assert abs(round_trip_entities[0].start().x() - 2.0) < 1e-7
assert abs(round_trip_entities[0].end().y() - 23.0) < 1e-7
)py";
#endif

        const std::string exerciseError = python.runString(script.c_str(), ns);
        if (!exerciseError.empty()) {
            std::cerr << "Python smoke failed during plugin/document checks:\n"
                      << exerciseError << std::endl;
            return 1;
        }

        std::cout << "Python runtime smoke passed: package="
                  << packageRoot.toStdString() << ", plugins="
                  << pluginReport.loadedPlugins.size()
                  << ", isolated stdlib/native imports, embedded lc/lcgui, "
                     "document undo/redo";
#ifdef LC_PYTHON_SMOKE_WITH_PERSISTENCE
        if (!QFileInfo::exists(dxfPath)) {
            std::cerr << "\nPython smoke failed: expected DXF artifact is missing\n";
            return 1;
        }
        std::cout << ", DXF round-trip";
#endif
        std::cout << std::endl;
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Python runtime smoke failed: " << error.what() << std::endl;
        return 1;
    }
}

} // namespace python
} // namespace ui
} // namespace lc

#endif // LC_WITH_PYTHONSCRIPT

"""CPU build-option regressions; native build/link proofs are separate evidence."""

import shutil
import itertools
import json
import runpy
import subprocess
import sys
import tempfile
import types
import unittest
from unittest import mock
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
OPTIONS = ROOT / 'lcviewernoqt/cmake/RendererOptions.cmake'


class RendererBuildOptionsTests(unittest.TestCase):
    def configure_options(self, **options):
        cmake = shutil.which('cmake')
        self.assertIsNotNone(cmake, 'CMake is required for renderer build contracts')
        with tempfile.TemporaryDirectory(prefix='renderer-options-') as tmp:
            driver = Path(tmp) / 'options.cmake'
            driver.write_text(
                'include("' + OPTIONS.as_posix() + '")\n'
                'lc_renderer_option(WITH_PYTHONSCRIPT "Python scripting" ON)\n'
                'foreach(flag WITH_QT_UI WITH_LUACMDINTERFACE WITH_UNITTESTS WITH_PYTHONSCRIPT '
                'WITH_LEGACY_OPENGL_RENDERER WITH_RHI_RENDERER '
                'WITH_RENDERING_UNITTESTS WITH_RENDERER_TESTS)\n'
                '  message(STATUS "option:' + '${flag}=${${flag}}' + '")\n'
                'endforeach()\n', encoding='utf-8')
            return subprocess.run(
                [cmake, *['-D' + name + ':BOOL=' + value for name, value in options.items()],
                 '-P', str(driver)], stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                text=True, check=False, timeout=30)

    def succeeds(self, **options):
        result = self.configure_options(**options)
        self.assertEqual(0, result.returncode, result.stdout)
        return result.stdout

    def rejects(self, reason, **options):
        result = self.configure_options(**options)
        self.assertNotEqual(0, result.returncode, result.stdout)
        self.assertIn(reason, result.stdout)

    def test_default_keeps_legacy_and_legacy_tests(self):
        output = self.succeeds(WITH_QT_UI='ON', WITH_LUACMDINTERFACE='ON', WITH_UNITTESTS='ON')
        for flag, value in (('WITH_LEGACY_OPENGL_RENDERER', 'ON'), ('WITH_RHI_RENDERER', 'OFF'),
                            ('WITH_RENDERING_UNITTESTS', 'ON'), ('WITH_RENDERER_TESTS', 'OFF')):
            self.assertIn('option:' + flag + '=' + value, output)

    def test_tests_disabled_have_disabled_defaults(self):
        output = self.succeeds(WITH_UNITTESTS='OFF')
        self.assertIn('option:WITH_RENDERING_UNITTESTS=OFF', output)
        self.assertIn('option:WITH_RENDERER_TESTS=OFF', output)

    def test_cpu_only_defaults_do_not_request_graphics_tests(self):
        output = self.succeeds(WITH_LEGACY_OPENGL_RENDERER='OFF', WITH_UNITTESTS='ON',
                              WITH_QT_UI='OFF', WITH_LUACMDINTERFACE='OFF')
        self.assertIn('option:WITH_RENDERING_UNITTESTS=OFF', output)

    def test_cairo_library_does_not_enable_a_gpu_host(self):
        self.succeeds(WITH_LEGACY_OPENGL_RENDERER='OFF', WITH_CAIRO='ON',
                      WITH_QT_UI='OFF', WITH_LUACMDINTERFACE='OFF', WITH_UNITTESTS='ON')

    def test_unmigrated_hosts_require_legacy(self):
        for host in ('WITH_QT_UI', 'WITH_LUACMDINTERFACE'):
            with self.subTest(host=host):
                self.rejects('current UI/CLI hosts require', WITH_LEGACY_OPENGL_RENDERER='OFF',
                             **{host: 'ON'})

    def test_explicit_or_cached_legacy_tests_are_not_forced_off(self):
        self.rejects('WITH_RENDERING_UNITTESTS requires', WITH_LEGACY_OPENGL_RENDERER='OFF',
                     WITH_UNITTESTS='ON', WITH_RENDERING_UNITTESTS='ON')

    def test_legacy_tests_require_unit_tests(self):
        self.rejects('WITH_RENDERING_UNITTESTS requires', WITH_UNITTESTS='OFF',
                     WITH_RENDERING_UNITTESTS='ON')

    def test_legacy_dxf_comparison_requires_persistence(self):
        self.rejects('WITH_RENDERING_UNITTESTS requires WITH_PERSISTENCE',
            WITH_UNITTESTS='ON', WITH_RENDERING_UNITTESTS='ON', WITH_PERSISTENCE='OFF')

    def test_new_renderer_tests_are_not_legacy_tests(self):
        self.rejects('WITH_RENDERER_TESTS requires', WITH_RENDERER_TESTS='ON',
                     WITH_RHI_RENDERER='OFF', WITH_UNITTESTS='ON')

    def test_rhi_only_enables_separate_tests_without_legacy(self):
        output = self.succeeds(WITH_RHI_RENDERER='ON', WITH_LEGACY_OPENGL_RENDERER='OFF',
                              WITH_UNITTESTS='ON', WITH_QT_UI='OFF', WITH_LUACMDINTERFACE='OFF')
        self.assertIn('option:WITH_RENDERER_TESTS=ON', output)
        self.assertIn('option:WITH_RENDERING_UNITTESTS=OFF', output)

    def test_rhi_tests_require_unit_tests(self):
        self.rejects('WITH_RENDERER_TESTS requires', WITH_RHI_RENDERER='ON',
                     WITH_RENDERER_TESTS='ON', WITH_UNITTESTS='OFF')

    def test_cpu_only_rejects_opengl_example_discovery(self):
        self.rejects('TINYSPLINE_BUILD_EXAMPLES requests OpenGL',
                     WITH_LEGACY_OPENGL_RENDERER='OFF', TINYSPLINE_BUILD_EXAMPLES='ON',
                     WITH_QT_UI='OFF', WITH_LUACMDINTERFACE='OFF')

    def test_all_backend_combinations_keep_independent_test_defaults(self):
        for legacy, rhi in itertools.product(('ON', 'OFF'), repeat=2):
            with self.subTest(legacy=legacy, rhi=rhi):
                output = self.succeeds(WITH_QT_UI='OFF', WITH_LUACMDINTERFACE='OFF',
                    WITH_UNITTESTS='ON', WITH_LEGACY_OPENGL_RENDERER=legacy, WITH_RHI_RENDERER=rhi)
                self.assertIn('option:WITH_RENDERING_UNITTESTS=' + legacy, output)
                self.assertIn('option:WITH_RENDERER_TESTS=' + rhi, output)

    def test_conan_cpu_defaults_are_applied_without_a_preset(self):
        output = self.succeeds(LC_CONAN_WITH_QT_UI='OFF', LC_CONAN_WITH_LUACMDINTERFACE='OFF',
            LC_CONAN_WITH_LEGACY_OPENGL_RENDERER='OFF', LC_CONAN_WITH_UNITTESTS='ON',
            LC_CONAN_WITH_PYTHONSCRIPT='OFF')
        for name in ('WITH_QT_UI', 'WITH_LUACMDINTERFACE', 'WITH_LEGACY_OPENGL_RENDERER', 'WITH_PYTHONSCRIPT'):
            self.assertIn('option:' + name + '=OFF', output)

    def test_every_conan_selection_rejects_a_cmake_override(self):
        defaults = {'WITH_QT_UI': 'OFF', 'WITH_LUACMDINTERFACE': 'OFF', 'WITH_UNITTESTS': 'ON',
            'WITH_PYTHONSCRIPT': 'OFF', 'WITH_LEGACY_OPENGL_RENDERER': 'ON', 'WITH_RHI_RENDERER': 'ON',
            'WITH_RENDERING_UNITTESTS': 'OFF', 'WITH_RENDERER_TESTS': 'ON'}
        flags = dict(defaults, **{'LC_CONAN_' + key: value for key, value in defaults.items()})
        self.succeeds(**flags)
        for name, value in defaults.items():
            with self.subTest(option=name):
                changed = dict(flags, **{name: 'OFF' if value == 'ON' else 'ON'})
                self.rejects("conflicts with Conan's dependency graph", **changed)

    def test_cached_conan_mismatch_requires_regeneration(self):
        with tempfile.TemporaryDirectory(prefix='renderer-cache-') as tmp:
            source = Path(tmp) / 'source'
            source.mkdir()
            (source / 'CMakeLists.txt').write_text(
                'cmake_minimum_required(VERSION 3.15)\nproject(CacheCheck NONE)\n'
                'include("' + OPTIONS.as_posix() + '")\n')
            command = ['cmake', '-S', str(source), '-B', str(Path(tmp) / 'build'),
                '-DWITH_QT_UI=OFF', '-DWITH_LUACMDINTERFACE=OFF', '-DWITH_UNITTESTS=OFF',
                '-DLC_CONAN_WITH_RHI_RENDERER=OFF']
            result = subprocess.run(command, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                text=True, check=False, timeout=30)
            self.assertEqual(0, result.returncode, result.stdout)
            result = subprocess.run([*command, '-DLC_CONAN_WITH_RHI_RENDERER=ON'],
                stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True, check=False, timeout=30)
            self.assertNotEqual(0, result.returncode)
            self.assertIn("conflicts with Conan's dependency graph", result.stdout)

    def test_provider_branch_does_not_replace_host_backend_or_test_options(self):
        root = (ROOT / 'CMakeLists.txt').read_text()
        branch = root.split('if(MSVC AND CMAKE_TOOLCHAIN_FILE AND', 1)[1].split('else()', 1)[0]
        self.assertNotRegex(branch, r'\bset\(WITH_')
        script = (ROOT / 'scripts/windows-install/buildLibrecadAndCreatePackage.bat').read_text()
        self.assertIn('-DWITH_LUACMDINTERFACE=OFF', script)

    def test_windows_root_license_is_installed_and_manifest_owned(self):
        root=(ROOT/'CMakeLists.txt').read_text()
        rule='install(FILES "${CMAKE_SOURCE_DIR}/LICENSE" DESTINATION ".")'
        self.assertIn(rule,root)
        self.assertNotIn('file(COPY ${CMAKE_SOURCE_DIR}/LICENSE',root)
        with tempfile.TemporaryDirectory(prefix='renderer-license-') as tmp:
            source=Path(tmp)/'source';source.mkdir();build=Path(tmp)/'build';stage=Path(tmp)/'stage'
            (source/'LICENSE').write_bytes((ROOT/'LICENSE').read_bytes())
            (source/'CMakeLists.txt').write_text('cmake_minimum_required(VERSION 3.15)\nproject(License NONE)\n'+rule+'\n')
            result=subprocess.run(['cmake','-S',str(source),'-B',str(build),'-DCMAKE_INSTALL_PREFIX='+str(stage)],capture_output=True,text=True,timeout=30)
            self.assertEqual(0,result.returncode,result.stdout+result.stderr);self.assertFalse(stage.exists())
            result=subprocess.run(['cmake','--install',str(build)],capture_output=True,text=True,timeout=30)
            self.assertEqual(0,result.returncode,result.stdout+result.stderr)
            self.assertEqual([str(stage/'LICENSE')],(build/'install_manifest.txt').read_text().splitlines())
            self.assertEqual((source/'LICENSE').read_bytes(),(stage/'LICENSE').read_bytes())

    def test_msvc_ui_source_parallelism_is_target_local_and_bounded(self):
        block=OPTIONS.read_text().split('function(lc_parallel_msvc_compile target)',1)[1].split('endfunction()',1)[0]
        self.assertIn('CMAKE_CXX_COMPILER_ID STREQUAL "MSVC"',block);self.assertIn('^Visual Studio',block)
        self.assertIn('target_compile_options(${target} PRIVATE /MP${_jobs})',block)
        self.assertIn('_jobs GREATER 4',block);self.assertIn('_jobs LESS 1',block);self.assertNotIn('CMAKE_CXX_FLAGS',block)
        self.assertIn('lc_parallel_msvc_compile(lcui_objects)',(ROOT/'lcUI/CMakeLists.txt').read_text())
        self.assertIn('lc_parallel_msvc_compile(lcunittest)',(ROOT/'unittest/CMakeLists.txt').read_text())

    def test_shared_ui_static_data_has_explicit_export_import_ownership(self):
        header=(ROOT/'lcUI/lcuiexport.h').read_text()
        self.assertIn('#ifdef LCUI_BUILDING',header);self.assertIn('Q_DECL_EXPORT',header);self.assertIn('Q_DECL_IMPORT',header)
        self.assertIn('target_compile_definitions(lcui_objects PRIVATE LCUI_BUILDING)',(ROOT/'lcUI/CMakeLists.txt').read_text())
        for path,name in [('dialogs/addlayerdialog.h','AddLayerDialog'),('widgets/layermodel.h','LayerModel'),
                          ('widgets/linepatternpathpart.h','LinePatternPathPart'),('windowmanager.h','WindowManager')]:
            self.assertIn('class LCUI_API '+name,(ROOT/'lcUI'/path).read_text())

    def test_ui_test_helpers_link_production_implementations_instead_of_recompiling_them(self):
        for path,form in [('dialogs/addlayerdialog.h','addlayerdialog'),
                          ('dialogs/addlinepatterndialog.h','addlinepatterndialog'),('widgets/layers.h','layers')]:
            source=(ROOT/'unittest/ui'/path).read_text()
            self.assertIn('#include <lcUI/'+path+'>',source)
            self.assertIn('#include <ui_'+form+'.h>',source)
            self.assertNotIn('.cpp>',source)

    def test_heavy_python_bindings_are_scheduled_first_without_duplicate_sources(self):
        body=(ROOT/'lcUI/CMakeLists.txt').read_text().split('# Phase 3 PR-3.2',1)[1].split(
            'if(WITH_PYTHONSCRIPT)',1)[1].split('endif()',1)[0]
        with tempfile.TemporaryDirectory(prefix='renderer-source-order-') as tmp:
            script=Path(tmp)/'order.cmake'
            script.write_text('set(UI_srcs ordinary.cpp)\n'+body+'\n'
                'if(NOT UI_srcs STREQUAL "python/pyguibridge.cpp;python/pyeventhooks.cpp;'
                'python/pypluginloader.cpp;python/pyruntimesmoke.cpp;ordinary.cpp")\n'
                '  message(FATAL_ERROR "Lost, duplicated or delayed UI sources: ${UI_srcs}")\nendif()\n')
            result=subprocess.run(['cmake','-P',str(script)],capture_output=True,text=True,timeout=30)
            self.assertEqual(0,result.returncode,result.stdout+result.stderr)

    def test_cpu_factory_has_no_opengl_implementation(self):
        cpu = (ROOT / 'lcviewernoqt/painters/createpainter.cpp').read_text()
        legacy = (ROOT / 'lcviewernoqt/painters/opengl/createpainter.cpp').read_text()
        self.assertNotIn('opengl', cpu.lower())
        self.assertIn('createCairoImagePainter', cpu)
        self.assertIn('createOpenGLPainter', legacy)

    def test_direct_legacy_consumers_link_native_apis_explicitly(self):
        for source in ('lcUI/CMakeLists.txt', 'luacmdinterface/CMakeLists.txt', 'unittest/CMakeLists.txt'):
            with self.subTest(source=source):
                text = (ROOT / source).read_text()
                self.assertIn('lcviewer_opengl', text)
                self.assertIn('OpenGL::GL', text)
                self.assertIn('GLEW::GLEW', text)

    def test_legacy_math_headers_are_an_explicit_target_dependency(self):
        text = (ROOT / 'lcviewernoqt/CMakeLists.txt').read_text().split('if(WITH_LEGACY_OPENGL_RENDERER)', 1)[1]
        self.assertIn('find_package(glm CONFIG REQUIRED)', text)
        self.assertIn('glm::glm', text)

    def test_python_runtime_process_checks_do_not_require_qt(self):
        text = (ROOT / 'unittest/python/python_runtime_test.cpp').read_text()
        self.assertNotRegex(text, r'#include\s*[<"]Q[A-Z]')
        self.assertIn('subprocess.run([executable,', text)
        self.assertIn('timeout=15', text)

    def test_python_dxf_case_is_conditional_not_the_other_callback_cases(self):
        text = (ROOT / 'unittest/scripting/customentitymanager_test.cpp').read_text()
        guarded = text.split('#if LC_USE_PERSISTENCE', 1)[1].split('#endif // LC_USE_PERSISTENCE', 1)[0]
        self.assertIn('TEST(CustomEntityDxfRoundTrip,', guarded)
        self.assertNotIn('TEST(ScriptCustomEntity,', guarded)
        self.assertNotIn('TEST(LuaCustomEntityManager,', guarded)
        self.assertIn('TEST(ScriptCustomEntityLuaRegistration,', text.split('#endif // LC_USE_PERSISTENCE', 1)[1])

    def test_macos_app_runtime_packaging_requires_the_app_target(self):
        text = (ROOT / 'CMakeLists.txt').read_text()
        self.assertIn('if(APPLE AND WITH_PYTHONSCRIPT AND TARGET librecad AND TARGET Python::Python)', text)

    def test_disabled_persistence_does_not_require_the_dxf_sdk(self):
        for name in ('CMakeLists.txt', 'unittest/CMakeLists.txt'):
            with self.subTest(source=name):
                self.assertRegex((ROOT / name).read_text(),
                    r'if\(WITH_PERSISTENCE\)\s+find_package\(libdxfrw REQUIRED\)')

    def test_current_ui_requires_persistence_without_forcing_it_on(self):
        self.rejects('current Qt UI requires WITH_PERSISTENCE',
            WITH_QT_UI='ON', WITH_PERSISTENCE='OFF', WITH_RENDERING_UNITTESTS='OFF')
        self.succeeds(WITH_QT_UI='ON', WITH_PERSISTENCE='ON')
        self.succeeds(WITH_QT_UI='OFF', WITH_LUACMDINTERFACE='ON', WITH_PERSISTENCE='OFF',
            WITH_RENDERING_UNITTESTS='OFF')

    def test_required_options_module_is_visible_to_git(self):
        result = subprocess.run(['git', 'check-ignore', '--no-index', str(OPTIONS)], cwd=ROOT,
                                stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, timeout=30)
        self.assertEqual(1, result.returncode, result.stdout + result.stderr)

    def test_build_metadata_is_generated_outside_the_source(self):
        with tempfile.TemporaryDirectory(prefix='renderer metadata space ') as tmp:
            source = Path(tmp) / 'source'
            source.mkdir()
            shutil.copy2(ROOT / 'lckernel/build_constants.cpp.in', source)
            output = Path(tmp) / 'build' / 'build_constants.cpp'
            result = subprocess.run(
                ['cmake', '-DDIR=' + str(source), '-DOUTPUT_FILE=' + str(output),
                 '-P', str(ROOT / 'lckernel/cmake/SetBuildInfo.cmake')],
                stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True, timeout=30)
            self.assertEqual(0, result.returncode, result.stdout)
            self.assertTrue(output.is_file())
            self.assertIn('BUILD_INFO = "Release"', output.read_text())
            self.assertFalse((source / 'build_constants.cpp').exists())

    def test_build_metadata_rejects_missing_or_relative_output(self):
        for value in (None, 'relative.cpp'):
            with self.subTest(value=value), tempfile.TemporaryDirectory() as tmp:
                args = ['cmake', '-DDIR=' + str(ROOT / 'lckernel')]
                if value is not None:
                    args.append('-DOUTPUT_FILE=' + value)
                result = subprocess.run(
                    [*args, '-P', str(ROOT / 'lckernel/cmake/SetBuildInfo.cmake')], cwd=tmp,
                    stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True, timeout=30)
                self.assertNotEqual(0, result.returncode, result.stdout)
                self.assertIn('absolute build-tree OUTPUT_FILE', result.stdout)


class UiObjectBuildTests(unittest.TestCase):
    """Build the actual UI target declarations with small dependency fixtures.

    This checks CMake topology/usage requirements, not native Qt UI behavior.
    The full host build is separate evidence.
    """

    def test_ui_objects_are_shared_with_optional_tests_and_python(self):
        declarations = (ROOT / 'lcUI/CMakeLists.txt').read_text()
        declarations = 'add_library(lcui_objects OBJECT' + declarations.split(
            'add_library(lcui_objects OBJECT', 1)[1].split(
            'if(WITH_PYTHONSCRIPT AND WITH_PERSISTENCE)', 1)[0]
        for tests, python in itertools.product((False, True), repeat=2):
            with self.subTest(tests=tests, python=python), tempfile.TemporaryDirectory(
                    prefix='renderer-ui-objects-') as tmp:
                source = Path(tmp) / 'source'
                build = Path(tmp) / 'build'
                source.mkdir()
                (source / 'res').mkdir()
                (source / 'res/librecad.rc').write_text('')
                (source / 'main.cpp').write_text(
                    'int ui_value();\nint main() { return ui_value() == 17 ? 0 : 1; }\n')
                (source / 'ui.cpp').write_text(
                    '#include <type_traits>\n'
                    '#ifndef UI_DEPENDENCY\n#error Missing compile usage requirement\n#endif\n'
                    '#if EXPECT_PYTHON\n'
                    '#ifndef LC_WITH_PYTHONSCRIPT\n#error Missing Python usage requirement\n#endif\n'
                    '#include "python-ready.h"\n'
                    '#elif defined(LC_WITH_PYTHONSCRIPT)\n#error Python leaked into OFF\n#endif\n'
                    'static_assert(std::is_same_v<int, int>, "C++17 dependency");\n'
                    'int ui_value() { return UI_DEPENDENCY; }\n')
                (source / 'CMakeLists.txt').write_text(
                    'cmake_minimum_required(VERSION 3.15)\nproject(UiObjects LANGUAGES CXX)\n'
                    'function(lc_parallel_msvc_compile target)'+OPTIONS.read_text().split(
                        'function(lc_parallel_msvc_compile target)',1)[1].split('endfunction()',1)[0]+'endfunction()\n'
                    'set(CMAKE_CXX_STANDARD 14)\nset(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)\n'
                    'set(CMAKE_EXPORT_COMPILE_COMMANDS ON)\n'
                    'add_library(ui_dependencies INTERFACE)\n'
                    'target_compile_features(ui_dependencies INTERFACE cxx_std_17)\n'
                    'target_compile_definitions(ui_dependencies INTERFACE UI_DEPENDENCY=17)\n'
                    'target_include_directories(ui_dependencies INTERFACE "${CMAKE_BINARY_DIR}")\n'
                    'add_compile_definitions(EXPECT_PYTHON=$<BOOL:${WITH_PYTHONSCRIPT}>)\n'
                    'if(WITH_PYTHONSCRIPT)\n'
                    '  target_compile_definitions(ui_dependencies INTERFACE LC_WITH_PYTHONSCRIPT)\n'
                    '  add_custom_target(lcuipy_build_tree COMMAND "${CMAKE_COMMAND}" -E touch\n'
                    '      "${CMAKE_BINARY_DIR}/python-ready.h")\n'
                    'endif()\nset(UI_srcs main.cpp ui.cpp)\n'
                    'set(EXTRA_LIBS ui_dependencies)\n' + declarations +
                    'foreach(consumer lcui librecad)\n'
                    '  if(TARGET ${consumer})\n'
                    '    foreach(generator AUTOMOC AUTOUIC AUTORCC)\n'
                    '      get_target_property(value ${consumer} ${generator})\n'
                    '      if(NOT value STREQUAL "OFF")\n'
                    '        message(FATAL_ERROR "Redundant consumer ${generator}")\n'
                    '      endif()\n'
                    '    endforeach()\n'
                    '  endif()\n'
                    'endforeach()\n')
                query = build / '.cmake/api/v1/query'
                query.mkdir(parents=True)
                (query / 'codemodel-v2').touch()
                commands = [
                    ['cmake', '-S', str(source), '-B', str(build),
                     '-DWITH_UNITTESTS=' + ('ON' if tests else 'OFF'),
                     '-DWITH_PYTHONSCRIPT=' + ('ON' if python else 'OFF')],
                    ['cmake', '--build', str(build), '--parallel', '2'],
                ]
                for command in commands:
                    result = subprocess.run(command, stdout=subprocess.PIPE,
                        stderr=subprocess.STDOUT, text=True, timeout=60)
                    self.assertEqual(0, result.returncode, result.stdout)
                reply = build / '.cmake/api/v1/reply'
                index = json.loads(next(reply.glob('index-*.json')).read_text())
                model = json.loads((reply / index['reply']['codemodel-v2']['jsonFile']).read_text())
                targets = {item['name']: json.loads((reply / item['jsonFile']).read_text())
                           for item in model['configurations'][0]['targets']}
                self.assertEqual('OBJECT_LIBRARY', targets['lcui_objects']['type'])
                self.assertEqual(tests, 'lcui' in targets)
                for name in ('librecad', 'lcui') if tests else ('librecad',):
                    target = targets[name]
                    self.assertNotIn(name + '_autogen', targets)
                    self.assertIn('lcui_objects', {dep['id'].split('::', 1)[0]
                                                for dep in target['dependencies']})
                    self.assertFalse(any(Path(item['path']).name in ('main.cpp', 'ui.cpp')
                                         for item in target.get('sources', [])))
                if (build / 'compile_commands.json').exists():
                    commands = json.loads((build / 'compile_commands.json').read_text())
                    self.assertEqual(2, len(commands))
                    self.assertTrue(all('lcui_objects.dir' in item['command'] for item in commands))
                executable = build / ('librecad.exe' if sys.platform == 'win32' else 'librecad')
                if not executable.exists():
                    executable = build / 'Debug' / executable.name
                result = subprocess.run([str(executable)], stdout=subprocess.PIPE,
                    stderr=subprocess.STDOUT, text=True, timeout=15)
                self.assertEqual(0, result.returncode, result.stdout)


class ConanRendererRecipeTests(unittest.TestCase):
    """Exercise recipe logic without installing Conan or resolving dependencies."""
    @classmethod
    def setUpClass(cls):
        class InvalidConfiguration(Exception):
            pass
        class Toolchain:
            instances = []
            def __init__(self, recipe):
                self.variables = {}
                self.generated = False
                self.instances.append(self)
            def generate(self):
                self.generated = True
        modules = {name: types.ModuleType(name) for name in
            ('conan', 'conan.errors', 'conan.tools', 'conan.tools.cmake')}
        modules['conan'].ConanFile = object
        modules['conan.errors'].ConanInvalidConfiguration = InvalidConfiguration
        modules['conan.tools.cmake'].CMakeToolchain = Toolchain
        with mock.patch.dict(sys.modules, modules):
            cls.recipe_class = runpy.run_path(str(ROOT / 'conanfile.py'))['LibreCAD3']
        cls.invalid = InvalidConfiguration
        cls.toolchain = Toolchain

    def recipe(self, **options):
        recipe = self.recipe_class()
        recipe.options = types.SimpleNamespace(**dict(recipe.default_options, **options))
        recipe.dependencies_requested = []
        recipe.requires = recipe.dependencies_requested.append
        return recipe

    def requirements(self, **options):
        recipe = self.recipe(**options)
        recipe.validate()
        recipe.requirements()
        return {p.split('/')[0] for p in recipe.dependencies_requested}

    def test_windows_defaults_stay_legacy_without_unused_cli_dependencies(self):
        self.assertEqual({'boost', 'eigen', 'freetype', 'glew', 'glm', 'gtest', 'lua', 'rapidjson'},
            self.requirements())

    def test_cpu_and_rhi_have_no_legacy_dependency_requirements(self):
        for rhi in (False, True):
            with self.subTest(rhi=rhi):
                self.assertEqual({'boost', 'eigen', 'gtest', 'lua', 'rapidjson'}, self.requirements(
                    with_qt_ui=False, with_legacy_opengl_renderer=False, with_rhi_renderer=rhi,
                    with_renderer_tests=rhi))

    def test_cli_and_legacy_tests_select_glfw_independently(self):
        self.assertIn('libcurl', self.requirements(with_luacmdinterface=True))
        self.assertIn('glfw', self.requirements(with_luacmdinterface=True))
        self.assertIn('glfw', self.requirements(with_rendering_unittests=True))
        self.assertNotIn('libcurl', self.requirements(with_rendering_unittests=True))

    def test_disabled_unit_tests_do_not_require_gtest(self):
        self.assertNotIn('gtest', self.requirements(with_unittests=False))

    def test_test_dependency_is_pinned_without_raising_application_cppstd(self):
        recipe=self.recipe();recipe.requirements()
        self.assertIn('gtest/1.18.0',recipe.dependencies_requested)
        command=(ROOT/'scripts/windows-install/createConanDirAndInstallDependencies.bat').read_text()
        self.assertIn('-s "gtest/*:compiler.cppstd=17"',command)
        self.assertNotIn('-s compiler.cppstd=17',command)

    def test_all_option_combinations_fail_closed(self):
        names = tuple(self.recipe_class.default_options)
        for values in itertools.product((False, True), repeat=len(names)):
            options = dict(zip(names, values))
            o = types.SimpleNamespace(**options)
            valid = ((not (o.with_qt_ui or o.with_luacmdinterface) or o.with_legacy_opengl_renderer)
                and (not o.with_rendering_unittests or (o.with_legacy_opengl_renderer and o.with_unittests))
                and (not o.with_renderer_tests or (o.with_rhi_renderer and o.with_unittests)))
            with self.subTest(options=options):
                if valid:
                    self.recipe(**options).validate()
                else:
                    with self.assertRaises(self.invalid):
                        self.recipe(**options).validate()

    def test_toolchain_records_every_boolean_without_duplicate_generation(self):
        options = dict(self.recipe_class.default_options, with_qt_ui=False,
            with_pythonscript=False, with_rhi_renderer=True, with_renderer_tests=True)
        recipe = self.recipe(**options)
        recipe.generate()
        tc = self.toolchain.instances[-1]
        self.assertTrue(tc.generated)
        self.assertEqual({'LC_CONAN_' + key.upper(): value for key, value in options.items()}, tc.variables)
        self.assertTrue(all(type(value) is bool for value in tc.variables.values()))
        self.assertNotIn('CMakeToolchain', recipe.generators)


if __name__ == '__main__':
    unittest.main()

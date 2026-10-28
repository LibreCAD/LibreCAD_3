#include <gtest/gtest.h>

#include <lcpython.h>
#include <pybind11/embed.h>
#include <python_runtime_config.h>

#include <string>

namespace py = pybind11;

extern std::string lcUnitTestExecutablePath;

TEST(PythonRuntime, IsolatedProviderIgnoresAmbientPaths) {
    EXPECT_NO_THROW(lc::python::PythonInit::configureForApplication());
    EXPECT_NO_THROW(lc::python::PythonInit::initialize());

    py::gil_scoped_acquire gil;
    py::exec(R"py(
import json
import lc
import sys

assert sys.flags.isolated == 1
assert sys.flags.ignore_environment == 1
assert sys.flags.utf8_mode == 1
assert '__librecad_python_poison__' not in sys.prefix
assert all('__librecad_python_poison__' not in path for path in sys.path)
assert json.dumps({'ready': True}) == '{"ready": true}'
)py");
}

TEST(PythonRuntime, RuntimeConfigurationIsValidatedInFreshProcesses) {
    ASSERT_FALSE(lcUnitTestExecutablePath.empty());
    py::gil_scoped_acquire gil;
    py::dict context;
    context["executable"] = lcUnitTestExecutablePath;
    context["staged_stdlib_relative"] = LC_PYTHON_STAGED_STDLIB_REL;
    // Reuse the selected CPython's portable process/filesystem implementation.
    // This runtime test must also build when Qt/UI and every renderer are OFF.
    py::exec(R"py(
import os
from pathlib import Path
import subprocess
import tempfile

environment = dict(os.environ)
environment.pop('PYTHONHOME', None)
environment.pop('PYTHONPATH', None)

with tempfile.TemporaryDirectory(prefix='LibreCAD runtime ') as temporary:
    def app(name, staged):
        prefix = Path(temporary) / name
        binary = prefix / 'bin'
        package = prefix / 'share/librecad/lcUIPy'
        binary.mkdir(parents=True)
        package.mkdir(parents=True)
        (package / '__init__.py').write_text('# runtime-selection test fixture\n', encoding='utf-8')
        if staged:
            stdlib = prefix / 'share/librecad/python' / staged_stdlib_relative
            stdlib.mkdir(parents=True)
            (stdlib / 'os.py').write_text('# runtime-selection test fixture\n', encoding='utf-8')
        return str(binary)

    missing = app('missing-runtime', False)
    first = app('first-staged-runtime', True)
    second = app('second-staged-runtime', True)

    def probe(mode, arguments, expected, diagnostic=None):
        result = subprocess.run([executable, '--python-runtime-probe', mode, *arguments],
            env=environment, stdin=subprocess.DEVNULL, capture_output=True,
            encoding='utf-8', errors='replace', timeout=15, check=False)
        assert result.returncode == expected, (mode, result.returncode, result.stdout, result.stderr)
        if diagnostic is not None:
            assert diagnostic in result.stderr, (mode, result.stderr)

    probe('missing', [missing], 2, 'staged CPython runtime is missing')
    probe('repeat', [first], 0)
    probe('conflict', [first, second], 2, 'conflicting Python runtime configuration')
    probe('late', [first], 2, 'must be selected before initialization')
)py", context, context);
}

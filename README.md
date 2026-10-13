# LibreCAD 3

[![Build](https://github.com/LibreCAD/LibreCAD_3/actions/workflows/main.yml/badge.svg?branch=master)](https://github.com/LibreCAD/LibreCAD_3/actions/workflows/main.yml)

LibreCAD 3 is the next-generation, C++/Qt implementation of LibreCAD. It
provides a desktop CAD application, DXF persistence, Lua scripting, and
optional Python scripting.

> LibreCAD 3 is under active development. For stable releases and assets, see
> the [releases page](https://github.com/LibreCAD/LibreCAD_3/releases).

## Requirements

- CMake 3.28 or newer
- A C++17 compiler
- Qt 6, including the Base, Tools, SVG, and OpenGL development packages
- Git submodules

The project also uses Boost, Eigen3, Lua 5.3+, GLEW, GLFW, FreeType,
RapidJSON, and several platform libraries. The supported CI dependency list is
maintained in
[`scripts/ubuntu-install/installDependenciesAndBuildRepo.sh`](scripts/ubuntu-install/installDependenciesAndBuildRepo.sh).
On Debian or Ubuntu, use that script as the authoritative package reference.

## Build from source

Clone recursively so the bundled dependencies are available:

```sh
git clone --recurse-submodules https://github.com/LibreCAD/LibreCAD_3.git
cd LibreCAD_3
```

For an existing checkout, initialize the submodules first:

```sh
git submodule update --init --recursive
```

Configure and build:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

The application is written to `build/bin`:

```sh
./build/bin/librecad
```

If CMake cannot find a manually installed Qt, pass its installation prefix:

```sh
cmake -S . -B build -DCMAKE_PREFIX_PATH=/path/to/Qt/6.x/<platform>
```

### Ubuntu

The CI build uses Ubuntu 26.04 and Qt 6. To reproduce its dependency install,
run the maintained script from a checkout:

```sh
./scripts/ubuntu-install/installDependenciesAndBuildRepo.sh
```

It installs packages with `sudo`, builds the project, runs the core test suites,
and stages an AppDir for packaging. For day-to-day development, install the
listed packages once and use the CMake commands above.

### Windows

Windows builds use Conan for third-party dependencies and Qt 6. The CI scripts
are the supported reference:

1. Run [`scripts/windows-install/createConanDirAndInstallDependencies.bat`](scripts/windows-install/createConanDirAndInstallDependencies.bat).
2. Configure with the generated `conan_toolchain.cmake` and a Qt 6
   `CMAKE_PREFIX_PATH`.
3. Build with `cmake --build build --config Release`.

The complete packaging workflow is in
[`scripts/windows-install/buildLibrecadAndCreatePackage.bat`](scripts/windows-install/buildLibrecadAndCreatePackage.bat).

## Configuration

Common CMake options are:

| Option | Default | Purpose |
| --- | --- | --- |
| `WITH_QT_UI` | `ON` | Build the Qt desktop application. |
| `WITH_PERSISTENCE` | `ON` | Build DXF/DWG persistence support. |
| `WITH_DWG_IMPORT` | `OFF` | Enable experimental DWG reading through libdxfrw. |
| `WITH_PYTHONSCRIPT` | `ON` | Enable embedded Python scripting. Conan/Windows builds disable it until Python bundling is available. |
| `WITH_LUACMDINTERFACE` | `ON` | Build the Lua command-line interface. |
| `WITH_UNITTESTS` | `ON` | Build `lcunittest`. |
| `WITH_RENDERING_UNITTESTS` | `ON` | Build rendering tests that require a graphics environment. |
| `WITH_DOCUMENTATION` | `OFF` | Build documentation. |

For example, a headless core build can be configured with:

```sh
cmake -S . -B build-core \
  -DWITH_QT_UI=OFF -DWITH_PYTHONSCRIPT=OFF \
  -DWITH_LUACMDINTERFACE=OFF -DWITH_RENDERING_UNITTESTS=OFF
```

## Tests

When unit tests are enabled, run the test executable from the build directory:

```sh
./build/bin/lcunittest
```

On a headless Linux machine, use a virtual X server for UI-dependent tests:

```sh
xvfb-run -a ./build/bin/lcunittest
```

The Linux CI workflow runs the stable core suites as blocking checks and runs
the remaining UI suites for visibility. See
[`.github/workflows/main.yml`](.github/workflows/main.yml) for the current CI
matrix and packaging jobs.

## Scripting

LibreCAD 3 embeds Lua and Python. Scripts, plugins, and custom entities share
the event bus, operation registry, and kernel API. See
[`doxygen/Scripting.md`](doxygen/Scripting.md) for architecture, API parity,
and plugin authoring notes.

## Contributing

Please open an issue or pull request on
[GitHub](https://github.com/LibreCAD/LibreCAD_3). Keep submodules pinned and
include build or test evidence appropriate to the change.

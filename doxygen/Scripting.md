Scripting (Lua + Python)
========================

LibreCAD 3 embeds two scripting languages simultaneously: Lua (historical,
retained) and Python. Both use the C++ neutral scripting layer (`lcscripting`)
for callback values, event dispatch, and custom-entity hooks. Language-specific
adapters still handle registration and value conversion; sharing kernel types
does not by itself guarantee API or behavioral parity.

For quick-start authoring examples see `lcUILua/` (Lua) and `lcUIPy/`
(Python).  This document is the architecture reference.

Architecture
------------

```
┌─────────────────────────────────────────────────────────────┐
│                        User script                          │
│   ┌───────────────────┐     ┌──────────────────────────┐    │
│   │  Lua sources      │     │  Python sources          │    │
│   │  lcUILua/*.lua    │     │  lcUIPy/**/*.py          │    │
│   └────────┬──────────┘     └────────┬─────────────────┘    │
│            │                          │                     │
│   ┌────────▼──────────┐     ┌────────▼─────────────────┐    │
│   │  kaguya (Lua)     │     │  pybind11 (Python)       │    │
│   └────────┬──────────┘     └────────┬─────────────────┘    │
│            │                          │                     │
│   ┌────────▼──────────┐     ┌────────▼─────────────────┐    │
│   │ lcadluascript     │     │ lcadpythonscript         │    │
│   │  - lua callback   │     │  - python callback       │    │
│   │  - lua object     │     │  - python object         │    │
│   └────────┬──────────┘     └────────┬─────────────────┘    │
│            └────────────┬─────────────┘                     │
│                         │                                   │
│               ┌─────────▼───────────┐                       │
│               │    lcscripting      │                       │
│               │  (neutral layer):   │                       │
│               │  ScriptCallback,    │                       │
│               │  ScriptObject,      │                       │
│               │  ScriptValue,       │                       │
│               │  EventBus           │                       │
│               └─────────┬───────────┘                       │
│                         │                                   │
│               ┌─────────▼───────────┐                       │
│               │      lckernel       │                       │
│               │  (entities, ops,    │                       │
│               │   builders, geo)    │                       │
│               └─────────────────────┘                       │
└─────────────────────────────────────────────────────────────┘
```

### Capability and validation status

This is the bounded Python surface maintained by the current implementation;
“implemented” means there is a binding and focused local regression coverage,
not that every C++ method is exposed or every platform artifact has passed.

| Area | Python entry point | Current evidence | Status |
|---|---|---|---|
| Namespace registration | `lc.entity`, `lc.builder`, `lc.operation`, `lc.geo`, `lc.meta`, `lc.storage`, `lc.event`, `lc.maths`, conditional `lc.persistence`, and `lcgui` | `PythonFixture.EmbeddedModuleHasAllSubmodules`; persistence namespace/enumeration tests | Tested; persistence is configuration-dependent, and exhaustive Lua/API parity is incomplete. |
| Geometry and entities | `lc.geo`, `lc.entity` | `CoordinateArithmeticEqualsCpp`, `ArcCirclePropertiesMatchCpp`, `PointMultipleInheritanceRoundTrip` | Tested; exhaustive type/method parity and non-macOS builds remain incomplete. |
| Builders, documents, and undo | `lc.builder`, `lc.storage` | `LineBuilderRoundTrip`, `UndoManagerBasics`, `EntityBuilderAndLineBuilderRuntimeCalls`, and Lua/Python canonical parity with undo/redo | Tested for these flows; broader operation and lifetime combinations remain incomplete. |
| Storage and persistence | `lc.storage`, conditional `lc.persistence` | `PersistenceTest.DxfRoundTrip`, `PersistenceTest.SaveFormatMatrixRoundTrip` (14 DXF/DXB variants) | Tested locally on macOS arm64; Windows/Linux/provider acceptance remains incomplete. |
| Operations and operation metadata | `lc.operation`, `@lc.register_operation` | `RegisterOperationDecoratorAddsToRegistry`, `PythonOperationOptionsWireDefaultAndNamedActions`, operation resolver and constructor/init-failure regressions | Tested for the documented registration/dispatch paths; full Lua metadata and lifecycle parity remains incomplete. |
| Events and callback values | `lc.event`, `ScriptCallback` adapters | `ScriptValueRoundTrip.*`, `EventBus.*`, two-window/nested/failure-cleanup UI regressions | Tested for declared conversion and dispatch cases on macOS arm64; full callback/platform parity remains incomplete. |
| GUI and widgets | `lcgui` | `PyGuiBridgeTest.*`, `MWindowTest.PyGuiBridgeRejectsWorkerThreadAccess` | Tested for the current bound surface and Qt-thread rejection; retained raw Qt-widget handle safety remains incomplete. |
| Custom entities | `lc.register_plugin`, `lc.entity.CustomEntity` | Typed callback/coexistence/error regressions and `CustomEntityDxfRoundTrip.PluginReconstructsEntityFromDxfAppData` | Tested for Python dispatch and DXF reconstruction locally; cross-platform provider/CI acceptance remains incomplete. |
| ScriptDock and bundled plugins | `lcUIPy` package and `ScriptDock` | `ScriptDockTest.PythonCreatesLine`, event-routing and plugin-idempotency tests; packaged runtime smoke loads two plugins | Tested for these examples and lifecycle cases; complete plugin/artifact coverage remains incomplete. |
| Runtime and distribution | Embedded CPython selected by CMake | `PythonRuntime.IsolatedProviderIgnoresAmbientPaths`; macOS arm64 relocated-bundle smoke; Ubuntu ARM64 installed-prefix smoke; Ubuntu 26.04 x86_64-emulated strict AppDir smoke | Tested locally on macOS arm64 and Ubuntu ARM64; strict bundled-libpython AppDir smoke passes under x86_64 emulation. Native/hosted Linux package closure, Windows, macOS x86_64, and final artifact validation remain incomplete. |
| Deliberately unsupported runtime features | In-application `pip`, Tk/IDLE, multiprocessing, and script sandboxing | Runtime contract below | Deliberately unsupported; plugins are trusted code and staged site-packages are not provided. |

Python call shapes are binding-specific: construct ordinary bound values with
their registered class signature (for example `lc.geo.Coordinate(...)` and
`lc.builder.LineBuilder()`), while selected operations use explicit factories
such as `lc.operation.EntityBuilder.new(document)`. Do not infer that Lua's
`Class.new(...)` or colon syntax transfers mechanically. At `ScriptValue`
boundaries, Python dictionaries require string keys; non-empty lists/tuples
must be homogeneous Coordinate or entity collections. Empty or mixed
sequences, out-of-range integers, and arbitrary objects are rejected with
diagnostics; `None` is a valid nil value.

No claim of universal support or complete Lua parity should be inferred from
this table. The CI/platform matrix and remaining acceptance work are tracked in
`PYTHON_BINDING_IMPLEMENTATION_PLAN.md`.

Core namespace organization intentionally parallels Lua: Python exposes
`lc.entity.*`, `lc.builder.*`, `lc.operation.*`, `lc.geo.*`, `lc.meta.*`,
`lc.storage.*`, `lc.event.*`, and `lc.maths.*`; persistence-enabled builds also
expose `lc.persistence.*`. The GUI surface is `gui` in Lua and `lcgui` in
Python.

### Runtime selection and packaging

CMake's `find_package(Python ... Interpreter Development.Embed)` selection is
the source of truth for the executable, headers/import library, runtime ABI,
standard library, and dynamic extension modules. Configuration writes
`python-runtime-inventory.txt` in the build directory for CI/evidence; it is a
diagnostic artifact and is not installed. Application/CLI startup configures
the embedded interpreter from that selected provider, ignores ambient
`PYTHONHOME`/`PYTHONPATH`, and prefers an app-relative staged runtime when one
is present. System/developer builds may use the configure-selected provider.

Staged runtime trees intentionally exclude `site-packages` and `dist-packages`:
third-party Python dependencies are not installed implicitly, and plugins are
expected to use the standard library plus LibreCAD's shipped modules. Tk/IDLE,
`pip` inside LibreCAD, multiprocessing, and script sandboxing are not supported
by this embedded-runtime contract. Plugins are trusted code.

Operation contract
------------------

A scripting-defined operation is a class with:

- A `name` attribute (string) — used as the registry key.
- A `command_line` attribute (string OR dict) — the CLI text that runs
  the op.  When a string, the op runs `_init_default` on invocation.
  When a dict, each key is a CLI text; the corresponding value is the
  init-method suffix (e.g. `{ "ARC": "arc_default", "ARCELLIPSE": "arc" }`
  runs `_init_arc_default` and `_init_arc` respectively).
- An `icon` attribute (string, optional) — filename relative to the
  Qt resource `:/icons/` prefix.  Plugin-supplied icons resolve via
  the file-path fallback in `ToolbarButton::changeIcon` (phase 5
  PR-5.4) — `<plugin_dir>/icons/<name>` is tried when the qrc lookup
  misses.
- A `description` attribute (string, optional) — tooltip text.
- A `menu_actions` attribute (dict, optional) — `{ init_method: qt_action_object_name }`
  wires each init method to a Qt menu action by object name.  E.g.
  `{ "default": "actionPoint" }` connects `_init_default` to the
  `actionPoint` QAction (used by Lua's `PointOperations` and Python's
  `PyPointOperations`).
- A `group` attribute (string, optional) — toolbar group under the
  "Creation" tab.  The exact string "Dim" is remapped to "Dimensions"
  as a legacy alias.
- An `_init_<method>` method for each init entry point.  Called by
  `MainWindow::runOperation` after the resolver produces the class
  and `instantiate()` runs the class's `__init__` (Python) or its
  `_init(...)` (Lua).

**Python-specific: do NOT call `super().__init__(...)` inside
`_init_default`** — `runOperation`'s `instantiate()` already ran the
base's `__init__` once.  A second call re-runs `registerEvents()`
(when the base is `CreateOperations`) and silently double-registers
every event listener, producing phantom duplicate entities on every
subsequent click app-wide until process restart.  See
`lcUIPy/create_actions/point_operations.py` for the correct pattern
(set instance state directly), and the regression guard
`CreateOperationsBaseInitNotDoubleCalled` in
`unittest/python/pythonbindings_test.cpp`.

Registration APIs
-----------------

Both languages autoregister operations at import time.  The
registration APIs are Python-only because Python needs a decorator
shape to be idiomatic; Lua uses the global-class-name convention it
always used.

| Purpose                    | Lua                     | Python                                |
|----------------------------|-------------------------|---------------------------------------|
| Register an operation      | (implicit — top-level   | `@lc.register_operation` decorator    |
|                            |  class assignment scans |                                       |
|                            |  the Lua state's global |                                       |
|                            |  table)                 |                                       |
| Register an event listener | `luaInterface:registerEvent(name, fn)` | `lc.event.register(name, fn)` |
| Deregister a listener      | `luaInterface:deleteEvent(name, fn)` | `lc.event.deregister(name, fn)` |
| Register a custom-entity plugin | `registerPlugin(name, fn)` (global) | `lc.register_plugin(name, fn)` (phase 6) |

Event list
----------

The C++-side event names + payload shapes (source: `mainwindow.cpp`
`trigger*` slot family).  Every payload is a language-neutral `Map`
that materializes into a Lua table or a Python dict per callback.

| Event                | Payload keys           | Source slot                    |
|----------------------|------------------------|--------------------------------|
| `mousePress`         | `widget: CadMdiChild*` | `MainWindow::triggerMousePressed` |
| `mouseRelease`       | `widget: CadMdiChild*` | `MainWindow::triggerMouseReleased` |
| `mouseMove`          | `widget: CadMdiChild*` | `MainWindow::triggerMouseMoved` |
| `selectionChanged`   | `widget: CadMdiChild*` | `MainWindow::triggerSelectionChanged` |
| `keyPressed`         | `key: int`             | `MainWindow::triggerKeyPressed` |
| `point`              | `position: Coordinate` | `MainWindow::triggerCoordinateEntered` |
| `relativePoint`      | `position: Coordinate` | `MainWindow::triggerRelativeCoordinateEntered` |
| `number`             | `number: double`       | `MainWindow::triggerNumberEntered` |
| `text`               | `text: string`         | `MainWindow::triggerTextEntered` |
| `command`            | `command: string`      | `MainWindow::triggerCommandEntered` |
| `operationFinished`  | (empty)                | `MainWindow::operationFinished` |
| `finishOperation`    | (empty)                | `CliCommand::finishOperation` |

Note: `number` arrives as a Python **float** (not int) — even for
integer-typed input.  `CliCommand::enterNumber(double)` →
`ScriptValue(Kind::Double)` → `toPyLocked`'s `py::float_()`.  Type-check
with `isinstance(n, (int, float))` and coerce via `int(n)` where an
integer is required.

Plugin layout
-------------

**Lua plugins** live under `lcUILua/plugins/<name>/plugin.lua`, loaded
by `PluginManager::loadPlugins()` via a directory walk driven by the
`plugin_path` global set from `lcUILua/CMakeLists.txt`.  The plugin is
`dofile`'d in the shared Lua state, so top-level class assignments
land in the operation registry immediately.

**Python plugins** live under `lcUIPy/plugins/<name>/plugin.py`,
loaded directly from the resolved application package directory in
`LuaInterface::initLua` (no generated `path.py` file is required).
Each plugin gets its own `py::dict` namespace with `import lc; import lcgui as gui`
pre-loaded + a `LC_interface = "gui"` global; `@lc.register_operation`
inside populates the process-global `lc.operation_registry` dict.
Load order is BEFORE `OperationLoader::loadPythonOperations` so the
CLI/menu/toolbar wiring walk sees plugin registrations (reverse of the Lua
order because Python's decorator model needs registration before the wiring
walk).

The `plugin.py` scan is bounded to the plugin subdir only — it does
NOT recurse into sub-subdirs.  For a plugin needing multiple files,
use standard Python packaging (`__init__.py` + siblings under the
same subdir).

Custom-entity round-trip (phase 6 PR-6.1)
------------------------------------------

Custom entities (script-defined entity classes with a persistence
hook) are stored in DXF as app-data led by `APP_NAME_CODE 102` /
`"LibreCAD"`, then codes 410 (plugin name) / 411 (entity name) and
470/471 param pairs.  Both languages register through the same
process-global `CustomEntityManager`, so a DXF written by a Lua-defined
custom entity reopens correctly when only a Python plugin is loaded
(assuming the plugin name matches — that's the routing key).

The multi-window bug (`~LuaInterface` calls
`LuaCustomEntityManager::removePlugins()` — closing ONE window
deregistered plugins for ALL windows) is fixed in phase 6 PR-6.1 by
making plugin registration process-scoped.

Deliberate API divergences
--------------------------

Some registration APIs are Python-only because the language shape
demands it:

- `@lc.register_operation` — decorator idiom; Lua uses the global-
  assignment convention.
- `lc.event.register(name, listener)` — Python needs a free function
  to route through the process-global hook installed at `MainWindow`
  construction time.  Lua's equivalent is `luaInterface:registerEvent(name, fn)`.
- `lc.register_plugin(name, fn)` — Python needs an explicit call to
  the process-global manager; Lua's `registerPlugin(name, fn)` is a
  global function in the Lua state.

The method-style binding surface (`b:appendEntity(l)` in Lua,
`b.appendEntity(l)` in Python) is deliberate 1:1 name parity — a
non-pythonic choice, but recording the same API surface in both
languages so authoring one translates trivially to the other.  A
Pythonic `append_entity` alias may land in a future phase; for now,
the camelCase preserves the reference-doc §Custom-entity contract and
minimizes maintenance burden across two language-facing bindings.

Related documents
-----------------

- `Structure.md` — module layout including `lcadpythonscript` +
  `lcscripting`.
- `Building.md` — build requirements including python3-dev.
- `README.md` — top-level project overview mentions scripting.
- `lckernel/cad/operations/entityops.h` — inline examples now
  language-neutral (both Lua and Python users read the same pseudocode).

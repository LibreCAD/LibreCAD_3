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
| Geometry and entities | `lc.geo`, `lc.entity`, `lc.meta` | `CoordinateArithmeticEqualsCpp`, `ArcCirclePropertiesMatchCpp`, `PointMultipleInheritanceRoundTrip` | Bound types are exercised by these flows; exhaustive C++ method parity and Windows artifact acceptance remain unverified. |
| Builders and document operations | `lc.builder`, `lc.operation` | `LineBuilderRoundTrip`, `EntityBuilderAndLineBuilderRuntimeCalls`, `OperationPushRemoveConstructedViaNew`, `PythonRectangleCreationIsOneUndoStep`, `PythonRectangleDragIsOneUndoStep` | Tested constructors/factories and representative transaction flows; the API does not expose every C++ operation or constructor. |
| Storage and undo | `lc.storage` (`Document`, `StorageManager`, `EntityContainer`, `UndoManager`) | `UndoManagerBasics`, `EntityBuilderAndLineBuilderRuntimeCalls`, rectangle create/drag undo regressions | Document/entity access and representative undo/redo are tested; broader retained-reference and storage lifecycle combinations remain incomplete. |
| Persistence | `lc.persistence.File`, `lc.persistence.formatVariants` | `PersistenceTest.DxfRoundTrip`, `ImportResultFromPython`, `AllWritableDxfVariantsRoundTripThroughPython`, `CustomEntityDxfRoundTrip.PluginReconstructsEntityFromDxfAppData` | DXF/DXB variants, import diagnostics and custom-entity reconstruction pass the hosted Linux/macOS source filters below; full DWG qualification and Windows artifact acceptance remain open. |
| Operation registration and metadata | `@lc.register_operation`, `lc.operation_registry`; class metadata such as `name`, `command_line`, `icon`, and `description` | `RegisterOperationDecoratorAddsToRegistry`, `PythonGearOperationCreatesScaledOutline`, `PythonPointAndRemoveOperationsUseRealSelection` | Registry behavior and shipped command/action flows are tested; every metadata combination and full Lua metadata parity are not claimed. |
| Events and GUI callbacks | `lc.event`, `lcgui.MainWindow` | Event registration tests, `PyGuiBridgeTest.MainWindowRemainderBindingsCallable`, `PythonEventDrivenOnEventReachesMainWindow` | Representative callbacks and owner-scoped dispatch are tested; complete lifecycle and cross-platform behavior remain open. |
| UI widgets | `lcgui` menu, toolbar, dialog, input, layer, and command APIs | `MenuFamilyConstructAndCall`, `MenuItemConstructAndCall`, `ToolbarWidgetFamilyConstructAndCompose`, `InputWidgetsConstructAndRoundTrip`, `DialogWidgetComposition` | The named widget families have focused coverage; this is not an exposure or validation claim for every Qt/C++ UI class. |
| Custom entities | `lc.register_plugin`, `lc.entity.CustomEntity` | Typed callback/coexistence/error regressions and `CustomEntityDxfRoundTrip.PluginReconstructsEntityFromDxfAppData` | Python dispatch and DXF reconstruction pass hosted Linux x64/macOS arm64 source filters; Windows and complete packaged lifecycle validation remain open. |
| ScriptDock and bundled plugins | `lcUIPy` package and `ScriptDock` | `ScriptDockTest.PythonCreatesLine`, `PythonEventDrivenOnEventReachesMainWindow`; packaged runtime smoke loads two plugins | Shipped examples and the smoke path are exercised; complete plugin lifecycle/packaged coverage remains outstanding. |
| Runtime and distribution | Embedded CPython selected by CMake | `PythonRuntime.IsolatedProviderIgnoresAmbientPaths`; macOS arm64 relocated-bundle smoke | Hosted Linux x64 AppImage/Snap and macOS arm64 relocated-bundle smoke pass; clean Linux runtime, Windows and complete final-artifact acceptance remain open. |

### Python value conversion and construction

Values returned through the language-neutral callback boundary have a narrower
contract than ordinary pybind11 method arguments: `None`, booleans, signed
32-bit integers, floats, strings, and string-keyed dictionaries are supported.
Lists and tuples must be non-empty and homogeneous, containing only
`lc.geo.Coordinate` values or only `lc.entity.CADEntity` values. Empty or mixed
sequences, non-string dictionary keys, out-of-range integers, and arbitrary
Python objects are rejected with diagnostics rather than silently becoming
`None`. Some bound methods accept other Python sequences through pybind11's STL
conversions; that does not widen the callback contract.

Construction also follows the binding rather than Python conventions for every
type. Use `lc.builder.*` for kernel entities whose classes have no direct
constructor. Operation factories exposed as `.new(...)` are not callable as
constructors—for example, use `lc.operation.EntityBuilder.new(document)` and
`lc.operation.Push.new(...)`. `LineBuilder` is directly constructible. In
interactive operations, the host instantiates the Python operation before
calling `_init_default`; do not call `super().__init__(...)` there, since it
would register event listeners twice.

No claim of universal support or complete Lua parity should be inferred from
this table. The CI/platform matrix and remaining acceptance work are tracked in
`PYTHON_BINDING_IMPLEMENTATION_PLAN.md`.

Core namespace organization intentionally parallels Lua: Python exposes
`lc.entity.*`, `lc.builder.*`, `lc.operation.*`, `lc.geo.*`, `lc.meta.*`,
`lc.storage.*`, `lc.event.*`, and `lc.maths.*`; persistence-enabled builds also
expose `lc.persistence.*`. The GUI surface is `gui` in Lua and `lcgui` in
Python.

Hosted checkpoint (2026-10-11): PR candidate `783266c3` on Linux x64 passes the
315-test required core filter, AppDir/final AppImage and installed core26 Snap
smokes; `38514122` on both macOS architectures passes 158 required scripting
tests plus one Lua runtime test, original/relocated/archive-extracted app smoke, and retained
archive/inventory checks. Both archives declare macOS 15.0 as their measured
minimum and are ad-hoc signed. Required tests report zero skips. These are
candidate results, not a claim that the roadmap is complete: clean Linux
runtime validation, Windows/NSIS, dependency qualification/notices and protected
signing still have open gates
in `PYTHON_BINDING_IMPLEMENTATION_PLAN.md`.

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

macOS bundles declare `LSMinimumSystemVersion` from the highest deployment
target in the shipped application, Python, Qt, and other native libraries,
including every Mach-O architecture slice. The packager preserves a higher
explicit application floor and fails if a deployment target cannot be read.
This reports the actual artifact requirement, not compatibility with an older
macOS release. The native arm64/x86_64 workflow retains the signed app archive,
its SHA-256 and minimum OS, the selected-provider inventory, and test/smoke logs.
PR bundles are ad-hoc signed; Developer ID signing and notarization are separate
release requirements.

Linux AppImage CI additionally runs the exact artifact's extraction launcher
in an unprivileged, offline, read-only Ubuntu 26.04 runtime container without
system Python, libpython, or a Python SDK. The bounded smoke still requires
bundled CPython and native extension dependencies; it complements the native
FUSE-launcher check, not older-distribution compatibility. This new gate awaits
hosted execution. The inspected Linux candidate payload requires GLIBC 2.43
and its executable declares x86-64-v3; compatibility with older glibc
distributions or older x86_64 CPU baselines is not claimed.

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
  Qt resource `:/icons/` prefix. When the qrc lookup misses,
  `ToolbarButton::changeIcon` tries `<plugin_dir>/icons/<basename>` for
  plugin-supplied icons.
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
| Register a custom-entity plugin | `registerPlugin(name, fn)` (global) | `lc.register_plugin(name, fn)` |

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

Custom-entity DXF persistence
-----------------------------

Custom entities (script-defined entity classes with a persistence
hook) are stored in DXF as app-data led by `APP_NAME_CODE 102` /
`"LibreCAD"`, then code 410 (plugin name), code 411 (entity name), and
470/471 parameter pairs. End-to-end regressions cover both Python
create/save/reopen/reconstruct and a mixed-language path where Lua writes the
custom entity and a fresh Python-only reconstruction hook reopens it. They
check concrete Insert/storage types, plugin/entity names, parameters, position,
and that reconstruction replaces the placeholder instead of duplicating it.
The tests are `CustomEntityDxfRoundTrip.PluginReconstructsEntityFromDxfAppData`
and `CustomEntityDxfRoundTrip.LuaWriterPythonPluginReconstructsEntity`; both
pass the hosted Linux x64/macOS arm64 required source filters at `783266c3`.
Windows and Snap-specific full regression acceptance remain open.

For Lua code that passes custom storage to shared-block APIs such as
`AddBlock.new` or `CustomEntityBuilder:setDisplayBlock`, create an owning block
with `lc.meta.CustomEntityStorage.create(plugin, entity, coordinate, params)`.
The callable `CustomEntityStorage(...)` constructor produces a value userdata;
it is not the shared pointer those APIs require.

Plugin registration is process-scoped: destroying one `LuaInterface` must not
clear registrations still needed by another window. The lifecycle regression is
`MWindowTest.PluginRegistrationSurvivesLuaInterfaceDestructor`.

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
languages so authoring one translates trivially to the other. A
Pythonic `append_entity` alias may be added later; for now,
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

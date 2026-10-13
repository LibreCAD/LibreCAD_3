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

`lcgui` calls must run on the Qt application thread. Retained Qt-backed
receivers and arguments raise `RuntimeError` after their native object is
destroyed, rather than dereferencing stale pointers. Shared kernel/viewer
objects retain their normal shared ownership. Python-created widgets start
Python-owned; when a bound call gives them a Qt parent, Qt takes over deletion.
Dialogs are parented to their supplied window, as are menus/items created by
the native menu factories. Collect UI wrappers on the application thread, too.
These guards do not make scripts a sandbox or support arbitrary native-code
ownership changes outside the bound API.

Standalone GUI callbacks restore their registering window while running and
restore the caller's outer scope afterward. The receiver's actual Qt window
takes precedence over an ambient script scope. An owner-free widget may bind
once after late adoption; an unattached callback runs with no window. Shared
actions with ambiguous windows fail instead of choosing insertion order.
Native toolbar clones retain the bound window after the original button is
removed, including adoption before the first invocation. Closed bound windows
make callbacks inert. Event subscriptions created by callbacks use that same
owner. This Qt-side policy does not bind process-global entity callbacks to a
window or replace their document/builder context. Entity-picker entries absent
from the drawing do not invent a selection or dereference a missing drawable.

Replacing a drawing finishes its active operation and removes its temporary
preview before adopting fresh storage, undo history, layers, and paper views.
Opening a file imports into a separate candidate first: canceling or failing
without recovered entities preserves the current drawing, filename, undo
history, and active operation. A canceled or failed new-window open does not
register an empty window. Successful adoption notifies observers only after
the drawing and its source path are coherent. Shared documents and previews
may outlive their window; deleted UI and undo observers must not receive their
callbacks. The actual bound replacement/reopen/retained-object regressions
exercise this contract; final platform and packaged acceptance remains tracked
in the implementation plan.

A retained `DocumentCanvas` keeps its original drawing's selection; selection
changes do not notify a replacement view. `selectAll`, `inverseSelection`, and
`removeSelection` remain usable after replacement or window destruction.
`selectPoint` needs the live view's painter to convert its pixel tolerance:
before painting or after detachment it raises `RuntimeError`, without borrowing
another view or retaining a Qt window. Document edits and undo on a retained
drawing likewise must not access a deleted painter cache.

Core namespace organization intentionally parallels Lua: Python exposes
`lc.entity.*`, `lc.builder.*`, `lc.operation.*`, `lc.geo.*`, `lc.meta.*`,
`lc.storage.*`, `lc.event.*`, and `lc.maths.*`; persistence-enabled builds also
expose `lc.persistence.*`. `ImportResult.failures` contains typed, readonly
`ImportFailure` records (`recordKind`, unsigned `handle`, `layer`, `block`,
`reason`) in both languages, distinct from parser diagnostics and unsupported
entity losses. The import-only DWG identity is `File.LIBDXFRW_DWG_IMPORT` (15;
also `File.Type.LIBDXFRW_DWG_IMPORT` in Python), mapping to `dwg.libdxfrw` even
when DWG reading is disabled; inspect `ok` and diagnostics rather than assuming
the enum enables a reader. The GUI surface is `gui` in Lua and `lcgui` in
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

All desktop installers place `lcUIPy`, the staged Python tree, and provider
licenses together under `share/librecad` (macOS bundle assembly relocates this
tree into Resources). Windows keeps its runtime DLL beside the executable in
`bin`; its stdlib/extensions still use the common data layout. This keeps the
runtime resolver and installed-package smoke consistent after relocation.

The shared layout also retains byte-for-byte project and vendored source
licenses under `licenses/source`, preserving their repository-relative paths.
Persistence/libdxfrw, CLI and pybind11 notices follow their build feature flags.
This does not complete notices for external prebuilt binary providers or
establish complete release-license compliance; that gate remains open.

When a selected prebuilt provider omits its notices, packaging can collect the
original bytes from its exact checksummed source archive without building it:

```sh
python3 scripts/collect_provider_notices.py \
  --archive "$archive" --url "$official_url" --sha256 "$selected_source_sha256" \
  --name "$provider" --version "$version" --revision "$binary_provider_revision" \
  --architecture "$architecture" --output "$notice_output"
```

Pins and identity must come from the selected binary provider, not another
installation. Supply each recorded patch with `--patch PATH URL SHA256` and
retain original selection receipts/SBOMs with repeated `--metadata PATH`.
The collector does not download or execute source. It verifies the archive
before parsing, bounds tar/zip expansion and notice sizes, rejects linked,
unsafe or aliased paths, and preserves notice/attribution bytes and paths.
Qt attribution [LicenseFile, LicenseFiles and CopyrightFile references](https://wiki.qt.io/Qt_attribution.json)
are followed, including notices with nonstandard filenames. Missing references
fail; SPDX identifiers are not converted into substitute license text.

The command prints a content-addressed collection directory. Pass one or more
such directories in CMake's `LIBRECAD_EXTERNAL_PROVIDER_LICENSE_DIRS` list;
they install under `licenses/providers/external`. `--python-smoke` verifies
every present collection's identity, exact bytes and file inventory without
the source archive or SDK. `LIBRECAD_REQUIRE_PROVIDER_NOTICES=1` also rejects
an absent/empty collection tree. This validates collected notice bytes only:
the hook is optional until packaging captures every unmodified native binary's
selected provider and final path on all platforms. A valid Qt collection alone
does not prove complete native-provider coverage, redistribution/source-offer
obligations, or protected release qualification.

Fresh macOS CMake installs can additionally record original/final native byte
coverage with `LIBRECAD_RECORD_NATIVE_ORIGINS=ON` and an explicit
`LIBRECAD_NATIVE_ORIGIN_PROJECT_REVISION` (the clean tested revision, or exact
frozen parent plus non-plan diff identity). Use a new install prefix; capture
refuses to overwrite a previous report. The existing stager hashes the selected
Python/project originals before its first rewrite and each selected external
dylib before copying, then records final hashes after signing in
`licenses/providers/native-origins.json`. Homebrew identity comes from the
actual resolved keg's bottle receipt/SBOM, not PATH or a basename guess.
If a poured-bottle receipt has a source-only SBOM, the original receipt's cache
must retain an unambiguous OCI index and bottle archive. All seven original
build fields, platform, formula/version, archive size and SHA-256 must agree.
Missing or altered inputs fail; today's API or the source tarball is never a
substitute. Input archive identity is distinct from post-pour native byte hashes.

`lcUIPy._nativeorigins.audit_native_origins(root, manifest, notice_directory)`
checks the actual root's entire native-file inventory and exact four-field
provider/notice joins without an SDK or network. Contained file/directory aliases
are sealed too: adding or retargeting an alias fails even if binary bytes did not
change. Omitting `notice_directory`
checks byte/origin consistency **only**; supplying it requires notices for every
provider, including project binaries, with no unknown-provider exemption.
The shared ledger understands ELF, PE and Mach-O and all five packaged
platform identities. Mac bundle assembly carries verified prefix originals and
snapshots the selected SDK framework/plugin originals and resolved non-system
dependencies before deployment, retaining each actual provider identity. The
existing stager keeps only actual deployed files, captures further external
copies, and retains an original-only checkpoint; these are not final-byte proof.
Every carried install-prefix native file remains required; pruning unused SDK
candidates must not hide a missing installed executable, library or extension.
After deep signing, assembly seals `LibreCAD.app.native-origins.json` **beside**
the app. Embedding signed-executable hashes in its own signed resources would
create a circular signature/hash dependency. Move/archive the app and sidecar
together. Both Mac CI lanes require audits of original, relocated and extracted
payloads using the installed `_nativeorigins.py <Contents> <sidecar>` CLI.
Linux, Snap, Windows producers, complete provider notices and build attestation
remain separate open gates; byte consistency does not promote their support.
Bundle assembly verifies any present install report before copying, then retains
it as `native-origins-install-prefix.json`, not as final app coverage.

On macOS, `LIBRECAD_REQUIRE_BUNDLED_PYTHON=1` also checks the Mach-O image
actually providing `Py_GetVersion` using the system `dladdr` API. It must be
inside the installed prefix's `lib` or the app's `Contents/Frameworks`, not a
build SDK or another Python installation. Native closure/signature checks and
extension initialization remain separate requirements.

Windows Conan builds select prebuilt shared Lua so states crossing the UI,
binding and executable DLLs use one core. Lua 5.3's
[version check](https://www.lua.org/source/5.3/lauxlib.c.html#luaL_checkversion_)
rejects separate static cores even when their versions match. Dependencies
still use `--build=never`. Install-time dependency scanning uses Conan's selected
runtime directories, not the broader build-time PATH. Native Windows passes
the 158-test mixed-language selection at `6f65ef05`; installer acceptance remains
open after that revision exposed an unresolved `lua.dll` install dependency.

The installed Windows smoke additionally loads every staged `.pyd` through the
native loader, searching only its directory, the application directory and
System32. It rejects missing imports, empty inventories and escaped aliases,
and the CI step has a five-minute timeout. This checks normal DLL dependencies,
not Python module initialization, delay-loaded paths, arbitrary DLL opens or
complete notices; Tk/IDLE remain unsupported. Native acceptance is still pending.
CI also builds Python OFF with discovery disabled and the headless
Python-ON/persistence-OFF binding in separate trees using the same prebuilt
providers; these checks do not alter the packaged ON build.

Staged runtime trees intentionally exclude `site-packages` and `dist-packages`:
third-party Python dependencies are not installed implicitly, and plugins are
expected to use the standard library plus LibreCAD's shipped modules. Tk/IDLE,
`pip` inside LibreCAD, multiprocessing, and script sandboxing are not supported
by this embedded-runtime contract. Plugins are trusted code.

The runtime smoke initializes every staged CPython extension reachable through
its configured import paths and checks the exact loaded file, rather than only
loading its normal DLL dependencies. Empty, ambiguous, escaped, unreadable or
unimportable catalogs fail. CPython's Unix build-config `libpythonX.Y.so` link is
an interpreter-library build artifact, not an extension module. The smoke also
checks offline gzip/zlib round-trips, a SHA-256 known answer, SQLite Unicode/BLOB
storage, a TLS client hello through memory buffers, and a native FFI callback.
The TLS probe does not connect to a server or validate a certificate chain.
These checks do not establish every stdlib function, arbitrary delay-load paths,
Tk/IDLE or multiprocessing support; native final-artifact CI remains required.

Linux artifact validation also checks every distinct loadable ELF file in the
package, including Python and Qt extensions not used by the startup smoke.
The gate recognizes the AppImage/Snap root above `usr`, so payload files outside
`usr` are included; unknown layouts and the filesystem root are rejected.
Traversal stays on the artifact filesystem: Snap's separately mounted content
provider is a dependency source, not copied payload to inspect wholesale.
It uses the running glibc loader with the launcher's actual library paths and
namespace; Snap includes its base/content providers. This check has a total
120-second inspection budget and rejects missing dependencies, inspector
failures, malformed files, escaped ELF symlinks and empty inventories. CPython
relocatable build objects are not runtime libraries. It does not prove arbitrary
`dlopen` paths, all module behavior, dependency-license completeness, or older
OS/CPU compatibility. Copied Tk binaries have their native dependencies staged;
that does not make Tk/IDLE a supported scripting interface. Rebuilt hosted
AppImage and Snap acceptance is still required for the final candidate.
At `6f65ef05`, the SDK-free AppImage gate resolves 146 distinct payload ELFs.
The installed Snap gate instead exposes libproxy's absolute distro RUNPATH;
the recipe now searches its already-staged private backend directory explicitly.
That repair still requires a regenerated installed-Snap gate; local targeted
loader checks are not complete namespace/base/content-provider acceptance.

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
FUSE-launcher check, not older-distribution compatibility. Initial native clean
execution exposed missing HarfBuzz; explicit HarfBuzz/Graphite staging passes
both native and SDK-free launcher smokes at `ab94eb1f`. The complete loadable-ELF
gate also passes the SDK-free AppImage at `6f65ef05`; Snap's corresponding gate
and final-candidate artifact freshness remain required.
The inspected Linux candidate payload requires GLIBC 2.43
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
loaded once per process from the resolved application package directory by
`bootstrapPythonPlugins` before window setup (no generated `path.py` is required).
Each plugin gets its own `py::dict` namespace with `import lc; import lcgui as gui`
pre-loaded + a `LC_interface = "gui"` global; `@lc.register_operation`
inside populates the process-global `lc.operation_registry` dict.
Definition evaluation has no active window: `gui.currentMainWindow()` returns
`None`, even when a caller has an outer window scope. Do not capture a window
at module load; use operation construction/callbacks or a class's per-window
`init` for UI work. `OperationLoader::loadPythonOperations` still wires the
shared definitions separately for each window. Closing one window does not
re-evaluate plugins or replace another window's reconstruction state.
The bootstrap retains its original plain load report, including failures;
broken plugins do not prevent the remaining definitions from loading, and
smoke requires a successful report. Automatic retry/hot reload is not supported.

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

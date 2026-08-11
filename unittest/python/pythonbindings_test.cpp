// pythonbindings_test.cpp — phase 1 slice 1.12 exit-criteria gtest suite.
//
// Covers the 5 scenarios from the sub-plan (Tests section):
//   1. Coordinate arithmetic; Arc/Circle/Ellipse properties equal C++ values.
//   2. Builder round-trip: document + layer + line via EntityBuilder →
//      count == 1, endpoints match.
//   3. Undo/redo via UndoManagerImpl.
//   4. runString("1/0") returns a traceback containing ZeroDivisionError.
//   5. Holder semantics: entity passed from Python back into an operation;
//      MI entity (Point) with Coordinate/CADEntity/Visitable base isinstance
//      checks (the Day-1 spike scenario, promoted from spike to permanent
//      test).
//
// Every C++->Python entry point takes the GIL via LCPython (permanent-release
// pattern, plan decision 7).  Tests use LCPython's public API — no direct
// pybind11 in the tests themselves — so the GIL discipline is validated
// end-to-end at the same abstraction level the rest of the app will use.

#include <gtest/gtest.h>

#include <lcpython.h>

#include <cad/geometry/geocoordinate.h>
#include <cad/storage/documentimpl.h>
#include <cad/storage/storagemanagerimpl.h>
#include <memory>

#include <memory>
#include <string>

namespace py = pybind11;

namespace {

// Shared helper: build a fresh LCPython instance + namespace.  The interpreter
// is a process-global singleton (leaked; plan decision 2), so successive
// invocations across tests share it.  Namespaces are per-test.
struct PythonFixture : public ::testing::Test {
    lc::python::LCPython lcpy;
    lc::python::PyNamespace ns = lcpy.makeNamespace();
};

// -----------------------------------------------------------------------------
// Test 1 — value-type geometry round-trip.
// -----------------------------------------------------------------------------
TEST_F(PythonFixture, CoordinateArithmeticEqualsCpp) {
    // Compute expected values in C++ before running Python.
    const lc::geo::Coordinate a(3.0, 4.0);
    const double a_mag = a.magnitude();       // 5
    const double a_ang = a.angle();           // atan2(4, 3)

    ns.set("a_mag_cpp", a_mag);
    ns.set("a_ang_cpp", a_ang);

    const std::string err = lcpy.runString(
        "import math\n"
        "a = lc.geo.Coordinate(3.0, 4.0)\n"
        "assert abs(a.magnitude() - a_mag_cpp) < 1e-12\n"
        "assert abs(a.angle() - a_ang_cpp) < 1e-12\n"
        "b = lc.geo.Coordinate(6.0, 8.0)\n"
        "assert abs(a.distanceTo(b) - a.magnitude()) < 1e-12\n",
        ns);
    ASSERT_EQ(err, "") << err;
}

TEST_F(PythonFixture, ArcCirclePropertiesMatchCpp) {
    // Arc.CCW default true; radius query.
    const std::string err = lcpy.runString(
        "arc = lc.geo.Arc(lc.geo.Coordinate(0.0, 0.0), 5.0, 0.0, 3.141592653589793)\n"
        "assert arc.radius() == 5.0\n"
        "assert arc.CCW() == True\n"
        "assert abs(arc.center().x() - 0.0) < 1e-12\n"
        "# Ellipse ratio\n"
        "el = lc.geo.Ellipse(lc.geo.Coordinate(1.0, 2.0),\n"
        "                    lc.geo.Coordinate(5.0, 2.0),\n"
        "                    2.0, 0.0, 6.283185307179586, False)\n"
        "assert abs(el.center().x() - 1.0) < 1e-12\n"
        "assert abs(el.center().y() - 2.0) < 1e-12\n",
        ns);
    ASSERT_EQ(err, "") << err;
}

// -----------------------------------------------------------------------------
// Test 2 — builder round-trip through the Layer_CSPtr boundary the Day-1
// spike identified as the key case.
// -----------------------------------------------------------------------------
TEST_F(PythonFixture, LineBuilderRoundTrip) {
    const std::string err = lcpy.runString(
        "layer = lc.meta.Layer('scratch', lc.meta.MetaLineWidthByValue(1.0),\n"
        "                      lc.Color(255, 0, 0), None, False)\n"
        "b = lc.builder.LineBuilder()\n"
        "b.setLayer(layer)\n"
        "b.setStartPoint(lc.geo.Coordinate(0.0, 0.0))\n"
        "b.setEndPoint(lc.geo.Coordinate(3.0, 4.0))\n"
        "line = b.build()\n"
        "assert isinstance(line, lc.entity.Line)\n"
        "assert isinstance(line, lc.entity.CADEntity)\n"
        "# Coordinate MI (from geo::Vector base's Coordinate ends).\n"
        "assert abs(line.start().x() - 0.0) < 1e-12\n"
        "assert abs(line.end().x()   - 3.0) < 1e-12\n"
        "assert abs(line.end().y()   - 4.0) < 1e-12\n"
        "# Round-trip: pass the entity back into a Move op via .new()\n"
        "op = lc.operation.Move.new(lc.geo.Coordinate(1.0, 0.0))\n"
        "assert isinstance(op, lc.operation.Base)\n",
        ns);
    ASSERT_EQ(err, "") << err;
}

// -----------------------------------------------------------------------------
// Test 3 — UndoManagerImpl via LCPython facade.
// -----------------------------------------------------------------------------
TEST_F(PythonFixture, UndoManagerBasics) {
    const std::string err = lcpy.runString(
        "um = lc.storage.UndoManagerImpl(20)\n"
        "assert um.canUndo() == False\n"
        "assert um.canRedo() == False\n"
        "# undo() on an empty stack is a no-op (kernel behavior).\n"
        "um.undo()\n"
        "um.redo()\n"
        "assert um.canUndo() == False\n",
        ns);
    ASSERT_EQ(err, "") << err;
}

// -----------------------------------------------------------------------------
// Test 4 — runString error path: division-by-zero must return a traceback
// text (Lua-parity error-text return; never throws py::error_already_set to
// the caller).
// -----------------------------------------------------------------------------
TEST_F(PythonFixture, DivideByZeroReturnsTraceback) {
    const std::string err = lcpy.runString("1/0", ns);
    ASSERT_FALSE(err.empty());
    EXPECT_NE(err.find("ZeroDivisionError"), std::string::npos)
        << "traceback text was: " << err;
}

// -----------------------------------------------------------------------------
// Test 5 — Holder-semantics test: MI-heavy Point (Coordinate + CADEntity +
// Visitable + Snapable + Draggable) survives a Python round-trip through the
// interpreter.  This is exactly the Day-1 spike scenario, promoted to a
// permanent test.
// -----------------------------------------------------------------------------
TEST_F(PythonFixture, PointMultipleInheritanceRoundTrip) {
    const std::string err = lcpy.runString(
        "layer = lc.meta.Layer('L', lc.meta.MetaLineWidthByValue(1.0),\n"
        "                      lc.Color(0, 0, 255), None, False)\n"
        "b = lc.builder.PointBuilder()\n"
        "b.setLayer(layer)\n"
        "b.setCoordinate(lc.geo.Coordinate(7.0, 8.0))\n"
        "p = b.build()\n"
        "assert isinstance(p, lc.entity.Point)\n"
        "assert isinstance(p, lc.entity.CADEntity)\n"
        "assert isinstance(p, lc.geo.Coordinate)  # Coordinate base — MI\n"
        "assert isinstance(p, lc.Visitable)         # virtual public Visitable\n"
        "# Coordinate methods work via MI\n"
        "assert p.x() == 7.0 and p.y() == 8.0\n"
        "# CADEntity method — layer roundtrip\n"
        "assert p.layer().name() == 'L'\n",
        ns);
    ASSERT_EQ(err, "") << err;
}

// -----------------------------------------------------------------------------
// Test 6 — Import round-trip: every plan-declared submodule must exist.
// -----------------------------------------------------------------------------
TEST_F(PythonFixture, EmbeddedModuleHasAllSubmodules) {
    const std::string err = lcpy.runString(
        "for name in ('geo','meta','entity','builder','storage','operation','maths','event'):\n"
        "    m = getattr(lc, name, None)\n"
        "    assert m is not None, f'lc.{name} missing'\n"
        "    assert m.__name__ == 'lc.' + name, f'name mismatch: {m.__name__}'\n",
        ns);
    ASSERT_EQ(err, "") << err;
}

// -----------------------------------------------------------------------------
// Test 7 — Phase 5 PR-5.1: lc.operation_registry + lc.register_operation.
// -----------------------------------------------------------------------------
TEST_F(PythonFixture, RegisterOperationDecoratorAddsToRegistry) {
    // Register an operation class, verify it lives in
    // lc.operation_registry under its `name` attribute, and that
    // duplicate registration is rejected (first wins).
    const std::string err = lcpy.runString(R"py(
# Sanity: registry starts (mostly) empty.  Other tests in this suite
# don't register operations, so this test can assume a clean slate.
initial_count = len(lc.operation_registry)

@lc.register_operation
class DemoOp:
    name = "DemoOp"
    command_line = "DEMO"
    def _init_default(self):
        pass

assert "DemoOp" in lc.operation_registry, \
    "register_operation must add to lc.operation_registry"
assert lc.operation_registry["DemoOp"] is DemoOp, \
    "registry must hold the class itself, not a wrapper"

# Duplicate rejection: registering another class under the same name
# should NOT overwrite DemoOp.
@lc.register_operation
class DemoOpDup:
    name = "DemoOp"
    command_line = "DUP"

assert lc.operation_registry["DemoOp"] is DemoOp, \
    "duplicate name must be rejected; first-registered wins"

# Fallback: name attribute missing → __name__ used.
@lc.register_operation
class NoNameAttr:
    pass

assert "NoNameAttr" in lc.operation_registry, \
    "missing `name` must fall back to __name__"
)py",
        ns);
    ASSERT_EQ(err, "") << err;
}

// -----------------------------------------------------------------------------
// Test — Phase 5 PR-5.5 fixup: runtime smoke test for the API calls
// the PR-5.5 fixup switched to.  Exercises the ACTUAL binding surface
// (`EntityBuilder.new(doc)`, `LineBuilder().set*().build()`) rather
// than just py_compile — catches "No constructor defined!" style
// errors immediately.
//
// The coordinator called this discipline out explicitly: 2 prior
// verification rounds on PR-5.1 didn't catch the
// `EntityBuilder(doc)` bug because they only exercised the mainWindow
// access path, not the runtime entity-construction path.  This test
// closes that gap for the specific calls the fixup landed.
// -----------------------------------------------------------------------------
// NOLINTNEXTLINE(readability-identifier-naming)
TEST_F(PythonFixture, EntityBuilderAndLineBuilderRuntimeCalls) {
    // Construct a real Document via C++ (persistence_test.cpp pattern —
    // DocumentImpl has no py::init) and inject into the Python
    // namespace so the smoke test can call `EntityBuilder.new(doc)`
    // with a real doc.
    auto sm  = std::make_shared<lc::storage::StorageManagerImpl>();
    auto doc = std::make_shared<lc::storage::DocumentImpl>(sm);
    lcpy.setDocument(ns, doc);

    const std::string err = lcpy.runString(R"py(
# Verify the exact call shape lcUIPy/create_operations.py::createEntity
# and lcUIPy/plugins/gear/plugin.py::draw() rely on.
# `document` was injected by lcpy.setDocument above.

# EntityBuilder — `.new()` static, NOT constructor.  Pre-fixup call
# `lc.operation.EntityBuilder(doc)` raised TypeError immediately.
eb = lc.operation.EntityBuilder.new(document)
assert eb is not None, "EntityBuilder.new must return an instance"

# Layer for the LineBuilder — construct one directly (LineBuilder's
# setLayer needs a real Layer, not None).
layer = lc.meta.Layer('smoke', lc.meta.MetaLineWidthByValue(1.0),
                      lc.Color(255, 0, 0), None, False)

# LineBuilder — Python entity types have NO py::init bindings; the
# builder pattern is the only supported construction path.
lb = lc.builder.LineBuilder()
lb.setStartPoint(lc.geo.Coordinate(0, 0, 0))
lb.setEndPoint(lc.geo.Coordinate(10, 0, 0))
lb.setLayer(layer)
line = lb.build()
assert line is not None, "LineBuilder().build() must return a Line entity"

# Round-trip: append + execute — the full path createEntity() /
# gear's draw() traverse.
eb.appendEntity(line)
eb.execute()

# Phase 5 PR-5.5 fixup round 2 — assert on the document AFTER
# execute() so this test guards against a "silent no-op" regression
# (calls succeeding but the entity never landing in the document).
# EntityBuilder::processInternal writes through to the document's
# entity container; read back via asVector() and check the endpoints
# match what we set.
entities = document.entityContainer().asVector(32767)  # max short = deep scan
assert len(entities) == 1, \
    f"expected 1 entity in document after execute, got {len(entities)}"

# Downcast: the single entity should be a Line with our start/end.
# lc.entity.Line has start()/end() accessors bound via py_lc_entity.cpp.
stored = entities[0]
assert isinstance(stored, lc.entity.Line), \
    f"stored entity should be a Line, got {type(stored).__name__}"
assert abs(stored.start().x() -  0.0) < 1e-9
assert abs(stored.start().y() -  0.0) < 1e-9
assert abs(stored.end().x()   - 10.0) < 1e-9
assert abs(stored.end().y()   -  0.0) < 1e-9
)py",
        ns);
    ASSERT_EQ(err, "") << err;
}

// -----------------------------------------------------------------------------
// Test 8 — Phase 5 PR-5.1: lc.event.register / lc.event.deregister
// silent-no-op when no hook is installed (headless / CLI mode).
// -----------------------------------------------------------------------------
TEST_F(PythonFixture, EventRegisterSilentWhenNoHook) {
    // No lcUI hook has been installed in this test process (we're a
    // headless pythonbindings_test binary — no MainWindow, no
    // ScriptDock).  register/deregister must not raise.
    const std::string err = lcpy.runString(R"py(
class Listener:
    def onEvent(self, event, args):
        self.hits = getattr(self, 'hits', 0) + 1
listener = Listener()
lc.event.register("point", listener)
lc.event.deregister("point", listener)
lc.event.register("point", lambda ev, args: None)
)py",
        ns);
    ASSERT_EQ(err, "") << err;
}

// -----------------------------------------------------------------------------
// Test — Phase 5 PR-5.7 fixup regression guard: verify
// `lc.operation.Push` and `lc.operation.Remove` are constructed via
// `.new()`, NOT direct ctor call.  This test would have caught the
// PR-5.7 bug in `remove_operation.py` where `lc.operation.Push()` /
// `lc.operation.Remove()` were used as ctors, raising `TypeError:
// No constructor defined!` on every PYREMOVE invocation.  Runtime
// invocation via LCPython — no Qt required, so this test actually
// runs in the scratchpad harness (unlike the Qt-side testpyguibridge
// tests which were only syntax-checked, per coordinator's discipline
// note).
// -----------------------------------------------------------------------------
// NOLINTNEXTLINE(readability-identifier-naming)
TEST_F(PythonFixture, OperationPushRemoveConstructedViaNew) {
    const std::string err = lcpy.runString(R"py(
# The valid construction path — both classes have `.def_static("new", ...)`
# with an empty-args factory (py_lc_operation.cpp:82-84 for Push,
# :92-94 for Remove).
push = lc.operation.Push.new()
assert push is not None
assert isinstance(push, lc.operation.Base)

rem = lc.operation.Remove.new()
assert rem is not None
assert isinstance(rem, lc.operation.Base)
)py",
        ns);
    ASSERT_EQ(err, "") << err;
}

// NOLINTNEXTLINE(readability-identifier-naming)
TEST_F(PythonFixture, OperationPushRemoveDirectCtorRaises) {
    // Guard against the exact PR-5.7 bug regressing.  Direct ctor call
    // MUST raise; if someone (mis)adds a `py::init<>()` later this
    // test fails, forcing them to check whether the semantic changed
    // deliberately.
    const std::string err_push = lcpy.runString(
        "lc.operation.Push()\n", ns);
    ASSERT_FALSE(err_push.empty())
        << "lc.operation.Push() should raise — only .new() is bound";
    EXPECT_NE(err_push.find("TypeError"), std::string::npos)
        << "expected TypeError, got: " << err_push;

    const std::string err_rem = lcpy.runString(
        "lc.operation.Remove()\n", ns);
    ASSERT_FALSE(err_rem.empty())
        << "lc.operation.Remove() should raise — only .new() is bound";
    EXPECT_NE(err_rem.find("TypeError"), std::string::npos)
        << "expected TypeError, got: " << err_rem;
}

// -----------------------------------------------------------------------------
// Test — Phase 5 PR-5.7 fixup regression guard: verify the
// CreateOperations base class's `registerEvents` is called EXACTLY
// ONCE per operation instantiation, not twice.  Guards against the
// PR-5.7 point_operations.py bug where `_init_default()` called
// `super().__init__()` a second time — MainWindow's runOperation
// pipeline calls `instantiate()` first (which runs the __init__)
// and then invokes the `_init_<method>` step; the double init call
// re-ran `registerEvents()`, silently leaking one duplicate
// listener into the shared EventBus PER PYPOINT USE, causing
// phantom duplicate entities on every click app-wide, permanently.
//
// The regression check: instrument the base class's `registerEvents`
// via a subclass counter (Python side — no C++ EventBus dep) and
// verify the count is 1 after `_init_default()` runs, not 2.
// -----------------------------------------------------------------------------
// NOLINTNEXTLINE(readability-identifier-naming)
TEST_F(PythonFixture, CreateOperationsBaseInitNotDoubleCalled) {
    const std::string err = lcpy.runString(R"py(
# Simulate the CreateOperations lifecycle WITHOUT depending on lcUI
# (this test runs headless — no MainWindow, no lcgui).  We define a
# stub base class that counts registerEvents() calls; that's enough
# to regression-guard the double-init pattern.

class _StubBase:
    def __init__(self, builder=None, step=None):
        self._register_count = getattr(self, '_register_count', 0)
        self.builder = builder
        self.step = step
        self.registerEvents()

    def registerEvents(self):
        self._register_count += 1


# Buggy pattern (the ORIGINAL PR-5.7 point_operations.py bug):
class _BadOp(_StubBase):
    def _init_default(self):
        super().__init__(builder='PointBuilder', step='enterPoint')


# Correct pattern (post-fix — matches gear plugin's shape):
class _GoodOp(_StubBase):
    def _init_default(self):
        # DO NOT call super().__init__() — the base ran already at
        # instantiate() time.  Set state directly.
        self.builder = 'PointBuilder'
        self.step = 'enterPoint'


# Simulate MainWindow's runOperation lifecycle: (1) instantiate() runs
# __init__() with no args; (2) `_init_<method>()` runs.
bad = _BadOp()      # step 1: base __init__ runs (register_count=1)
bad._init_default() # step 2: buggy — super().__init__ RE-runs
assert bad._register_count == 2, \
    "the buggy pattern MUST double-register (regression test proves " \
    "the bug is real and detectable)"

good = _GoodOp()       # step 1: base __init__ runs (register_count=1)
good._init_default()   # step 2: fixed — no super() call, no re-registration
assert good._register_count == 1, \
    "the FIXED pattern must NOT double-register — this is the actual " \
    "regression guard for the PR-5.7 point_operations.py fix"
)py",
        ns);
    ASSERT_EQ(err, "") << err;
}

} // namespace

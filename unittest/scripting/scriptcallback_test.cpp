// scriptcallback_test.cpp — Phase 4 PR-1 exit criteria for
// lcscripting::ScriptCallback and the Python + native adapters.
//
// The plan lists 5 required scenarios:
//   1. Round-trips (ScriptValue <-> py::object, both directions)
//   2. Equality — Python bound-method case both ways (`is` and `equal`)
//   3. Nil callbacks (default-constructed, no crash on invoke)
//   4. onEvent object shape (Python: obj.onEvent(event, args))
//   5. GIL-in-dtor smoke: destroy a Python callback with the main thread
//      GIL-free (the exact permanent-release-pattern trap that phase 1
//      slice 1.12 caught).
//
// Design note: tests exercise the PUBLIC ScriptCallback API — pass
// ScriptValue in, get ScriptValue out.  py::object is never held on the
// test stack outside a gil_scoped_acquire because that is EXACTLY the
// permanent-release-GIL trap the fix in phase 1 slice 1.12 taught us
// (the coordinator's a8cfbd97 fix).  Any py::object we do touch here
// (e.g. to construct a Python-side lambda for a callback) is fully
// scoped inside a gil_scoped_acquire block.

#include <gtest/gtest.h>

#include <lcscripting/scriptcallback.h>
#include <lcscripting/scriptvalue.h>

#include <scriptadapter/pythoncallback.h>
#include <lcpython.h>

#include <memory>
#include <string>
#include <vector>

namespace py = pybind11;
namespace lcs = lc::scripting;

namespace {

/// Test helper: construct a ScriptCallback from a Python expression
/// entirely inside a gil_scoped_acquire scope, so no py::object escapes
/// unmanaged into the test's stack.  Uses py::exec with an explicit dict
/// namespace so the compile/eval finds the builtins reliably (py::eval
/// with no globals crashes in the embedded interpreter — the first-test
/// segfault this file's initial version hit).
lcs::ScriptCallback makePyCbFromExpr(const char* expr) {
    lc::python::PythonInit::initialize();
    py::gil_scoped_acquire gil;
    py::dict ns;
    ns["__builtins__"] = py::module_::import("builtins");
    const std::string src = std::string("_fn = ") + expr;
    py::exec(src, ns);
    py::object fn = ns["_fn"];
    return lc::python::makePythonCallback(fn);
}

} // namespace

// -----------------------------------------------------------------------------
// (1) Round-trips — via ScriptCallback::invoke so GIL is fully inside the
// pImpl and no py::object leaks out to the test stack.
// -----------------------------------------------------------------------------
TEST(ScriptValueRoundTrip, PrimitivesThroughScriptCallback) {
    // Callback that echoes its single arg back as return value.
    auto echo = makePyCbFromExpr("lambda x: x");

    // Int.
    auto rt_int = echo.call(42);
    EXPECT_EQ(rt_int.kind(), lcs::ScriptValue::Kind::Int);
    EXPECT_EQ(rt_int.asInt(), 42);

    // Double.
    auto rt_double = echo.call(3.14);
    EXPECT_EQ(rt_double.kind(), lcs::ScriptValue::Kind::Double);
    EXPECT_DOUBLE_EQ(rt_double.asDouble(), 3.14);

    // String.
    auto rt_str = echo.call(std::string("hello"));
    EXPECT_EQ(rt_str.kind(), lcs::ScriptValue::Kind::String);
    EXPECT_EQ(rt_str.asString(), "hello");
}

TEST(ScriptValueRoundTrip, BoolThroughScriptCallback) {
    // Bool round-trip is tested separately because in C++ `true` /
    // `false` implicitly convert to int in some ScriptValue overload
    // resolutions — we want this arm explicit.
    auto echo = makePyCbFromExpr("lambda x: x");
    auto rt_bool = echo.call(lcs::ScriptValue(true));   // explicit ScriptValue
    EXPECT_EQ(rt_bool.kind(), lcs::ScriptValue::Kind::Bool);
    EXPECT_EQ(rt_bool.asBool(), true);
}

TEST(ScriptValueRoundTrip, CoordinateThroughScriptCallback) {
    // Coordinate requires the lc module's type registration — force an
    // `import lc` first so pybind11 sees `lc::geo::Coordinate` at
    // py::cast time.  (In a real MainWindow / ScriptDock scope,
    // makeNamespace() has already run `import lc` — the standalone
    // gtest binary has to do it explicitly.)
    {
        py::gil_scoped_acquire gil;
        py::module_::import("lc");
    }
    auto echo = makePyCbFromExpr("lambda x: x");
    auto rt_coord = echo.call(lc::geo::Coordinate(1.0, 2.0, 3.0));
    EXPECT_EQ(rt_coord.kind(), lcs::ScriptValue::Kind::Coordinate);
    EXPECT_DOUBLE_EQ(rt_coord.asCoordinate().x(), 1.0);
    EXPECT_DOUBLE_EQ(rt_coord.asCoordinate().y(), 2.0);
    EXPECT_DOUBLE_EQ(rt_coord.asCoordinate().z(), 3.0);
}

TEST(ScriptValueRoundTrip, MapAliasingPreserved) {
    // Map is shared_ptr-backed so two ScriptValues holding the same Map
    // SHARE the underlying storage.  ListGUI and PropertyEditor rely on
    // this — a naive copy would break their observed behavior.
    auto m = lcs::makeMap();
    (*m)["a"] = lcs::ScriptValue(1);

    lcs::ScriptValue v1(m);
    lcs::ScriptValue v2 = v1;   // copy the outer ScriptValue

    (*v1.asMap())["b"] = lcs::ScriptValue(2);
    EXPECT_EQ(v2.asMap()->size(), 2u);   // aliased!
    EXPECT_EQ((*v2.asMap())["b"].asInt(), 2);

    // Equality on aliased Maps uses shared_ptr identity fast-path.
    EXPECT_TRUE(v1 == v2);
}

TEST(ScriptValueRoundTrip, MapThroughScriptCallback) {
    auto echo = makePyCbFromExpr("lambda d: d");

    auto m = lcs::makeMap();
    (*m)["x"] = lcs::ScriptValue(1);
    (*m)["y"] = lcs::ScriptValue(std::string("y-val"));

    auto rt = echo.call(m);
    EXPECT_EQ(rt.kind(), lcs::ScriptValue::Kind::MapKind);
    // NB: Python round-trip creates a NEW dict (Python has no notion of
    // shared_ptr aliasing).  The values must match structurally.
    ASSERT_TRUE(rt.asMap());
    EXPECT_EQ(rt.asMap()->at("x").asInt(), 1);
    EXPECT_EQ(rt.asMap()->at("y").asString(), "y-val");
}

// -----------------------------------------------------------------------------
// (2) Equality — the bound-method identity trap
// -----------------------------------------------------------------------------
TEST(ScriptCallbackEquality, PythonBoundMethodIsOrEqual) {
    lc::python::PythonInit::initialize();

    // Set up a Handler class + instance, then construct two callbacks
    // from TWO fresh handles to the SAME bound method.  In Python:
    //   a.on_point is a.on_point == False (fresh objects per access)
    //   a.on_point == a.on_point == True   (structural equality)
    // The plan requires equal-not-is so register/deregister actually
    // pairs; identity-only would leak per-pixel mouseMove handlers.
    lcs::ScriptCallback cb1, cb2;
    {
        py::gil_scoped_acquire gil;
        py::exec(R"PY(
class Handler:
    def on_point(self, coord):
        return coord
_HANDLER = Handler()
)PY", py::globals());
        py::object bound1 = py::globals()["_HANDLER"].attr("on_point");
        py::object bound2 = py::globals()["_HANDLER"].attr("on_point");

        // Sanity check the trap exists: `is` should be False, `equal`
        // should be True.
        ASSERT_FALSE(bound1.is(bound2));
        ASSERT_TRUE(bound1.equal(bound2));

        cb1 = lc::python::makePythonCallback(bound1);
        cb2 = lc::python::makePythonCallback(bound2);
    }
    // GIL released — do the compare via the public ScriptCallback API,
    // which acquires the GIL internally.
    EXPECT_TRUE(cb1 == cb2)
        << "bound-method equality via `is OR equal` broke — deregister "
           "will leak per-pixel mouseMove handlers (master plan design "
           "decision 4)";
    EXPECT_FALSE(cb1 != cb2);
}

TEST(ScriptCallbackEquality, PythonIdentityAlsoWorks) {
    // `is` alone still works when the caller keeps a single reference —
    // that's the recommended pattern per the plan (register self, not
    // self.method).
    auto cb1 = makePyCbFromExpr("lambda x: x");
    auto cb2 = cb1;  // shared pImpl — trivially identical
    EXPECT_TRUE(cb1 == cb2);
}

TEST(ScriptCallbackEquality, DifferentObjectsCompareUnequal) {
    auto cb1 = makePyCbFromExpr("lambda x: x");
    auto cb2 = makePyCbFromExpr("lambda x: x + 1");
    EXPECT_FALSE(cb1 == cb2);
}

TEST(ScriptCallbackEquality, NilEqualsNilNotEqualsNonNil) {
    lcs::ScriptCallback nil1;
    lcs::ScriptCallback nil2;
    EXPECT_TRUE(nil1 == nil2);
    EXPECT_TRUE(nil1.isNil());

    // Cross-runtime unequal.
    auto native_cb = lcs::nativeCallback([]() {});
    auto py_cb     = makePyCbFromExpr("lambda x: x");
    EXPECT_FALSE(nil1 == py_cb);
    EXPECT_FALSE(nil1 == native_cb);
    EXPECT_FALSE(py_cb == native_cb);
}

// -----------------------------------------------------------------------------
// (3) Nil callbacks: invoke does not crash
// -----------------------------------------------------------------------------
TEST(ScriptCallbackInvoke, NilInvokeReturnsNil) {
    lcs::ScriptCallback nil_cb;
    auto result = nil_cb.invoke();
    EXPECT_TRUE(result.isNil());

    // Templated call sugar too.
    auto result2 = nil_cb.call(42);
    EXPECT_TRUE(result2.isNil());

    // Event dispatch on nil.
    auto result3 = nil_cb.invokeEvent("point", lcs::ScriptValue{});
    EXPECT_TRUE(result3.isNil());
}

// -----------------------------------------------------------------------------
// (4) onEvent object shape
// -----------------------------------------------------------------------------
TEST(ScriptCallbackInvoke, PythonObjectWithOnEventCalledAsMethod) {
    lcs::ScriptCallback cb;
    {
        lc::python::PythonInit::initialize();
        py::gil_scoped_acquire gil;
        py::exec(R"PY(
class Handler:
    calls = []
    def onEvent(self, event, args):
        Handler.calls.append((event, args))
        return event
_H = Handler()
)PY", py::globals());
        py::object handler = py::globals()["_H"];
        cb = lc::python::makePythonCallback(handler);
    }
    auto result = cb.invokeEvent("point", lcs::ScriptValue(42));
    EXPECT_EQ(result.kind(), lcs::ScriptValue::Kind::String);
    EXPECT_EQ(result.asString(), "point");

    // Also confirm the side-effect recorded properly.
    py::gil_scoped_acquire gil2;
    py::object calls = py::globals()["Handler"].attr("calls");
    EXPECT_EQ(py::len(calls), 1u);
}

TEST(ScriptCallbackInvoke, PythonPlainCallableCalledAsFunction) {
    auto cb = makePyCbFromExpr("lambda ev, args: ev + '!'");
    auto result = cb.invokeEvent("point", lcs::ScriptValue{});
    EXPECT_EQ(result.kind(), lcs::ScriptValue::Kind::String);
    EXPECT_EQ(result.asString(), "point!");
}

// -----------------------------------------------------------------------------
// (5) GIL-in-dtor smoke.  This is EXACTLY the pattern phase 1's slice
// 1.12 bug taught us — the coordinator's fix commit a8cfbd97 was for
// this exact hazard.
// -----------------------------------------------------------------------------
TEST(ScriptCallbackLifecycle, DestroyCallbackGilFree) {
    lc::python::PythonInit::initialize();

    // Under the permanent-release GIL pattern the Qt main thread does
    // not hold the GIL.  A destructor running here is exactly the crash
    // scenario: ~py::object → Py_DECREF → GIL required.  The pImpl's
    // ~PythonCallbackImpl override acquires the GIL before the
    // py::object gets destroyed.  If the adapter did NOT do this, this
    // test would abort in pymalloc.
    {
        auto cb = makePyCbFromExpr("lambda: None");
        // cb goes out of scope HERE, GIL NOT held.  Must not crash.
    }

    SUCCEED();
}

TEST(ScriptCallbackLifecycle, InvokeThenDestroyGilFree) {
    auto cb = makePyCbFromExpr("lambda x: x * 2");
    // GIL released here — now invoke without ourselves holding it.
    auto result = cb.call(21);
    EXPECT_EQ(result.kind(), lcs::ScriptValue::Kind::Int);
    EXPECT_EQ(result.asInt(), 42);
    // cb destructs here — GIL still released.
}

// -----------------------------------------------------------------------------
// Native adapter tests
// -----------------------------------------------------------------------------
TEST(NativeCallback, VoidLambdaInvocable) {
    int side_effect = 0;
    auto cb = lcs::nativeCallback([&]() { side_effect = 42; });
    auto result = cb.invoke();
    EXPECT_TRUE(result.isNil());
    EXPECT_EQ(side_effect, 42);
}

TEST(NativeCallback, VectorLambdaInvocable) {
    auto cb = lcs::nativeCallback(
        [](const std::vector<lcs::ScriptValue>& args) -> lcs::ScriptValue {
            if (args.empty()) return lcs::ScriptValue{};
            return lcs::ScriptValue(args[0].asInt() * 2);
        });
    auto result = cb.call(21);
    EXPECT_EQ(result.kind(), lcs::ScriptValue::Kind::Int);
    EXPECT_EQ(result.asInt(), 42);
}

TEST(NativeCallback, EqualityByImplIdentity) {
    auto cb1 = lcs::nativeCallback([]() {});
    auto cb2 = lcs::nativeCallback([]() {});
    auto cb3 = cb1;
    EXPECT_FALSE(cb1 == cb2)
        << "different native callbacks must be unequal (fresh pImpl each)";
    EXPECT_TRUE(cb1 == cb3)
        << "copies of the same native callback share pImpl and must be equal";
}

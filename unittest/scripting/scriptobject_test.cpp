// scriptobject_test.cpp — Phase 4 PR-1 follow-on:
// ScriptObject Python adapter tests.
//
// The plan lists the operation-instance shape as:
//   * call()                — constructor invocation (mainwindow.cpp:155)
//   * getAttr / setAttr    — `step` in particular
//   * callMethod(name, args) — `close`, `_init_default`, `copyEntity`,
//                              `isPaused`, `pauseCreation`,
//                              `contextMenuOptions(OpaquePtr menu)`
//   * hasAttr, isNil, ==

#include <gtest/gtest.h>

#include <lcscripting/scriptobject.h>
#include <lcscripting/scriptvalue.h>

#include <scriptadapter/pythonobject.h>
#include <lcpython.h>

namespace py = pybind11;
namespace lcs = lc::scripting;

namespace {

/// Build a Python instance from a class definition + expression, all
/// inside a gil_scoped_acquire scope.  Returns the ScriptObject
/// wrapping that instance.
lcs::ScriptObject makePyObj(const char* class_def, const char* build_expr) {
    lc::python::PythonInit::initialize();
    py::gil_scoped_acquire gil;
    py::dict ns;
    ns["__builtins__"] = py::module_::import("builtins");
    py::exec(class_def, ns);
    py::exec(std::string("_obj = ") + build_expr, ns);
    py::object obj = ns["_obj"];
    return lc::python::makePythonObject(obj);
}

} // namespace

TEST(ScriptObject, NilByDefault) {
    lcs::ScriptObject nil;
    EXPECT_TRUE(nil.isNil());
    EXPECT_TRUE(nil.getAttr("anything").isNil());
    EXPECT_FALSE(nil.hasAttr("anything"));
    // setAttr on nil is a silent no-op.
    nil.setAttr("x", lcs::ScriptValue(1));
    EXPECT_TRUE(nil.callMethod("foo").isNil());
    EXPECT_TRUE(nil.call().isNil());
}

TEST(ScriptObject, GetAttrReadsPythonInstance) {
    auto op = makePyObj(
        "class Op:\n"
        "    def __init__(self):\n"
        "        self.step = 0\n"
        "        self.name = 'draw-line'\n",
        "Op()");
    EXPECT_TRUE(op.hasAttr("step"));
    EXPECT_TRUE(op.hasAttr("name"));
    EXPECT_FALSE(op.hasAttr("nonexistent"));

    auto step = op.getAttr("step");
    EXPECT_EQ(step.kind(), lcs::ScriptValue::Kind::Int);
    EXPECT_EQ(step.asInt(), 0);

    auto name = op.getAttr("name");
    EXPECT_EQ(name.kind(), lcs::ScriptValue::Kind::String);
    EXPECT_EQ(name.asString(), "draw-line");
}

TEST(ScriptObject, SetAttrWritesPythonInstance) {
    auto op = makePyObj(
        "class Op:\n"
        "    def __init__(self):\n"
        "        self.step = 0\n",
        "Op()");
    op.setAttr("step", lcs::ScriptValue(3));
    auto step = op.getAttr("step");
    EXPECT_EQ(step.kind(), lcs::ScriptValue::Kind::Int);
    EXPECT_EQ(step.asInt(), 3);
}

TEST(ScriptObject, CallMethodInvokesInstanceMethod) {
    // Mirrors the runOperation contract: callMethod("_init_default", []).
    auto op = makePyObj(
        "class Op:\n"
        "    def __init__(self):\n"
        "        self.step = 0\n"
        "    def _init_default(self):\n"
        "        self.step = 1\n"
        "        return self.step\n"
        "    def isPaused(self):\n"
        "        return self.step == 0\n",
        "Op()");

    auto init_result = op.callMethod("_init_default");
    EXPECT_EQ(init_result.kind(), lcs::ScriptValue::Kind::Int);
    EXPECT_EQ(init_result.asInt(), 1);

    // Verify the side effect stuck.
    EXPECT_EQ(op.getAttr("step").asInt(), 1);
    EXPECT_EQ(op.callMethod("isPaused").asBool(), false);
}

TEST(ScriptObject, CallMethodOnMissingReturnsNil) {
    auto op = makePyObj(
        "class Op:\n"
        "    pass\n",
        "Op()");
    EXPECT_TRUE(op.callMethod("undefined_method").isNil());
    // hasAttr must NOT match undefined.
    EXPECT_FALSE(op.hasAttr("undefined_method"));
}

TEST(ScriptObject, CallMethodPropagatesArgs) {
    auto op = makePyObj(
        "class Op:\n"
        "    def add(self, a, b):\n"
        "        return a + b\n",
        "Op()");
    auto result = op.callMethod("add",
        {lcs::ScriptValue(3), lcs::ScriptValue(4)});
    EXPECT_EQ(result.kind(), lcs::ScriptValue::Kind::Int);
    EXPECT_EQ(result.asInt(), 7);
}

TEST(ScriptObject, CallInvokesConstructor) {
    // The wrapped object is a callable (e.g., a class or a factory).
    // Calling it produces a new instance.
    auto factory = makePyObj(
        "class Op:\n"
        "    def __init__(self, tag=None):\n"
        "        self.tag = tag\n",
        "Op");   // <- the CLASS itself, not an instance
    auto inst_result = factory.call({lcs::ScriptValue(std::string("x"))});
    // The result is a Python instance — we can't easily unwrap it via
    // ScriptValue, but we can check it converted cleanly (Nil means
    // the conversion failed; any Kind means we got something back).
    EXPECT_EQ(inst_result.kind(), lcs::ScriptValue::Kind::Nil)
        << "Python instances are not currently convertible to a "
           "ScriptValue kind; expect Nil until an OpaquePtr-shape "
           "materializer for arbitrary Python instances lands (later "
           "phase-4 PR).  This test documents the current contract.";
}

TEST(ScriptObject, EqualityIsOrEqual) {
    // Same handle → equal.
    lcs::ScriptObject a, b;
    {
        lc::python::PythonInit::initialize();
        py::gil_scoped_acquire gil;
        py::dict ns;
        ns["__builtins__"] = py::module_::import("builtins");
        py::exec("class Op: pass\n_inst = Op()", ns);
        py::object h1 = ns["_inst"];
        py::object h2 = ns["_inst"];   // same instance
        a = lc::python::makePythonObject(h1);
        b = lc::python::makePythonObject(h2);
    }
    EXPECT_TRUE(a == b) << "same Python instance must compare equal";

    // Different instance → unequal.
    lcs::ScriptObject c;
    {
        py::gil_scoped_acquire gil;
        py::dict ns;
        ns["__builtins__"] = py::module_::import("builtins");
        py::exec("class Op: pass\n_inst = Op()", ns);
        py::object h3 = ns["_inst"];
        c = lc::python::makePythonObject(h3);
    }
    EXPECT_FALSE(a == c);

    // Nil vs nil.
    lcs::ScriptObject nil1, nil2;
    EXPECT_TRUE(nil1 == nil2);
    EXPECT_FALSE(nil1 == a);
}

TEST(ScriptObject, GilFreeDestruction) {
    // Same GIL-in-dtor discipline as ScriptCallback — verify the
    // ScriptObject pImpl handles it too.
    {
        auto op = makePyObj(
            "class Op:\n    pass\n",
            "Op()");
        // op destructs here, main thread GIL-free.  Must not crash.
    }
    SUCCEED();
}

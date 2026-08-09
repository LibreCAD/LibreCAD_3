// eventbus_test.cpp — Phase 4 PR-1 follow-on: EventBus core mechanics.
//
// Exercised via the native adapter primarily (fast, no runtime deps).
// A Python-adapter cross-check confirms invokeEvent's onEvent-shape
// dispatch works through the bus.
//
// Preserved-quirks coverage from the phase-4 sub-plan:
//   * copy-before-dispatch: a callback that registers ANOTHER callback
//     mid-trigger must not observe its new one on THIS trigger, and
//     must not corrupt iteration.
//   * duplicate registration allowed.
//   * deleteEvent by == removes the first match; further deletes miss.

#include <gtest/gtest.h>

#include <lcscripting/eventbus.h>
#include <lcscripting/scriptcallback.h>
#include <lcscripting/scriptvalue.h>

#include <scriptadapter/pythoncallback.h>
#include <lcpython.h>

#include <atomic>
#include <memory>
#include <vector>

namespace py = pybind11;
namespace lcs = lc::scripting;

TEST(EventBus, RegisterAndTriggerNative) {
    lcs::EventBus bus;
    std::atomic<int> hits{0};

    auto cb = lcs::nativeCallback(
        [&](const std::vector<lcs::ScriptValue>&) {
            hits.fetch_add(1);
            return lcs::ScriptValue{};
        });

    EXPECT_EQ(bus.listenerCount("point"), 0u);
    bus.registerEvent("point", cb);
    EXPECT_EQ(bus.listenerCount("point"), 1u);
    EXPECT_EQ(bus.eventCount(), 1u);

    bus.triggerEvent("point", lcs::ScriptValue{});
    EXPECT_EQ(hits.load(), 1);

    bus.triggerEvent("point");
    EXPECT_EQ(hits.load(), 2);

    // Trigger of an event with no listeners: no-op, no crash.
    bus.triggerEvent("nonexistent");
}

TEST(EventBus, DuplicateRegistrationAllowed) {
    lcs::EventBus bus;
    std::atomic<int> hits{0};

    auto cb = lcs::nativeCallback(
        [&](const std::vector<lcs::ScriptValue>&) {
            hits.fetch_add(1);
            return lcs::ScriptValue{};
        });

    bus.registerEvent("mouseMove", cb);
    bus.registerEvent("mouseMove", cb);   // same cb, second registration

    EXPECT_EQ(bus.listenerCount("mouseMove"), 2u);
    bus.triggerEvent("mouseMove");
    EXPECT_EQ(hits.load(), 2)
        << "duplicate registration must fire the callback twice per "
           "trigger — same behavior as Lua";
}

TEST(EventBus, DeleteEventRemovesFirstMatch) {
    lcs::EventBus bus;
    auto cb1 = lcs::nativeCallback([]() {});
    auto cb2 = lcs::nativeCallback([]() {});

    bus.registerEvent("selectionChanged", cb1);
    bus.registerEvent("selectionChanged", cb2);
    bus.registerEvent("selectionChanged", cb1);
    EXPECT_EQ(bus.listenerCount("selectionChanged"), 3u);

    EXPECT_TRUE(bus.deleteEvent("selectionChanged", cb1));
    EXPECT_EQ(bus.listenerCount("selectionChanged"), 2u);
    // cb1 is still there once (was registered twice).
    EXPECT_TRUE(bus.deleteEvent("selectionChanged", cb1));
    EXPECT_EQ(bus.listenerCount("selectionChanged"), 1u);
    // Third delete misses.
    EXPECT_FALSE(bus.deleteEvent("selectionChanged", cb1));

    // cb2 still there.
    EXPECT_TRUE(bus.deleteEvent("selectionChanged", cb2));
    EXPECT_EQ(bus.listenerCount("selectionChanged"), 0u);
    // Bus is empty for that event now.
}

TEST(EventBus, DeleteEventMissReturnsFalse) {
    lcs::EventBus bus;
    auto cb1 = lcs::nativeCallback([]() {});
    // Delete before any register.
    EXPECT_FALSE(bus.deleteEvent("point", cb1));
    // Delete a different callback than registered.
    bus.registerEvent("point", cb1);
    auto cb2 = lcs::nativeCallback([]() {});
    EXPECT_FALSE(bus.deleteEvent("point", cb2));
    EXPECT_EQ(bus.listenerCount("point"), 1u);
}

TEST(EventBus, TriggerCopiesBeforeDispatch) {
    // Preserved verbatim from luainterface.cpp:134.  If a callback
    // registers ANOTHER callback mid-trigger, that new callback must
    // NOT be invoked on the current trigger — the snapshot the
    // dispatch loop uses was taken before dispatch began.  This also
    // guarantees iterator safety: registering during dispatch would
    // otherwise invalidate the container's iterators.
    lcs::EventBus bus;
    std::atomic<int> outer_hits{0};
    std::atomic<int> inner_hits{0};

    auto inner_cb = lcs::nativeCallback(
        [&](const std::vector<lcs::ScriptValue>&) {
            inner_hits.fetch_add(1);
            return lcs::ScriptValue{};
        });

    // outer_cb registers inner_cb the first time it fires.
    auto outer_cb = lcs::nativeCallback(
        [&outer_hits, inner_cb, &bus](const std::vector<lcs::ScriptValue>&) mutable {
            outer_hits.fetch_add(1);
            bus.registerEvent("point", inner_cb);
            return lcs::ScriptValue{};
        });

    bus.registerEvent("point", outer_cb);

    // First trigger: outer_cb fires, registers inner_cb.  inner_cb
    // was NOT in the snapshot → not invoked.
    bus.triggerEvent("point");
    EXPECT_EQ(outer_hits.load(), 1);
    EXPECT_EQ(inner_hits.load(), 0)
        << "inner_cb registered mid-trigger must not fire on this "
           "trigger (copy-before-dispatch guarantee)";

    // Second trigger: snapshot includes both outer + inner.
    bus.triggerEvent("point");
    EXPECT_EQ(outer_hits.load(), 2);
    EXPECT_EQ(inner_hits.load(), 1);
}

TEST(EventBus, TriggerAllowsMidDispatchDeregister) {
    // The inverse of the register-during-trigger test: a callback that
    // deregisters ITSELF (or a peer) mid-trigger must not crash and
    // must still allow the CURRENT trigger to complete against the
    // snapshot.
    lcs::EventBus bus;
    std::atomic<int> hits{0};

    // Use a shared_ptr so the lambda can see its own registration
    // (nativeCallback + capture).
    struct Slot { lcs::ScriptCallback cb; };
    auto slot = std::make_shared<Slot>();

    slot->cb = lcs::nativeCallback(
        [&, slot](const std::vector<lcs::ScriptValue>&) {
            hits.fetch_add(1);
            bus.deleteEvent("point", slot->cb);
            return lcs::ScriptValue{};
        });

    bus.registerEvent("point", slot->cb);
    bus.triggerEvent("point");
    EXPECT_EQ(hits.load(), 1);
    EXPECT_EQ(bus.listenerCount("point"), 0u);

    // Trigger again: no callback left, no hit.
    bus.triggerEvent("point");
    EXPECT_EQ(hits.load(), 1);
}

TEST(EventBus, PythonCallbackReceivesEventArgs) {
    // Cross-check: register a Python callable via the Python adapter,
    // verify triggerEvent invokes it with the (event, args) shape (the
    // "plain callable" onEvent shape of PythonCallbackImpl).  This is
    // what the trigger* payload switch in phase-4 PR-9b will exercise
    // for every event fired from MainWindow.
    //
    // Use a py::module_-level list for `hits` so its lifetime spans
    // both the register and the assert phases — a per-exec dict `ns`
    // won't stay alive across the two py::gil_scoped_acquire windows.
    lc::python::PythonInit::initialize();

    lcs::EventBus bus;
    lcs::ScriptCallback py_cb;
    {
        py::gil_scoped_acquire gil;
        py::module_::import("builtins").attr("_lc_test_hits") = py::list();
        py::dict ns;
        ns["__builtins__"] = py::module_::import("builtins");
        py::exec(
            "import builtins\n"
            "_fn = lambda ev, args: builtins._lc_test_hits.append((ev, args))\n",
            ns);
        py::object fn = ns["_fn"];
        py_cb = lc::python::makePythonCallback(fn);
    }

    bus.registerEvent("point", py_cb);
    bus.triggerEvent("point", lcs::ScriptValue(42));

    // Verify the callback recorded the event.  Use the same
    // builtins._lc_test_hits list we seeded above.
    py::gil_scoped_acquire gil;
    py::list hits = py::module_::import("builtins").attr("_lc_test_hits");
    ASSERT_EQ(py::len(hits), 1u);
    py::tuple first = hits[0];
    EXPECT_EQ(first[0].cast<std::string>(), "point");
    EXPECT_EQ(first[1].cast<int>(), 42);

    // Clean up so subsequent runs don't accumulate.
    py::module_::import("builtins").attr("_lc_test_hits") = py::list();
}

TEST(EventBus, ClearRemovesAll) {
    lcs::EventBus bus;
    auto cb = lcs::nativeCallback([]() {});
    bus.registerEvent("a", cb);
    bus.registerEvent("b", cb);
    bus.registerEvent("c", cb);
    EXPECT_EQ(bus.eventCount(), 3u);

    bus.clear();
    EXPECT_EQ(bus.eventCount(), 0u);
    EXPECT_EQ(bus.listenerCount("a"), 0u);
}

// py_lc_event.cpp — Python port of lcadluascript/bridge/lc_event.cpp.
//
// The kernel event types (AddLayerEvent, AddEntityEvent, ...) are the value
// objects fired through nano-signal-slot from Document*/StorageManagerImpl.
// Every one is a plain value with a ctor taking one or two _CSPtr metas.
// Kaguya binds them with UserdataMetatable; pybind11 uses py::class_.

#include "py_lc_event.h"

#include <cad/events/addentityevent.h>
#include <cad/events/addlayerevent.h>
#include <cad/events/addlinepatternevent.h>
#include <cad/events/commitprocessevent.h>
#include <cad/events/newwaitingcustomentityevent.h>
#include <cad/events/removeentityevent.h>
#include <cad/events/removelayerevent.h>
#include <cad/events/removelinepatternevent.h>
#include <cad/events/replaceentityevent.h>
#include <cad/events/replacelayerevent.h>
#include <cad/events/replacelinepatternevent.h>
#include <cad/primitive/insert.h>
#include <cad/primitive/point.h>

namespace py = pybind11;

namespace lc {
namespace python {

// -----------------------------------------------------------------------------
// Phase 5 PR-5.1 — event register/deregister hook storage.
// -----------------------------------------------------------------------------
// A single process-global hook per direction.  lcUI installs both at
// MainWindow construction (see lcUI/python/pyeventhooks.cpp) so
// `lc.event.register("point", self)` reaches the LuaInterface's
// EventBus.  Unset hook = silent no-op (headless CLI mode).

namespace {

EventHook& registerHookSlot() {
    static EventHook h;
    return h;
}

EventHook& deregisterHookSlot() {
    static EventHook h;
    return h;
}

} // namespace

void setEventRegisterHook(EventHook hook) {
    registerHookSlot() = std::move(hook);
}

void setEventDeregisterHook(EventHook hook) {
    deregisterHookSlot() = std::move(hook);
}

void import_py_lc_event_namespace(py::module_& m_event) {
    // Phase 5 PR-5.1 — `lc.event.register(name, obj)` /
    // `lc.event.deregister(name, obj)` bindings.  Route through the
    // hook slots (installed by lcUI); silent no-op if unset.
    m_event.def("register",
        [](const std::string& name, py::object obj) {
            auto& hook = registerHookSlot();
            if (hook) hook(name, std::move(obj));
        },
        py::arg("name"), py::arg("callback"),
        "Register a Python callback for the named event.  The callback "
        "may be a plain callable (`fn(event, args)`) or an object with "
        "an `onEvent(self, event, args)` method — the underlying "
        "PythonCallbackImpl chooses the shape per call.  Multiple "
        "registrations of the same object stack (matches Lua's "
        "std::vector<LuaRef> behavior).");

    m_event.def("deregister",
        [](const std::string& name, py::object obj) {
            auto& hook = deregisterHookSlot();
            if (hook) hook(name, std::move(obj));
        },
        py::arg("name"), py::arg("callback"),
        "Remove the first matching registration for the named event.  "
        "Equality follows PythonCallbackImpl's is-or-equal rule: "
        "identical py::object (same instance) OR __eq__ True.  Silent "
        "no-op on miss.");


    py::class_<lc::event::AddLayerEvent>(m_event, "AddLayerEvent")
        .def(py::init<const lc::meta::Layer_CSPtr>())
        .def("layer", &lc::event::AddLayerEvent::layer);

    py::class_<lc::event::RemoveLayerEvent>(m_event, "RemoveLayerEvent")
        .def(py::init<const lc::meta::Layer_CSPtr>())
        .def("layer", &lc::event::RemoveLayerEvent::layer);

    py::class_<lc::event::ReplaceLayerEvent>(m_event, "ReplaceLayerEvent")
        .def(py::init<const lc::meta::Layer_CSPtr, const lc::meta::Layer_CSPtr>())
        .def("newLayer", &lc::event::ReplaceLayerEvent::newLayer)
        .def("oldLayer", &lc::event::ReplaceLayerEvent::oldLayer);

    py::class_<lc::event::NewWaitingCustomEntityEvent>(m_event, "NewWaitingCustomEntityEvent")
        .def(py::init<const lc::entity::Insert_CSPtr&>())
        .def("insert", &lc::event::NewWaitingCustomEntityEvent::insert);

    py::class_<lc::event::CommitProcessEvent>(m_event, "CommitProcessEvent")
        .def(py::init<lc::operation::DocumentOperation_SPtr>())
        .def("operation", &lc::event::CommitProcessEvent::operation);

    py::class_<lc::event::AddEntityEvent>(m_event, "AddEntityEvent")
        .def(py::init<lc::entity::CADEntity_CSPtr>())
        .def("entity", &lc::event::AddEntityEvent::entity);

    py::class_<lc::event::RemoveEntityEvent>(m_event, "RemoveEntityEvent")
        .def(py::init<const lc::entity::CADEntity_CSPtr>())
        .def("entity", &lc::event::RemoveEntityEvent::entity);

    py::class_<lc::event::ReplaceEntityEvent>(m_event, "ReplaceEntityEvent")
        .def(py::init<const lc::entity::CADEntity_CSPtr>())
        .def("entity", &lc::event::ReplaceEntityEvent::entity);

    py::class_<lc::event::AddLinePatternEvent>(m_event, "AddLinePatternEvent")
        .def(py::init<const lc::meta::DxfLinePatternByValue_CSPtr>())
        .def("linePattern", &lc::event::AddLinePatternEvent::linePattern);

    py::class_<lc::event::RemoveLinePatternEvent>(m_event, "RemoveLinePatternEvent")
        .def(py::init<const lc::meta::DxfLinePatternByValue_CSPtr>())
        .def("linePattern", &lc::event::RemoveLinePatternEvent::linePattern);

    py::class_<lc::event::ReplaceLinePatternEvent>(m_event, "ReplaceLinePatternEvent")
        .def(py::init<const lc::meta::DxfLinePatternByValue_CSPtr,
                      const lc::meta::DxfLinePatternByValue_CSPtr>())
        .def("newLinePattern", &lc::event::ReplaceLinePatternEvent::newLinePattern)
        .def("oldLinePattern", &lc::event::ReplaceLinePatternEvent::oldLinePattern);
}

} // namespace python
} // namespace lc

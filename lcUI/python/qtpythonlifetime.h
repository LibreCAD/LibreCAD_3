#pragma once

#include <qt_keywords_push.h>
#include <pybind11/pybind11.h>
#include <qt_keywords_pop.h>
#include <QCoreApplication>
#include <QPointer>
#include <QThread>
#include <functional>
#include <memory>
#include <stdexcept>

namespace lc { namespace ui {
class LuaInterface;
namespace python {

class QtUiThreadCallGuard {
public:
    QtUiThreadCallGuard() {
        auto* app = QCoreApplication::instance();
        if (!app) throw std::runtime_error("LibreCAD GUI bindings require a Qt application instance");
        if (QThread::currentThread() != app->thread())
            throw std::runtime_error("LibreCAD GUI bindings must be called from the Qt application thread");
    }
};

constexpr const char* qtLifetimeKey = "_lc_qobject_lifetime";
struct QtLifetimeState {
    QPointer<QObject> object;
    std::function<void()> releaseToQt;
    void adoptIfParented() const {
        if (object && object->parent()) releaseToQt();
    }
};
using QtLifetime = std::shared_ptr<QtLifetimeState>;

inline pybind11::object instanceDictionary(pybind11::handle object) {
    auto* dictionary = PyObject_GenericGetDict(object.ptr(), nullptr);
    if (!dictionary && PyErr_ExceptionMatches(PyExc_AttributeError)) {
        PyErr_Clear();  // Scalars and non-dynamic kernel objects have no dict.
        return pybind11::none();
    }
    if (!dictionary) throw pybind11::error_already_set();
    return pybind11::reinterpret_steal<pybind11::object>(dictionary);
}

inline QtLifetime qtLifetime(pybind11::handle object) {
    if (!object) return {};
    auto dictionary = instanceDictionary(object);
    if (dictionary.is_none()) return {};
    auto* capsule = PyDict_GetItemString(dictionary.ptr(), qtLifetimeKey);
    if (!capsule) return {};  // Includes not-yet-initialized constructor self.
    auto* lifetime = static_cast<QtLifetime*>(PyCapsule_GetPointer(capsule, qtLifetimeKey));
    if (!lifetime) throw pybind11::error_already_set();
    return *lifetime;
}

inline void checkQtLifetime(pybind11::handle object) {
    auto lifetime = qtLifetime(object);
    if (!lifetime) return;
    if (lifetime->object.isNull())
        throw std::runtime_error("LibreCAD GUI handle refers to a destroyed Qt object");
    lifetime->adoptIfParented();
}

// Mixed InputGUIContainer bases are polymorphic; LuaInterface uses its window
// owner because it is not itself a QObject. No Qt ownership is transferred.
template<class T> QObject* qtLifetimeOwner(T* value) { return dynamic_cast<QObject*>(value); }
QObject* qtLifetimeOwner(LuaInterface* value);

namespace {
template<class T, class... Options>
class QtClass : public pybind11::class_<T, Options...> {
    using Base = pybind11::class_<T, Options...>;
    using Initializer = void (*)(pybind11::detail::instance*, const void*);
    static Initializer& initializer() { static Initializer value = nullptr; return value; }

    static void initialize(pybind11::detail::instance* instance, const void* holder) {
        // These hooks follow the pinned pybind11 type-info implementation.
        initializer()(instance, holder);
        auto* info = pybind11::detail::get_type_info(typeid(T));
        auto value = instance->get_value_and_holder(info);
        auto* owner = qtLifetimeOwner(value.template value_ptr<T>());
        if (!owner) return;
        auto lifetime = std::make_shared<QtLifetimeState>();
        lifetime->object = owner;
        lifetime->releaseToQt = [instance, info]() {
            auto value = instance->get_value_and_holder(info);
            if (value.holder_constructed())
                value.template holder<typename Base::holder_type>().release();
        };
        // Relinquish the owning holder as soon as Qt adopts a widget, not
        // only at QObject::destroyed: a derived destructor can first release
        // a callback that holds the last Python reference to its own widget.
        lifetime->adoptIfParented();
        pybind11::setattr(pybind11::handle(reinterpret_cast<PyObject*>(instance)),
            qtLifetimeKey, pybind11::capsule(new QtLifetime(lifetime), qtLifetimeKey,
                [](void* pointer) { delete static_cast<QtLifetime*>(pointer); }));
        QObject::connect(owner, &QObject::destroyed,
            [weak = std::weak_ptr<QtLifetimeState>(lifetime), instance, info]() {
                // Native Qt teardown need not already hold the GIL. Acquire
                // it before locking the weak state or touching wrapper data.
                pybind11::gil_scoped_acquire gil;
                auto lifetime = weak.lock();
                if (!lifetime) return;
                auto value = instance->get_value_and_holder(info);
                if (value.instance_registered()) {
                    // Forget the dead pointer so address reuse cannot return
                    // this stale wrapper for a new widget. During Python
                    // deallocation it may already have been deregistered.
                    pybind11::detail::deregister_instance(instance, value.value_ptr(), info);
                    value.set_instance_registered(false);
                }
                lifetime->releaseToQt();
            });
    }

public:
    template<class... Extra>
    QtClass(pybind11::handle scope, const char* name, const Extra&... extra)
        : Base(scope, name, pybind11::dynamic_attr(), extra...) {
        auto* info = pybind11::detail::get_type_info(typeid(T));
        initializer() = info->init_instance;
        info->init_instance = initialize;
    }
};
} // namespace

}}} // namespace lc::ui::python

namespace pybind11 { namespace detail {
template<>
struct process_attribute<call_guard<lc::ui::python::QtUiThreadCallGuard>>
    : process_attribute_default<call_guard<lc::ui::python::QtUiThreadCallGuard>> {
    static void precall(function_call& call) {
        lc::ui::python::QtUiThreadCallGuard thread;
        // New-style constructors put a native value_and_holder pointer in
        // slot zero, not a PyObject. Their remaining arguments are ordinary
        // Python handles and still require lifetime checks.
        for (std::size_t index = call.init_self ? 1 : 0; index < call.args.size(); ++index)
            lc::ui::python::checkQtLifetime(call.args[index]);
    }
    static void postcall(function_call& call, handle result) {
        // Bound adoption calls can give an existing Python-created widget
        // its first Qt parent. Their arguments remain alive until this hook.
        for (std::size_t index = call.init_self ? 1 : 0; index < call.args.size(); ++index)
            if (auto lifetime = lc::ui::python::qtLifetime(call.args[index]))
                lifetime->adoptIfParented();
        if (result && result.ptr() != PYBIND11_TRY_NEXT_OVERLOAD)
            if (auto lifetime = lc::ui::python::qtLifetime(result))
                lifetime->adoptIfParented();
    }
};
}} // namespace pybind11::detail

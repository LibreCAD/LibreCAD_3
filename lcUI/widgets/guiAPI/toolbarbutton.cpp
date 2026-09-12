#include "toolbarbutton.h"

#include "toolbargroup.h"

#include <utility>

using namespace lc::ui::api;

ToolbarButton::ToolbarButton(const char* buttonLabel, const char* icon,
                             lc::scripting::ScriptCallback callback,
                             const char* tooltip, bool _checkable,
                             QWidget* parent, const char* fallbackDir)
    :
    ToolbarButton(buttonLabel, icon, tooltip, _checkable, parent, fallbackDir)
{
    callbacks.push_back(std::move(callback));
}

ToolbarButton::ToolbarButton(const char* buttonLabel, const char* icon,
                             const char* tooltip, bool _checkable,
                             QWidget* parent, const char* fallbackDir)
    :
    _label(buttonLabel),
    QPushButton("", parent),
    _checkable(_checkable)
{
    this->setObjectName(buttonLabel);
    // tooltip is a const char*, so `tooltip == ""` compared addresses and was
    // effectively never true; test the string itself, and tolerate nullptr.
    if (tooltip == nullptr || *tooltip == '\0') {
        this->setToolTip(buttonLabel);
    }
    else {
        this->setToolTip(tooltip);
    }
    this->setFlat(true);

    if (_checkable) {
        this->setCheckable(true);
    }

    changeIcon(icon, fallbackDir);

    if (_checkable) {
        connect(this, &ToolbarButton::toggled, this, &ToolbarButton::callbackCalledToggle);
    }
    else {
        connect(this, &ToolbarButton::clicked, this, &ToolbarButton::callbackCalled);
    }
}

std::string ToolbarButton::label() const {
    return _label;
}

void ToolbarButton::setLabel(const char* newLabel) {
    _label = std::string(newLabel);
}

void ToolbarButton::setTooltip(const char* newToolTip) {
    this->setToolTip(newToolTip);
}

void ToolbarButton::addCallback(lc::scripting::ScriptCallback callback) {
    callbacks.push_back(std::move(callback));
}

void ToolbarButton::changeIcon(const char* icon, const char* fallbackDir) {
    if (icon == nullptr) return;

    QIcon resolved(icon);
    // Phase 5 PR-5.4 — icon file-path fallback.  When the qrc lookup
    // misses (QIcon silently constructs an empty icon), try
    // `<fallbackDir>/icons/<basename>`.  Strip a leading `:/icons/` if
    // present so callers can pass the same string they'd pass in the
    // qrc-only world.  Benefits both Python operations (fallbackDir =
    // registering module's directory) and Lua plugins (fallbackDir =
    // plugin directory).
    if (resolved.isNull() && fallbackDir != nullptr && *fallbackDir != '\0') {
        QString base = QString::fromUtf8(icon);
        const QString qrcPrefix = QStringLiteral(":/icons/");
        if (base.startsWith(qrcPrefix)) {
            base = base.mid(qrcPrefix.length());
        }
        QString fallbackPath =
            QString::fromUtf8(fallbackDir) + QStringLiteral("/icons/") + base;
        resolved = QIcon(fallbackPath);
    }

    this->setIcon(resolved);
    this->setIconSize(QSize(24, 24));
}

void ToolbarButton::remove() {
    emit removeButton(this);
}

bool ToolbarButton::checkable() const {
    return _checkable;
}

lc::scripting::ScriptCallback& ToolbarButton::getCallback(int index) {
    return callbacks[index];
}

void ToolbarButton::addCallback(const char* cb_name, lc::scripting::ScriptCallback callback) {
    namedCallbacks[cb_name] = callbacks.size();
    addCallback(std::move(callback));
}

void ToolbarButton::removeCallback(const char* cb_name) {
    callbacks.erase(callbacks.begin() + namedCallbacks[cb_name]);
    namedCallbacks.erase(namedCallbacks.find(cb_name));
}

void ToolbarButton::callbackCalled() {
    for (size_t i = 0; i < callbacks.size(); i++) {
        // Nil-safe: nil callback's invoke() is a graceful no-op returning Nil.
        callbacks[i].invoke();
    }
}

void ToolbarButton::callbackCalledToggle(bool enabled) {
    for (size_t i = 0; i < callbacks.size(); i++) {
        callbacks[i].call(enabled);
    }
}

ToolbarButton* ToolbarButton::clone() {
    ToolbarButton* clonedButton = new ToolbarButton(_label.c_str(), "", this->toolTip().toStdString().c_str(), _checkable);
    clonedButton->setIcon(this->icon());

    // Copy callbacks by value — ScriptCallback holds a shared_ptr pImpl,
    // so this is a refcount bump not a deep copy.  CustomizeToolbar
    // drag/drop retest applies (retested manually per PR-3 sub-plan).
    for (const auto& cb : callbacks) {
        clonedButton->addCallback(cb);
    }

    for (auto namedCb : namedCallbacks) {
        clonedButton->namedCallbacks[namedCb.first] = namedCb.second;
    }

    return clonedButton;
}

#include "hookmanager.h"

using namespace lc::ui;

void HookEvent::clear() {
    keyEvent = nullptr;
    mouseEvent = nullptr;
    eventType = "";
    free();
}

void HookEvent::grab() {
    nextDispatch=dispatchedTo;
}

void HookEvent::free() {
    nextDispatch=nullptr;
}

void HookManager::onKeyEvent(QKeyEvent* keyEvent) {
    hookEvent.keyEvent = keyEvent;
    hookEvent.eventType = "keyEvent";
    dispatch();
};

void HookManager::onMouseEvent(std::string type,QMouseEvent* mouseEvent) {
    hookEvent.mouseEvent = mouseEvent;
    hookEvent.eventType = type+"MouseEvent";
    dispatch();
};

void HookManager::dispatch() {
    if(!hookEvent.nextDispatch) {
        bool lastResult = false; //handled?
        for(auto& i: _handleVector) {
            // If last result was not false
            // !lastResult => handled previously
            // hookEvent.nextDispatch()=> check if grab is called
            if (lastResult || hookEvent.nextDispatch)
                return;
            hookEvent.dispatchedTo = i.second;//to enable grab
            lastResult = i.second(hookEvent);
        }
    } else {
        hookEvent.nextDispatch(hookEvent);
    }
}

void HookManager::append(std::string name, hookCallback cb) {
    _handleVector.insert(std::pair<std::string, hookCallback>(name, cb));
}

void HookManager::selectState(std::string name) {
    // It is ok to crash if no value is found since the input is not available to user
    hookEvent.nextDispatch = _handleVector[name];
}

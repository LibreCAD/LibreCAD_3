#include "operationmanager.h"

#include "cad/base/metainfo.h"
#include "guioperationfinishedevent.h"

OperationManager::OperationManager( std::shared_ptr<lc::Document> document) : _document(document) {
}

void OperationManager::cancel() {
    _activeGuiOperations.clear();
}

void OperationManager::startOperation(GuiOperation_SPtr operation) {

    //    if (_activeGuiOperations.count()>0) {
    //        throw "We currently only support one operation";
    //    }

    _activeGuiOperations.clear();
    _activeGuiOperations.append(operation);

    connect(operation.get(), SIGNAL(guiOperationFinished(const GuiOperationFinishedEvent&)),
            this, SLOT(on_guioperationFinished_Event(const GuiOperationFinishedEvent&)));
}

void OperationManager::restart() {
    if (!_activeGuiOperations.empty()) {
        GuiOperation_SPtr lastOperation = _activeGuiOperations.top();
        lastOperation->restart();
    }

}

QStack<GuiOperation_SPtr > OperationManager::activeOperations() const {
    return this->_activeGuiOperations;
}

void OperationManager::on_guioperationFinished_Event(const GuiOperationFinishedEvent& event) {

    GuiOperation_SPtr lastOperation = _activeGuiOperations.top();
    event.guiOperation().operation()->execute();
    _activeGuiOperations.clear();
    startOperation(lastOperation->next());
}

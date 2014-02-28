#include "operationmanager.h"

#include "cad/operations/createentities.h"
#include "cad/base/metainfo.h"
#include "guioperationfinishedevent.h"

OperationManager::OperationManager(lc::AbstractDocument* document) : _document(document) {
}

void OperationManager::cancel() {
    _activeGuiOperations.clear();
}

void OperationManager::startOperation(boost::shared_ptr<GuiOperation> operation) {

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
        boost::shared_ptr<GuiOperation> lastOperation = _activeGuiOperations.top();
        lastOperation->restart();
    }

}

QStack<boost::shared_ptr<GuiOperation> > OperationManager::activeOperations() const {
    return this->_activeGuiOperations;
}

void OperationManager::on_guioperationFinished_Event(const GuiOperationFinishedEvent& event) {

    boost::shared_ptr<GuiOperation> lastOperation = _activeGuiOperations.top();
    _document->operateOn(event.guiOperation().operation());
    _activeGuiOperations.clear();
    startOperation(lastOperation->next());
}

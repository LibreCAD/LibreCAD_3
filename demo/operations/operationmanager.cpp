#include "operationmanager.h"

#include "cad/operations/createentities.h"
#include "cad/base/metainfo.h"

OperationManager::OperationManager(lc::AbstractDocument* document) : _document(document) {
}

void OperationManager::cancel() {
    _activeOperations.clear();
}

void OperationManager::startOperation(std::tr1::shared_ptr<GuiOperation> operation) {
    _activeOperations.clear();
    _activeOperations.append(operation);

    connect(operation.get(), SIGNAL(guiOperationFinished(const GuiOperationFinishedEvent&)),
            this, SLOT(on_guioperationFinished_Event(const GuiOperationFinishedEvent&)));


}

void OperationManager::restart() {
    if (!_activeOperations.empty()) {
        std::tr1::shared_ptr<GuiOperation> lastOperation = _activeOperations.top();
        lastOperation->restart();
    }

}

QStack<std::tr1::shared_ptr<GuiOperation> > OperationManager::activeOperations() const {
    return this->_activeOperations;
}

void OperationManager::setActiveOperations(QStack<std::tr1::shared_ptr<GuiOperation> > operations) {
    _activeOperations = operations;
}

void OperationManager::on_guioperationFinished_Event(const GuiOperationFinishedEvent& event) {
    lc::CreateEntities* foo = new  lc::CreateEntities(_document, "0");

    std::tr1::shared_ptr<GuiOperation> lastOperation = _activeOperations.top();

    for (int i = 0; i < _activeOperations.count(); i++) {
        std::tr1::shared_ptr<GuiOperation> o = _activeOperations.at(0);
        QList<std::tr1::shared_ptr<const lc::MetaType> > metaTypes;
        foo->append(o->cadEntity(metaTypes));
    }

    _activeOperations.clear();
    _document->operateOn(std::tr1::shared_ptr<lc::Operation>(foo));

    startOperation(lastOperation->next());
}

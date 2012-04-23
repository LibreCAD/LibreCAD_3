#include "operationmanager.h"

#include "cad/operations/createentities.h"
#include "cad/base/metainfo.h"

OperationManager::OperationManager(lc::AbstractDocument* document) : _document(document) {
}

void OperationManager::cancel() {
    _activeOperations.clear();
}

void OperationManager::startOperation(OperationPtr operation) {
    _activeOperations.clear();
    _activeOperations.append(operation);
    connect(operation.get(), SIGNAL(operationFinished(const OperationFinishedEvent&)),
            this, SLOT(on_operationFinished_Event(const OperationFinishedEvent&)));


}

void OperationManager::restart() {
    if (!_activeOperations.empty()) {
        OperationPtr lastOperationPtr = _activeOperations.top();
        lastOperationPtr->restart();
    }

}

QStack<OperationPtr> OperationManager::activeOperations() const {
    return this->_activeOperations;
}

void OperationManager::setActiveOperations(QStack<OperationPtr> operations) {
    _activeOperations = operations;
}

void OperationManager::on_operationFinished_Event(const OperationFinishedEvent& event) {
    lc::CreateEntities* foo = new  lc::CreateEntities(_document, "0");

    OperationPtr lastOperationPtr = _activeOperations.top();

    for (int i = 0; i < _activeOperations.count(); i++) {
        OperationPtr o = _activeOperations.at(0);
        QList<lc::MetaTypePtr> metaTypes;
        foo->append(o->cadEntity(metaTypes));
    }

    _activeOperations.clear();
    _document->operateOn(shared_ptr<lc::Operation>(foo));

    startOperation(lastOperationPtr->next());
}

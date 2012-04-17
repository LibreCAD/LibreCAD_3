#include "operationstackmanager.h"

OperationStackManager::OperationStackManager() {
}



void OperationStackManager::startOperation(OperationPtr operation) {
    _activeOperations.append(operation);
}

void OperationStackManager::undo() {
    if (!_activeOperations.empty()) {
        OperationPtr lastOperationPtr = _activeOperations.top();
        lastOperationPtr->undo();
    } else {
        throw "Trying to undo a operation that doesn't exist, the stack is empty";
    }

}

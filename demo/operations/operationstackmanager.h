#ifndef OPERATIONSTACKMANAGER_H
#define OPERATIONSTACKMANAGER_H

#include <QStack>
#include "operation.h"

class OperationStackManager {
    public:
        OperationStackManager();

        /**
          * Call this function with a initialised operation
          *
          */
        void startOperation(OperationPtr operation);
        void undo();

    private:
        QStack<OperationPtr> _activeOperations;
};

#endif // OPERATIONSTACKMANAGER_H

#ifndef COMMITPROCESSEVENT_H
#define COMMITPROCESSEVENT_H

#include "cad/const.h"
#include "cad/operations/operation.h"

namespace lc {

    /**
     * Event that get's emitted when a operation has been comitted
     */
    class CommitProcessEvent {
        public:
            CommitProcessEvent(OperationPtr operation) : _operation(operation) {
            };

            OperationPtr operation() const {
                return _operation;
            }
        private:
            const OperationPtr _operation;
    };
}
#endif // COMMITPROCESSEVENT_H

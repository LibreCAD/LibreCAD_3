#ifndef COMMITPROCESSEVENT_H
#define COMMITPROCESSEVENT_H

#include "cad/const.h"

namespace lc {

    class Operation;

    /**
     * Event that get's emitted when a operation has been comitted
     */
    class CommitProcessEvent {
        public:
            CommitProcessEvent(OperationPtr operation) {
                _operation = operation;
            };

            OperationPtr operation() const {
                return _operation;
            }
        private:
            OperationPtr _operation;
    };
}
#endif // COMMITPROCESSEVENT_H

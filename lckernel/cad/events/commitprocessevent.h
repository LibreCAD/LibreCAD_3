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
            CommitProcessEvent(shared_ptr<lc::Operation> operation) : _operation(operation) {
            };

            shared_ptr<lc::Operation> operation() const {
                return _operation;
            }
        private:
            const shared_ptr<lc::Operation> _operation;
    };
}
#endif // COMMITPROCESSEVENT_H

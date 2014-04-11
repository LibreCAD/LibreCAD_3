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
            CommitProcessEvent(shared_ptr<operation::Operation> operation) : _operation(operation) {
            };
            /*!
             * \brief operation
             * \return operation::Operation Operation.
             */
            shared_ptr<operation::Operation> operation() const {
                return _operation;
            }
        private:
            const shared_ptr<operation::Operation> _operation; /*!< operation _operation */
    };
}
#endif // COMMITPROCESSEVENT_H

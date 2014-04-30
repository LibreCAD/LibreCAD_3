#ifndef COMMITPROCESSEVENT_H
#define COMMITPROCESSEVENT_H

#include "cad/const.h"
#include "cad/operations/documentoperation.h"

namespace lc {

    /**
     * Event that get's emitted when a operation has been comitted
     */
    class CommitProcessEvent {
        public:
            CommitProcessEvent(std::shared_ptr<operation::DocumentOperation> operation) : _operation(operation) {
            };
            /*!
             * \brief operation
             * \return operation::DocumentOperation Operation.
             */
            std::shared_ptr<operation::DocumentOperation> operation() const {
                return _operation;
            }
        private:
            const std::shared_ptr<operation::DocumentOperation> _operation; /*!< operation _operation */
    };
}
#endif // COMMITPROCESSEVENT_H

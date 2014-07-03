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
            CommitProcessEvent(operation::DocumentOperation_SPtr operation) : _operation(operation) {
            }
            /*!
             * \brief operation
             * \return operation::DocumentOperation Operation.
             */
            operation::DocumentOperation_SPtr operation() const {
                return _operation;
            }
        private:
            const operation::DocumentOperation_SPtr _operation; /*!< operation _operation */
    };
}
#endif // COMMITPROCESSEVENT_H

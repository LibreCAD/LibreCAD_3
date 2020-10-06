#pragma once

#include "cad/const.h"
#include "cad/operations/documentoperation.h"

namespace lc {
namespace event {
/**
 * Event that gets emitted when a operation has been committed
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
}

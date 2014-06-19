#ifndef GUIOPERATIONFINISHEDEVENT_H
#define GUIOPERATIONFINISHEDEVENT_H

#include "guioperation.h"

/*!
 * \brief When a operstion is finished a operstion nedds to send this event to notivy the operation manager that it can further process the data
 *
 */
class GuiOperationFinishedEvent {
    public:

        GuiOperationFinishedEvent(const GuiOperation& guiOperation) : _guiOperation(guiOperation) {       }
        const GuiOperation& guiOperation() const {
            return _guiOperation;
        }
    private:
        const GuiOperation& _guiOperation;
};

#endif // GUIOPERATIONFINISHEDEVENT_H

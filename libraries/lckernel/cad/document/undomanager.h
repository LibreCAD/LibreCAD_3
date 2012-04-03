#ifndef UNDOMANAGER_H
#define UNDOMANAGER_H

#include <QString>

#include "cad/const.h"

#include "basemanager.h"

namespace lc {
    /**
     * Undo manager is a Interface for Undo operations
     */
    class UndoManager: public BaseManager {
            Q_OBJECT
        public:
            virtual void undo() = 0;
            virtual void redo() = 0;

            virtual bool canUndo() const = 0;
            virtual bool canRedo() const = 0;

            virtual void removeUndoables() = 0;
    };

}

#endif // UNDOMANAGER_H

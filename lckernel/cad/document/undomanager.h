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
            /*!
             * \brief undo
             */
            virtual void undo() = 0;
            /*!
             * \brief redo
             */
            virtual void redo() = 0;
            /*!
             * \brief canUndo
             */
            virtual bool canUndo() const = 0;
            /*!
             * \brief canRedo
             */
            virtual bool canRedo() const = 0;
            /*!
             * \brief Clear the undo/redo stack.
             */
            virtual void removeUndoables() = 0;

        private:


    };

}

#endif // UNDOMANAGER_H

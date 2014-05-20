#ifndef UNDOMANAGER_H
#define UNDOMANAGER_H

#include "cad/const.h"

namespace lc {
    /**
     * Undo manager is a Interface for Undo operations
     */
    class UndoManager {
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

    };
    typedef std::shared_ptr<UndoManager> UndoManager_SPtr;
    typedef std::shared_ptr<const UndoManager> UndoManager_CSPtr;

}

#endif // UNDOMANAGER_H

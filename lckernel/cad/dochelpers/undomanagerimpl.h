#pragma once

#include <stack>
#include <vector>

#include "cad/const.h"

#include "cad/document/document.h"
#include "cad/document/undomanager.h"
#include "cad/events/commitprocessevent.h"
#include "cad/operations/undoable.h"

namespace lc {
    /**
     * UndoManagerImpl manages a stack of operations and allows for
     * undo or re-do operations that where done on a canvas
     * @param maximumUndoLevels
     */
    class UndoManagerImpl: public UndoManager {
        public:
            UndoManagerImpl(unsigned int maximumUndoLevels);

            /*!
             * \brief redo an operation.
             * \sa lc::Undoable
             * \sa lc::UndoManager
             * \sa lc::UndoManagerImpl
             */
            virtual void redo();
            /*!
             * \brief undo an operation.
             * \sa lc::Undoable
             * \sa lc::UndoManager
             * \sa lc::UndoManagerImpl
             */
            virtual void undo();

            /*!
             * \brief Check if undo can be done
             * \sa lc::Undoable
             * \sa lc::UndoManager
             * \sa lc::UndoManagerImpl
             * \return
             */
            virtual bool canUndo() const;
            /*!
             * \brief Check if redo can be done
             * \sa lc::Undoable
             * \sa lc::UndoManager
             * \sa lc::UndoManagerImpl
             * \return
             */
            virtual bool canRedo() const;

            /*!
             * \brief Clears the Undo/Redo stack.
             * \sa lc::Undoable
             * \sa lc::UndoManager
             * \sa lc::UndoManagerImpl
             * \return
             */
            virtual void removeUndoables();

        private:
            std::vector<operation::Undoable_SPtr> _unDoables; /*!< Undo list */
            std::stack<operation::Undoable_SPtr> _reDoables; /*!< Redo stack */
            const unsigned int _maximumUndoLevels; /*!< Maximum undo level */

        public:
            void on_CommitProcessEvent(const lc::CommitProcessEvent& event);
    };

    DECLARE_SHORT_SHARED_PTR(UndoManagerImpl)
}

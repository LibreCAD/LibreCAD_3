#pragma once

#include "cad/const.h"

namespace lc {
    namespace storage {
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

        DECLARE_SHORT_SHARED_PTR(UndoManager)
    }
}

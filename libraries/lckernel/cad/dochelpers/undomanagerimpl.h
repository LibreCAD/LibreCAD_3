#ifndef UNDOMANAGERIMPL_H
#define UNDOMANAGERIMPL_H

#include <QStack>

#include "cad/const.h"

#include "cad/document/abstractdocument.h"
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
            Q_OBJECT
        public:
            UndoManagerImpl(int maximumUndoLevels);

            virtual void redo();
            virtual void undo();

            virtual void setDocument(AbstractDocument* document);

            virtual bool canUndo() const;
            virtual bool canRedo() const;

            virtual void removeUndoables();

        private:
            QList <UndoablePtr> _unDoables;
            QStack <UndoablePtr> _reDoables;
            int _maximumUndoLevels;

        public slots:
            void on_CommitProcessEvent(const lc::CommitProcessEvent& event);


    };

}

#endif // UNDOMANAGERIMPL_H

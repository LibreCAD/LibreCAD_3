#ifndef UNDOMANAGERIMPL_H
#define UNDOMANAGERIMPL_H

#include <QStack>

#include "cad/document/abstractdocument.h"
#include "cad/document/undomanager.h"
#include "cad/events/commitprocessevent.h"
#include "cad/operations/undoable.h"

namespace lc {

    class UndoManagerImpl: public UndoManager {
            Q_OBJECT
        public:
            UndoManagerImpl(int maximumUndoLevels);

            virtual void redo();
            virtual void undo();

            virtual void setDocument(AbstractDocument* document);

            virtual bool canUndo() const;
            virtual bool canRedo() const;

        private:
            QList <Undoable *> _unDoables;
            QStack <Undoable *> _reDoables;
            int _maximumUndoLevels;

            void manageQueue();
        public slots:
            void on_CommitProcessEvent(lc::CommitProcessEvent* commitProcessEvent);


    };

}

#endif // UNDOMANAGERIMPL_H

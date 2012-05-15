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
            UndoManagerImpl(lc::AbstractDocument* document, int maximumUndoLevels);

            virtual void redo();
            virtual void undo();

            virtual bool canUndo() const;
            virtual bool canRedo() const;

            virtual void removeUndoables();

        private:
            virtual AbstractDocument* document() const {
                return _document;
            }
            AbstractDocument* _document;

        private:
            QList <std::tr1::shared_ptr<lc::Undoable> > _unDoables;
            QStack <std::tr1::shared_ptr<lc::Undoable> > _reDoables;
            const int _maximumUndoLevels;

        public slots:
            void on_CommitProcessEvent(const lc::CommitProcessEvent& event);


    };

}

#endif // UNDOMANAGERIMPL_H

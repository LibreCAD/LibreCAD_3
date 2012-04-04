#ifndef UNDOABLE_H
#define UNDOABLE_H

#include <QString>

#include "cad/const.h"

namespace lc {
    class AbstractDocument;

    /**
     * Abstract class for a Undoable operations
     *
     * All operations need to implement all pure virtual functions
     *
     * @param text
     */
    class Undoable {
        public:
            Undoable(const QString& text) {
                _text = text;
            }
            virtual ~Undoable() {}

            /**
             * Undo a given operation.
             *
             * For any operation that means for example when it added entities to the document
             * it now needs to remove all created entites on the document.
             * When the operation added a layer or block, it will call functions to remove that block
             */
            virtual void undo() const = 0;

            /**
             * Redo a given operation
             *
             * This can get called if we want to redo a operation. Redo is ususally called after a undo
             * operation. If in the undo operatation entities where removed, it needs to get added again.
             * it doesn't have to do any re.calculation but just rememebr what entities have been added or removed.
             */
            virtual void redo() const = 0;

            /**
             * Return the name of the operation. For example when a operation started it can be given the name 'add lines'
             * So in the UI you can say 'Undo add lines' or 'Redo add lines'
             * @return
             */
            virtual QString text() {
                return _text;
            };

        private:
            QString _text;

    };
    typedef shared_ptr<lc::Undoable> UndoablePtr;

}

#endif // UNDOABLE_H

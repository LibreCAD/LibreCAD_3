#ifndef UNDOABLE_H
#define UNDOABLE_H

#include <QString>

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
            /**
             * Undo a given operation
             */
            virtual void undo() const = 0;

            /**
             * Redo a given operation
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
}

#endif // UNDOABLE_H

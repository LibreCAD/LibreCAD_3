#ifndef UNDOABLE_H
#define UNDOABLE_H

#include <QString>

namespace lc {
    class AbstractDocument;
    class Undoable {
        public:
            Undoable(const QString& text) {
                _text = text;
            }
            virtual void undo() const = 0;
            virtual void redo() const = 0;
            virtual QString text() {
                return _text;
            };
        private:
            QString _text;

    };
}

#endif // UNDOABLE_H

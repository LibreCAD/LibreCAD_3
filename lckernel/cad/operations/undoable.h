#ifndef UNDOABLE_H
#define UNDOABLE_H

#include <QString>
#include <QDebug>

#include "cad/const.h"

namespace lc {
    class AbstractDocument;
    namespace operation {

        /**
         * @brief: Abstract class for a Undoable operations
         * All operations you wnt to beable to get place in the undo stack should inherit from Undoable
         * and implement undo and redo.
         * A provided text can be used to show within the undo stack what operation will be re-done or un-done.
         *
         * @param text
         */
        class Undoable {
            public:
                /*!
                  * \brief Name of this operartion. Keep it as short as possible since it will be used in the UI
                  *
                  */
                Undoable(const QString& text) {
                    _text = text;
                }
                virtual ~Undoable() {
                    qDebug() << _text << " removed";
                }

                /*!
                 * \brief Undo a given operation.
                 *
                 * For any operation that means for example when it added entities to the document
                 * it now needs to remove all created entites on the document.
                 * When the operation added a layer or block, it will call functions to remove that block
                 */
                virtual void undo() const = 0;

                /*!
                 * \brief Redo a given operation
                 *
                 * This can get called if we want to redo a operation. Redo is ususally called after a undo
                 * operation. If in the undo operatation entities where removed, it needs to get added again.
                 * it doesn't have to do any re.calculation but just rememebr what entities have been added or removed.
                 */
                virtual void redo() const = 0;

                /*!
                 * \brief Name of the operation
                 *
                 * Return the name of the operation. For example when a operation started it can be given the name 'add lines'
                 * So in the UI you can say 'Undo add lines' or 'Redo add lines'
                 *
                 * @return
                 */
                virtual QString text() {
                    return _text;
                }

            private:
                QString _text;
        };
    };
}

#endif // UNDOABLE_H

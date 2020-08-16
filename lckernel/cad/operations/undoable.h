#pragma once

#include <string>
#include "cad/const.h"
#include <memory>
namespace lc {
namespace storage {
class Document;
}

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
    Undoable(const std::string& text) {
        _text = text;
    }
    virtual ~Undoable() {
        // LOG4CXX_DEBUG(logger, "Undoable removed:");
    }

    /*!
     * \brief Undo a given operation.
     *
     * For any operation that means for example when it added entities to the document
     * it now needs to remove all created entities on the document.
     * When the operation added a layer or block, it will call functions to remove that block
     */
    virtual void undo() const = 0;

    /*!
     * \brief Redo a given operation
     *
     * This can get called if we want to redo a operation. Redo is usually called after an undo
     * operation. If in the undo operation entities where removed, it needs to get added again.
     * it doesn't have to do any re.calculation but just remember what entities have been added or removed.
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
    virtual std::string text() {
        return _text;
    }

private:
    std::string _text;
};

DECLARE_SHORT_SHARED_PTR(Undoable)
}
}

// UNDOABLE_H

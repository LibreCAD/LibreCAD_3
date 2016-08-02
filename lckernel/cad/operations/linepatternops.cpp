//
// Created by R. van Twisk on 4/15/16.
//

#include "linepatternops.h"
#include "cad/document/document.h"

using namespace lc;
using namespace lc::operation;

/********************************************************************************************************/
/** AddLinePattern                                                                                          ***/
/********************************************************************************************************/
AddLinePattern::AddLinePattern(std::shared_ptr<Document> document,  const DxfLinePattern_CSPtr LinePattern) : DocumentOperation(document), Undoable("AddLinePattern"), _LinePattern(LinePattern)  {
}

void AddLinePattern::processInternal() {
    document()->addDocumentMetaType(_LinePattern);
}

void AddLinePattern::undo() const {
    document()->removeDocumentMetaType(_LinePattern);
}

void AddLinePattern::redo() const {
    document()->addDocumentMetaType(_LinePattern);
}

/********************************************************************************************************/
/** RemoveLinePattern                                                                                       ***/
/********************************************************************************************************/
RemoveLinePattern::RemoveLinePattern(std::shared_ptr<Document> document,  const DxfLinePattern_CSPtr LinePattern) : DocumentOperation(document), Undoable("RemoveLinePattern"), _LinePattern(LinePattern)  {
}

void RemoveLinePattern::processInternal() {

    document()->removeDocumentMetaType(_LinePattern);
}

void RemoveLinePattern::undo() const {
    document()->addDocumentMetaType(_LinePattern);
}

void RemoveLinePattern::redo() const {
    for (auto i : _entities) {
        document()->removeEntity(i);
    }

    document()->removeDocumentMetaType(_LinePattern);
}


/********************************************************************************************************/
/** ReplaceLinePattern                                                                                       ***/
/********************************************************************************************************/
ReplaceLinePattern::ReplaceLinePattern(std::shared_ptr<Document> document,  const DxfLinePattern_CSPtr oldLinePattern,  const DxfLinePattern_CSPtr newLinePattern) : DocumentOperation(document), Undoable("ReplaceLinePattern"), _oldLinePattern(oldLinePattern), _newLinePattern(newLinePattern)  {
}

void ReplaceLinePattern::processInternal() {
    document()->removeDocumentMetaType(_oldLinePattern);
    document()->addDocumentMetaType(_newLinePattern);
}

void ReplaceLinePattern::undo() const {
}

void ReplaceLinePattern::redo() const {
}

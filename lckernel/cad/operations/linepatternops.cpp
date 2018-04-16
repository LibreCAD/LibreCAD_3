//
// Created by R. van Twisk on 4/15/16.
//

#include "linepatternops.h"
#include "cad/document/document.h"

using namespace lc;
using namespace operation;

/********************************************************************************************************/
/** AddLinePattern                                                                                          ***/
/********************************************************************************************************/
AddLinePattern::AddLinePattern(Document_SPtr document, DxfLinePattern_CSPtr linePattern) :
        DocumentOperation(std::move(document), "AddLinePattern"),
        _linePattern(std::move(linePattern)) {
}

void AddLinePattern::processInternal() {
    document()->addDocumentMetaType(_linePattern);
}

void AddLinePattern::undo() const {
    document()->removeDocumentMetaType(_linePattern);
}

void AddLinePattern::redo() const {
    document()->addDocumentMetaType(_linePattern);
}

/********************************************************************************************************/
/** RemoveLinePattern                                                                                       ***/
/********************************************************************************************************/
RemoveLinePattern::RemoveLinePattern(Document_SPtr document, DxfLinePattern_CSPtr linePattern) :
        DocumentOperation(std::move(document), "RemoveLinePattern"),
        _linePattern(std::move(linePattern)) {
}

void RemoveLinePattern::processInternal() {
    document()->removeDocumentMetaType(_linePattern);
}

void RemoveLinePattern::undo() const {
    document()->addDocumentMetaType(_linePattern);
}

void RemoveLinePattern::redo() const {
    document()->removeDocumentMetaType(_linePattern);
}


/********************************************************************************************************/
/** ReplaceLinePattern                                                                                       ***/
/********************************************************************************************************/
ReplaceLinePattern::ReplaceLinePattern(Document_SPtr document,
                                       DxfLinePattern_CSPtr oldLinePattern,
                                       DxfLinePattern_CSPtr newLinePattern) :
        DocumentOperation(std::move(document), "ReplaceLinePattern"),
        _oldLinePattern(std::move(oldLinePattern)),
        _newLinePattern(std::move(newLinePattern)) {
}

void ReplaceLinePattern::processInternal() {
    document()->removeDocumentMetaType(_oldLinePattern);
    document()->addDocumentMetaType(_newLinePattern);
}

void ReplaceLinePattern::undo() const {
}

void ReplaceLinePattern::redo() const {
}
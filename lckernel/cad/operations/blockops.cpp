#include "blockops.h"

using namespace lc;
using namespace operation;

AddBlock::AddBlock(const Document_SPtr& document, const Block_CSPtr& block) :
    DocumentOperation(document, "AddBlock"),
    _block(block) {

}

void AddBlock::undo() const {
    document()->removeDocumentMetaType(_block);
}

void AddBlock::redo() const {
    document()->addDocumentMetaType(_block);
}

void AddBlock::processInternal() {
    document()->addDocumentMetaType(_block);
}


RemoveBlock::RemoveBlock(const Document_SPtr& document, const Block_CSPtr& block) :
    DocumentOperation(document, "RemoveBlock"),
    _block(block){

}

void RemoveBlock::undo() const {
    document()->addDocumentMetaType(_block);
}

void RemoveBlock::redo() const {
    document()->removeDocumentMetaType(_block);
}

void RemoveBlock::processInternal() {
    document()->removeDocumentMetaType(_block);
}


ReplaceBlock::ReplaceBlock(const Document_SPtr& document, const Block_CSPtr& oldBlock, const Block_CSPtr& newBlock) :
    DocumentOperation(document, "ReplaceBlock"),
    _oldBlock(oldBlock),
    _newBlock(newBlock) {

}

void ReplaceBlock::undo() const {
    document()->removeDocumentMetaType(_newBlock);
    document()->addDocumentMetaType(_oldBlock);
}

void ReplaceBlock::redo() const {
    document()->removeDocumentMetaType(_oldBlock);
    document()->addDocumentMetaType(_newBlock);
}

void ReplaceBlock::processInternal() {
    auto entities = document()->entitiesByBlock(_oldBlock).asVector();

    for(auto entity : entities) {
        document()->removeEntity(entity);
        document()->insertEntity(entity->modify(
                entity->layer(),
                entity->metaInfo(),
                _newBlock
        ));
    }

    document()->removeDocumentMetaType(_oldBlock);
    document()->addDocumentMetaType(_newBlock);

    //TODO: should we modify the inserts ?
}

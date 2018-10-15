#include "blockops.h"

using namespace lc;
using namespace operation;

AddBlock::AddBlock(const storage::Document_SPtr& document, meta::Block_CSPtr block) :
    DocumentOperation(document, "AddBlock"),
    _block(std::move(block)) {

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


RemoveBlock::RemoveBlock(const storage::Document_SPtr& document, meta::Block_CSPtr block) :
    DocumentOperation(document, "RemoveBlock"),
    _block(std::move(block)) {

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


ReplaceBlock::ReplaceBlock(const storage::Document_SPtr& document, meta::Block_CSPtr oldBlock, meta::Block_CSPtr newBlock) :
    DocumentOperation(document, "ReplaceBlock"),
    _oldBlock(std::move(oldBlock)),
    _newBlock(std::move(newBlock)) {

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

    for(const auto& entity : entities) {
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

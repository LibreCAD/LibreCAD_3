#include "insert.h"
#include <cad/primitive/insert.h>

using namespace lc;
using namespace builder;

InsertBuilder::InsertBuilder() :
        _displayBlock(nullptr) {
}

InsertBuilder* InsertBuilder::copy(const entity::Insert_CSPtr& insert) {
    CADEntityBuilder::copy(insert);

    _displayBlock = insert->_displayBlock;
    _document = insert->_document;
    _coordinate = insert->_position;

    return this;
}

bool InsertBuilder::checkValues() {
    return CADEntityBuilder::checkValues() &&
           _displayBlock != nullptr &&
           _document != nullptr;
}

const meta::Block_CSPtr& InsertBuilder::displayBlock() const {
    return _displayBlock;
}

InsertBuilder* InsertBuilder::setDisplayBlock(const meta::Block_CSPtr& displayBlock) {
    _displayBlock = displayBlock;

    return this;
}

entity::Insert_CSPtr InsertBuilder::build() {
    if(!checkValues()) {
        throw std::runtime_error("Missing values");
    }

    return entity::Insert_CSPtr(new entity::Insert(*this));
}

const geo::Coordinate& InsertBuilder::coordinate() const {
    return _coordinate;
}

InsertBuilder* InsertBuilder::setCoordinate(const geo::Coordinate& coordinate) {
    _coordinate = coordinate;

    return this;
}

const storage::Document_SPtr& InsertBuilder::document() const {
    return _document;
}

InsertBuilder* InsertBuilder::setDocument(const storage::Document_SPtr& document) {
    _document = document;

    return this;
}

#include "insert.h"
#include <cad/primitive/insert.h>

using namespace lc;
using namespace builder;

InsertBuilder::InsertBuilder() :
        _displayBlock(nullptr) {
}

bool InsertBuilder::checkValues() {
    return CADEntityBuilder::checkValues() &&
           _displayBlock != nullptr &&
           _document != nullptr;
}

const Block_CSPtr& InsertBuilder::displayBlock() const {
    return _displayBlock;
}

InsertBuilder* InsertBuilder::setDisplayBlock(const Block_CSPtr& displayBlock) {
    _displayBlock = displayBlock;

    return this;
}

entity::Insert_CSPtr InsertBuilder::build() {
    if(!checkValues()) {
        throw "Missing values";
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

const Document_SPtr& InsertBuilder::document() const {
    return _document;
}

InsertBuilder* InsertBuilder::setDocument(const Document_SPtr& document) {
    _document = document;

    return this;
}

#include "block.h"

using namespace lc::meta;

Block::Block(std::string name, lc::geo::Coordinate base) :
    _name(std::move(name)),
    _base(std::move(base)) {
}

const std::string Block::id() const {
    return BLOCK_ID_PREFIX "_" + _name;
}

const std::string Block::name() const {
    return _name;
}

const lc::geo::Coordinate& Block::base() const {
    return _base;
}
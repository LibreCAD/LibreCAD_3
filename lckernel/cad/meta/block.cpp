#include "block.h"

lc::Block::Block(std::string name, lc::geo::Coordinate base) :
        _name(std::move(name)),
        _base(std::move(base)) {
}

const std::string lc::Block::id() const {
    return BLOCK_ID_PREFIX "_" + _name;
}

const std::string lc::Block::name() const {
    return _name;
}

const lc::geo::Coordinate& lc::Block::base() const {
    return _base;
}
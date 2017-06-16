#include "block.h"

lc::Block::Block(std::string name, lc::geo::Coordinate base,
                 lc::MetaColor_CSPtr color, lc::MetaLineWidth_CSPtr lineWidth, lc::DxfLinePattern_CSPtr linePattern) :
        _name(name),
        _base(base),
        _color(color),
        _lineWidth(lineWidth),
        _linePattern(linePattern) {
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

const lc::MetaColor_CSPtr& lc::Block::color() const {
    return _color;
}

const lc::EntityMetaType_CSPtr& lc::Block::lineWidth() const {
    return _lineWidth;
}

const lc::DxfLinePattern_CSPtr& lc::Block::linePattern() const {
    return _linePattern;
}

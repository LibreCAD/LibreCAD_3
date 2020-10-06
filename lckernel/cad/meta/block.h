#pragma once

#include <cad/interface/metatype.h>
#include <vector>
#include <cad/geometry/geocoordinate.h>
#include "metacolor.h"
#include "dxflinepattern.h"
#include "metalinewidth.h"

#define BLOCK_ID_PREFIX "_BLOCK"

namespace lc {
namespace meta {
class Block : public DocumentMetaType {
public:
    Block(std::string name, geo::Coordinate base=geo::Coordinate(0,0));

    const std::string id() const override;

    const std::string name() const override;

    const geo::Coordinate& base() const;

    virtual const std::string metaTypeID() const {
        return Block::LCMETANAME();
    }

    static const std::string LCMETANAME() {
        return BLOCK_ID_PREFIX;
    }

private:
    std::string _name;
    geo::Coordinate _base;
};

DECLARE_SHORT_SHARED_PTR(Block)
}
}
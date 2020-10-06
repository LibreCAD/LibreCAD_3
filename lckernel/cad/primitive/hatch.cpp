#include "cad/primitive/hatch.h"
#include <cmath>
#include <algorithm>
#include "cad/interface/metatype.h"

using namespace lc;
using namespace entity;

Hatch::Hatch(meta::Layer_CSPtr layer,
             meta::MetaInfo_CSPtr metaInfo,
             meta::Block_CSPtr block) :
    CADEntity(std::move(layer), std::move(metaInfo), std::move(block))
{
}


Hatch::Hatch(const Hatch_CSPtr& other, bool sameID) : CADEntity(other, sameID) {
}

CADEntity_CSPtr Hatch::move(const geo::Coordinate &offset) const {
    auto newHatch =  std::make_shared<Hatch>(layer(), metaInfo(), block());
    newHatch->setRegion(_region.move(offset));
    newHatch->setPattern(_pattern);
    newHatch->setPatternName(_name);
    newHatch->setSolid(_solid);
    newHatch->setAngle(_angle);
    newHatch->setScale(_scale);
    newHatch->setID(this->id());
    return newHatch;
}

CADEntity_CSPtr Hatch::copy(const geo::Coordinate &offset) const {
    auto newHatch =  std::make_shared<Hatch>(layer(), metaInfo(), block());
    newHatch->setRegion(_region.copy(offset));
    newHatch->setPattern(_pattern);
    newHatch->setPatternName(_name);
    newHatch->setSolid(_solid);
    newHatch->setAngle(_angle);
    newHatch->setScale(_scale);
    return newHatch;
}

CADEntity_CSPtr Hatch::rotate(const geo::Coordinate &rotation_center, const double rotation_angle) const {
    auto newHatch =  std::make_shared<Hatch>(layer(), metaInfo(), block());
    newHatch->setRegion(_region.rotate(rotation_center, rotation_angle));
    newHatch->setPattern(_pattern);
    newHatch->setPatternName(_name);
    newHatch->setSolid(_solid);
    newHatch->setAngle(_angle);
    newHatch->setScale(_scale);
    newHatch->setID(this->id());
    return newHatch;
}

CADEntity_CSPtr Hatch::scale(const geo::Coordinate &scale_center, const geo::Coordinate &scale_factor) const {
    auto newHatch =  std::make_shared<Hatch>(layer(), metaInfo(), block());
    newHatch->setRegion(_region.scale(scale_center, scale_factor));
    newHatch->setPattern(_pattern);
    newHatch->setPatternName(_name);
    newHatch->setSolid(_solid);
    newHatch->setAngle(_angle);
    newHatch->setScale(_scale);
    newHatch->setID(this->id());
    return newHatch;
}

CADEntity_CSPtr Hatch::mirror(const geo::Coordinate &axis1, const geo::Coordinate &axis2) const {
    auto newHatch =  std::make_shared<Hatch>(layer(), metaInfo(), block());
    newHatch->setRegion(_region.mirror(axis1, axis2));
    newHatch->setPattern(_pattern);
    newHatch->setPatternName(_name);
    newHatch->setSolid(_solid);
    newHatch->setAngle(_angle);
    newHatch->setScale(_scale);
    newHatch->setID(this->id());
    return newHatch;
}

const geo::Area Hatch::boundingBox() const {
    return _region.boundingBox();
}

CADEntity_CSPtr Hatch::modify(meta::Layer_CSPtr layer, const meta::MetaInfo_CSPtr metaInfo, meta::Block_CSPtr block) const {
    auto newHatch =  std::make_shared<Hatch>(layer, metaInfo, block);
    newHatch->setRegion(_region.move(lc::geo::Coordinate(0,0)));
    newHatch->setPattern(_pattern);
    newHatch->setPatternName(_name);
    newHatch->setSolid(_solid);
    newHatch->setAngle(_angle);
    newHatch->setScale(_scale);
    newHatch->setID(this->id());
    return newHatch;
}

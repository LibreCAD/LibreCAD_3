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


Hatch::Hatch(const Hatch_CSPtr& other, bool sameID) : CADEntity(other, sameID){
}

CADEntity_CSPtr Hatch::move(const geo::Coordinate &offset) const {
    return nullptr;
}

CADEntity_CSPtr Hatch::copy(const geo::Coordinate &offset) const {
    return nullptr;
}

CADEntity_CSPtr Hatch::rotate(const geo::Coordinate &rotation_center, const double rotation_angle) const {
    return nullptr;
}

CADEntity_CSPtr Hatch::scale(const geo::Coordinate &scale_center, const geo::Coordinate &scale_factor) const {
    return nullptr;
}

CADEntity_CSPtr Hatch::mirror(const geo::Coordinate &axis1, const geo::Coordinate &axis2) const {
    return nullptr;
}

const geo::Area Hatch::boundingBox() const {
    return geo::Area();
}

CADEntity_CSPtr Hatch::modify(meta::Layer_CSPtr layer, const meta::MetaInfo_CSPtr metaInfo, meta::Block_CSPtr block) const {
    return nullptr;
}
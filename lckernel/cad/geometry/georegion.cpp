#include "georegion.h"

using namespace lc;
using namespace geo;

Loop::Loop(std::vector<entity::CADEntity_CSPtr> loop): _objList(loop){
    //Calculate bounding box for entity
    _boundingBox =  loop[0]->boundingBox();
    //Merge box of each entities
    for(auto &x:loop){
            _boundingBox = _boundingBox.merge(x->boundingBox());
    }
}

Region::Region(){   
}

void Region::addLoop(Loop loop){
    _loopList.push_back(loop);
}

unsigned int Region::numLoops() const{
    return _loopList.size();
}

bool Region::isPointInside(geo::Coordinate& testPoint) const{
    throw("Not implemented");
};

lc::geo::Area Region::boundingBox() const{
    lc::geo::Area boundingBox=_loopList[0].boundingBox();
    for(auto &x: _loopList){
        boundingBox = boundingBox.merge(x.boundingBox());
    }
    return boundingBox;
};

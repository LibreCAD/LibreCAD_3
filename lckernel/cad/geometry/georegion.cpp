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

std::vector<lc::geo::Coordinate> Region::getLineIntersection(const lc::geo::Vector& i1) const{
    lc::maths::Intersect intersect(lc::maths::Intersect::OnEntity, LCTOLERANCE);
    for(auto &x: _loopList){
        for(auto &y: x.entities())
            visitorDispatcher<bool, lc::GeoEntityVisitor>(intersect, i1, *y.get());
    }
    return intersect.result();
}

bool Region::isPointInside(const geo::Coordinate& testPoint) const{
    auto bbox = boundingBox();
    auto minP = bbox.minP();
    auto testLine = lc::geo::Vector(minP, testPoint);
    auto intersect = getLineIntersection(testLine);
    if(intersect.size()%2==0)
        return false;
    else
        return true;
}

lc::geo::Area Region::boundingBox() const{
    lc::geo::Area boundingBox=_loopList[0].boundingBox();
    for(auto &x: _loopList){
        boundingBox = boundingBox.merge(x.boundingBox());
    }
    return boundingBox;
}

#include "georegion.h"

using namespace lc;
using namespace geo;

Loop::Loop(std::vector<entity::CADEntity_CSPtr> loop): _objList(loop) {
    //Calculate bounding box for entity
    _boundingBox =  loop[0]->boundingBox();
    //Merge box of each entities
    for(auto &x:loop) {
        _boundingBox = _boundingBox.merge(x->boundingBox());
    }
}

Region::Region() {
}

Region::Region(std::vector<lc::entity::CADEntity_CSPtr> entities) {
    Loop loop(entities);
    addLoop(loop);
}

void Region::addLoop(Loop loop) {
    _loopList.push_back(loop);
}

unsigned int Region::numLoops() const {
    return _loopList.size();
}

std::vector<lc::geo::Coordinate> Region::getLineIntersection(const lc::geo::Vector& i1) const {
    lc::maths::Intersect intersect(lc::maths::Intersect::OnEntity, LCTOLERANCE);
    for(auto &x: _loopList) {
        for(auto &y: x.entities())
            visitorDispatcher<bool, lc::GeoEntityVisitor>(intersect, i1, *y.get());
    }
    return intersect.result();
}

bool Region::isPointInside(const geo::Coordinate& testPoint) const {
    auto bbox = boundingBox();
    auto minP = bbox.minP();
    auto testLine = lc::geo::Vector(minP, testPoint);
    auto intersect = getLineIntersection(testLine);
    if(intersect.size()%2==0)
        return false;
    else
        return true;
}

lc::geo::Area Region::boundingBox() const {
    lc::geo::Area boundingBox=_loopList[0].boundingBox();
    for(auto &x: _loopList) {
        boundingBox = boundingBox.merge(x.boundingBox());
    }
    return boundingBox;
}

Region Region::move(const geo::Coordinate &offset) const {
    Region reg;
    for(auto &x: _loopList) {
        std::vector<lc::entity::CADEntity_CSPtr> loopData;
        for(auto &y: x.entities())
            loopData.push_back(y->move(offset));
        Loop loop(loopData);
        reg.addLoop(loop);
    }
    return reg;
}

Region Region::copy(const geo::Coordinate &offset) const {
    Region reg;
    for(auto &x: _loopList) {
        std::vector<lc::entity::CADEntity_CSPtr> loopData;
        for(auto &y: x.entities())
            loopData.push_back(y->copy(offset));
        Loop loop(loopData);
        reg.addLoop(loop);
    }
    return reg;
}

Region Region::rotate(const geo::Coordinate &rotation_center, const double rotation_angle) const {
    Region reg;
    for(auto &x: _loopList) {
        std::vector<lc::entity::CADEntity_CSPtr> loopData;
        for(auto &y: x.entities())
            loopData.push_back(y->rotate(rotation_center, rotation_angle));
        Loop loop(loopData);
        reg.addLoop(loop);
    }
    return reg;
}

Region Region::scale(const geo::Coordinate &scale_center,
                     const geo::Coordinate &scale_factor) const {
    Region reg;
    for(auto &x: _loopList) {
        std::vector<lc::entity::CADEntity_CSPtr> loopData;
        for(auto &y: x.entities())
            loopData.push_back(y->scale(scale_center, scale_factor));
        Loop loop(loopData);
        reg.addLoop(loop);
    }
    return reg;
}

Region Region::mirror(const geo::Coordinate& axis1,
                      const geo::Coordinate& axis2) const {
    Region reg;
    for(auto &x: _loopList) {
        std::vector<lc::entity::CADEntity_CSPtr> loopData;
        for(auto &y: x.entities())
            loopData.push_back(y->mirror(axis1, axis2));
        Loop loop(loopData);
        reg.addLoop(loop);
    }
    return reg;
}

// Approximate function, change if possible
double Region::Area() const {
    double area=0.;

    auto bbox = boundingBox();
    auto minP = bbox.minP();
    auto maxP = bbox.maxP();
    unsigned int APPROX_POINTS = 500;
    float delta = (maxP.y() - minP.y())/(float)APPROX_POINTS;
    std::vector<float> oldI;
    float extraSize=-delta;//Comensation if odd intersection is found, delta for first run
    std::vector<float> oldOk;
    for (float i=minP.y(); i<=maxP.y(); i+=delta)
    {
        std::vector<lc::geo::Coordinate> newIc = getLineIntersection(
                    lc::geo::Vector(
                        lc::geo::Coordinate(minP.x()-delta, i),
                        lc::geo::Coordinate(maxP.x()+delta, i)
                    )
                );
        std::vector<float> newI;
        for (auto &i : newIc) {
            newI.push_back(i.x());
        }//sort
        sort(newI.begin(), newI.end());
        if (newI.size()!=oldI.size() || newI.size()%2!=0) {
            //Proper block not found, approximate with newerData
            extraSize += delta;
            oldI = newI;
            continue;
        }
        //Counter for old and new loop
        for(unsigned int x=0; x<newI.size(); x+=2) {
            area += (delta+extraSize)*((oldI[x+1]-oldI[x])+(newI[x+1]-newI[x]))/2;
        }
        extraSize=0.;
        oldI = newI;
        oldOk = newI;
    }
    if(extraSize!=0) { //Last size failed too
        for(unsigned int x=0; x<oldOk.size(); x+=2) {
            area += extraSize*(oldOk[x+1]-oldOk[x]);
        }
    }
    return area;
}

#include "lcvhatch.h"
#include "../painters/lcpainter.h"
#include "../lcdrawoptions.h"
#include "cad/primitive/arc.h"
#include "cad/primitive/line.h"
#include "cad/primitive/ellipse.h"
#include "cad/primitive/spline.h"
#include "cad/primitive/lwpolyline.h"
#include <documentcanvas.h>
#include <cad/math/intersect.h>

using namespace lc::viewer;
LCVHatch::LCVHatch(const lc::entity::Hatch_CSPtr& hatch) :
    LCVDrawItem(hatch, true),
    _hatch(hatch) {
}

void LCVHatch::drawSolid(LcPainter& painter, const LcDrawOptions &options, const lc::geo::Area& rect) const {
    // Hatch for solid fill based on approximation
    auto& reg = _hatch->getRegion();
    auto bbox = reg.boundingBox();
    auto minP = bbox.minP();
    auto maxP = bbox.maxP();
    unsigned int APPROX_POINTS = 300;
    float delta = (maxP.y() - minP.y())/(float)APPROX_POINTS;
    std::vector<float> oldI;
    float oldIY=minP.y();
    float extraSize=-delta;//Comensation if odd intersection is found, delta for first run
    std::vector<float> oldOk;
    for (float i=minP.y(); i<=maxP.y(); i+=delta)
    {
        std::vector<lc::geo::Coordinate> newIc = reg.getLineIntersection(
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
            oldIY = i;
            continue;
        }
        //Counter for old and new loop
        for(unsigned int x=0; x<newI.size(); x+=2) {
            auto oldY = i-delta-extraSize;
            painter.move_to(oldI[x], oldY);
            painter.line_to(newI[x], i);
            painter.line_to(newI[x+1], i);
            painter.line_to(oldI[x+1], oldY);
            painter.close_path();
            painter.fill();
        }
        extraSize=0.;
        oldI = newI;
        oldIY = i;
        oldOk = newI;
    }
    if(extraSize!=0) { //Last size failed too
        for(unsigned int x=0; x<oldOk.size(); x+=2) {
            auto oldY = oldIY-delta;
            auto newY = oldY+extraSize;
            painter.move_to(oldOk[x], oldY);
            painter.line_to(oldOk[x], newY);
            painter.line_to(oldOk[x+1], newY);
            painter.line_to(oldOk[x+1], oldY);
            painter.close_path();
            painter.fill();
        }
    }
}

std::vector<lc::entity::CADEntity_CSPtr> getPatterrnEntitiesFromHatch(const lc::entity::Hatch_CSPtr& hatch) {
    std::vector<lc::entity::CADEntity_CSPtr> entities;
    auto& reg = hatch->getRegion();
    auto bbox = reg.boundingBox();
    auto scale = hatch->getScale();
    auto angle = hatch->getAngle();
    float hsize = hatch->getPattern().boundingBox.maxP().x();
    float vsize = hatch->getPattern().boundingBox.maxP().y();

    int xmin,xmax,ymin,ymax;
    xmin = floor(bbox.minP().x()/hsize/scale);
    ymin = floor(bbox.minP().y()/hsize/scale);
    xmax = floor(bbox.maxP().x()/vsize/scale);
    ymax = floor(bbox.maxP().y()/vsize/scale);

    //Create new entities for the bounding box
    if(angle!=0) {
        xmin-=1;
        ymin-=1;
        xmax+=1;
        ymax+=1;
    }
    for(int i=xmin; i<=xmax; i++)
        for(int j=ymin; j<=ymax; j++) {
            for(const auto& entity : hatch->getPattern().entities) {
                entities.push_back(
                    entity
                    ->scale(lc::geo::Coordinate(0,0), lc::geo::Coordinate(scale,scale))
                    ->rotate(lc::geo::Coordinate(0,0), angle)
                    ->move(lc::geo::Coordinate(i*scale*hsize,j*scale*vsize)
                          ));
            }
        }
    return entities;
};

void trimEntities(const std::vector<lc::geo::Coordinate>& cutPoints, std::vector<lc::entity::CADEntity_CSPtr>& spiltedEntities) {
    for(auto& cutPoint : cutPoints) {
        std::vector<lc::entity::CADEntity_CSPtr> tempEntities;
        for(auto& se: spiltedEntities) {
            if (auto splitable2 = std::dynamic_pointer_cast<const lc::entity::Splitable>(se)) {
                auto ent2 = splitable2->splitEntity(cutPoint);
                if(ent2.size()==0)
                    tempEntities.push_back(se);
                else
                    for(auto& jsx: ent2)
                        tempEntities.push_back(jsx);
            }
        }
        spiltedEntities=tempEntities;
    }
}

// This fails when intersection fails
void LCVHatch::drawPattern(LcPainter& painter, const LcDrawOptions &options, const lc::geo::Area& rect) const {
    auto& reg = _hatch->getRegion();
    auto bbox = reg.boundingBox();
    std::vector<lc::entity::CADEntity_CSPtr> entities = getPatterrnEntitiesFromHatch(_hatch);
    std::vector<lc::entity::CADEntity_CSPtr> finalEntities;
    for(const auto& entity : entities) {
        if (!entity->boundingBox().overlaps(bbox))//optimization
            continue;// It decreased the rendering time from ~5sec to <1s
        if (auto splitable = std::dynamic_pointer_cast<const lc::entity::Splitable>(entity)) {
            lc::maths::Intersect intersect(lc::maths::Intersect::OnEntity, LCTOLERANCE);
            for(auto &x: reg.loopList()) {
                //if(!entity->boundingBox().overlaps(x.boundingBox())) continue;//not needed i think
                for(auto &y: x.entities()) {
                    if(entity->boundingBox().overlaps(y->boundingBox()))
                        visitorDispatcher<bool, lc::GeoEntityVisitor>(intersect, *y.get(), *entity.get());
                }
            }
            auto cutPoints = intersect.result();
            if(cutPoints.size()==0) {
                if(reg.isPointInside(splitable->representingPoint()))
                    finalEntities.push_back(entity);
            } else {
                std::vector<lc::entity::CADEntity_CSPtr> spiltedEntities;
                spiltedEntities.push_back(entity);
                trimEntities(cutPoints, spiltedEntities);
                for(auto& se: spiltedEntities)
                    if(auto splitable2 = std::dynamic_pointer_cast<const lc::entity::Splitable>(se))
                        if(reg.isPointInside(splitable2->representingPoint()))
                            finalEntities.push_back(se);
            }
        }
    }

    for(const auto& entity : finalEntities) {
        DocumentCanvas::asDrawable(entity)->draw(painter, options, rect);
    }
}

void LCVHatch::draw(LcPainter& painter, const LcDrawOptions &options, const lc::geo::Area& rect) const {
    if(_hatch->isSolid())
        drawSolid(painter, options, rect);
    else
        drawPattern(painter, options, rect);
}

lc::entity::CADEntity_CSPtr LCVHatch::entity() const {
    return _hatch;
}

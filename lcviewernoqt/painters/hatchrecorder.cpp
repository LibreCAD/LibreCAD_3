#include "pathgeometry.h"
#include "cad/primitive/line.h"
#include "cad/primitive/arc.h"
#include "cad/primitive/circle.h"
#include "cad/primitive/ellipse.h"
#include "cad/primitive/lwpolyline.h"
#include "cad/primitive/point.h"
#include "cad/primitive/spline.h"
#include "cad/objects/pattern.h"
#include <cmath>
#include <stdexcept>

namespace lc { namespace viewer { namespace render {
namespace {
void check(bool ok,const char* code) { if (!ok) throw std::invalid_argument(code); }
Point point(const lc::geo::Coordinate& p) {
    check(std::isfinite(p.x()) && std::isfinite(p.y()) && p.z()==0,"invalid-hatch-coordinate");
    return {p.x(),p.y()};
}
void record(PathRecorder& r,const lc::entity::CADEntity_CSPtr& entity,RecordingLimits limits) {
    check(bool(entity),"invalid-hatch-entity");r.newPath();
    if (auto p=std::dynamic_pointer_cast<const lc::entity::Line>(entity)) {
        r.moveTo(point(p->start()));r.lineTo(point(p->end()));
    } else if (auto p=std::dynamic_pointer_cast<const lc::entity::Arc>(entity)) {
        r.arc(point(p->center()),p->radius(),p->startAngle(),p->endAngle(),!p->CCW());
    } else if (auto p=std::dynamic_pointer_cast<const lc::entity::Circle>(entity)) {
        r.circle(point(p->center()),p->radius());
    } else if (auto p=std::dynamic_pointer_cast<const lc::entity::Ellipse>(entity)) {
        const bool closed=!p->isArc();
        // Honor the CAD model's full-ellipse classification and tolerance.
        // Capture the periodic seam explicitly for dashes/joins.
        r.ellipse(point(p->center()),p->majorRadius(),p->minorRadius(),p->startAngle(),closed?p->startAngle():p->endAngle(),p->getAngle(),p->isReversed());
        if (closed) r.closePath();
    } else if (auto p=std::dynamic_pointer_cast<const lc::entity::LWPolyline>(entity)) {
        const auto& vertices=p->vertex();check(vertices.size()<=limits.elements,"vertex-limit");
        check(p->elevation()==0 && p->extrusionDirection().x()==0 && p->extrusionDirection().y()==0 &&
              p->extrusionDirection().z()==1,"unsupported-hatch-plane");
        check(p->width()==0,"unsupported-hatch-width");
        std::vector<Point> points;std::vector<double> bulges;
        for (std::size_t i=0;i<vertices.size();++i) {
            const auto& v=vertices[i];check(v.startWidth()==0 && v.endWidth()==0,"unsupported-hatch-width");
            points.push_back(point(v.location()));
            if (i+1<vertices.size() || p->closed()) bulges.push_back(v.bulge());
        }
        r.polyline(points,bulges,p->closed());
    } else if (auto p=std::dynamic_pointer_cast<const lc::entity::Point>(entity)) {
        r.point(point(*p),3,PointUnits::Physical);return;
    } else if (auto p=std::dynamic_pointer_cast<const lc::entity::Spline>(entity)) {
        check(p->controlPoints().size()<=limits.elements && p->knotPoints().size()<=limits.elements &&
              p->fitPoints().size()<=limits.elements && p->bezierCount()<=limits.elements,"vertex-limit");
        check(p->bezierCount()>0,"unsupported-hatch-spline");
        check((unsigned(p->flags())&unsigned(lc::geo::Spline::RATIONAL))==0,"unsupported-rational-provider");
        check(!p->closed() && (unsigned(p->flags())&unsigned(lc::geo::Spline::PERIODIC))==0,"unsupported-hatch-spline");
        for (std::size_t i=0;i<p->bezierCount();++i) {
            const auto& span=p->bezierAt(i);check(bool(span),"invalid-hatch-entity");
            // The current provider creates only polynomial quadratic/cubic
            // spans. Do not copy an unknown provider's unbounded control array.
            check(bool(std::dynamic_pointer_cast<const lc::geo::Bezier>(span)) ||
                  bool(std::dynamic_pointer_cast<const lc::geo::CubicBezier>(span)),"unsupported-hatch-spline");
            const auto controls=span->getCP();check(controls.size()==3 || controls.size()==4,"unsupported-hatch-spline");
            std::vector<Point> points;for (const auto& c:controls) points.push_back(point(c));
            r.bezierSpan(points);
        }
    } else throw std::invalid_argument("unsupported-hatch-entity");
    r.stroke();
}
}
HatchTile recordHatchTile(const lc::objects::Pattern& pattern,const PaintState& state,RecordingLimits limits) {
    PathRecorder recorder(limits);state.transform.validate();
    recorder.source(state.rgba[0],state.rgba[1],state.rgba[2],state.rgba[3]);
    recorder.lineWidth(state.lineWidth);recorder.lineWidthCompensation(state.lineWidthCompensation);
    recorder.dash(state.dash,state.dashOffset,state.dashScaled);recorder.antialias(state.antialias);
    // One source entity is one submitted paint at most. Aggregate element,
    // command and dash budgets remain enforced by the SAME recorder instance.
    check(pattern.entities.size()<=limits.commands && pattern.entities.size()<=limits.elements,"command-limit");
    HatchTile out;out.state=state;
    if (!pattern.entities.empty()) {
        out.minimum=point(pattern.boundingBox.minP());out.maximum=point(pattern.boundingBox.maxP());
        check(std::isfinite(out.maximum.x-out.minimum.x) && std::isfinite(out.maximum.y-out.minimum.y) &&
              out.maximum.x>out.minimum.x && out.maximum.y>out.minimum.y,"invalid-hatch-tile");
    }
    for (const auto& entity:pattern.entities) record(recorder,entity,limits);
    out.commands=recorder.commands();return out;
}
}}}

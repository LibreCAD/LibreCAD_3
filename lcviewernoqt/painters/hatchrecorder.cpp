#include "pathgeometry.h"
#include "pathnumeric_p.h"
#include "cad/primitive/line.h"
#include "cad/primitive/arc.h"
#include "cad/primitive/circle.h"
#include "cad/primitive/ellipse.h"
#include "cad/primitive/lwpolyline.h"
#include "cad/primitive/point.h"
#include "cad/primitive/spline.h"
#include "cad/objects/pattern.h"
#include "cad/geometry/georegion.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>

namespace lc { namespace viewer { namespace render {
namespace {
void check(bool ok,const char* code) { if (!ok) throw std::invalid_argument(code); }
Point point(const lc::geo::Coordinate& p) {
    check(std::isfinite(p.x()) && std::isfinite(p.y()) && p.z()==0,"invalid-hatch-coordinate");
    return {p.x(),p.y()};
}
void record(PathRecorder& r,const lc::entity::CADEntity_CSPtr& entity,RecordingLimits limits,bool boundary=false) {
    check(bool(entity),"invalid-hatch-entity");r.newPath();
    if (auto p=std::dynamic_pointer_cast<const lc::entity::Line>(entity)) {
        r.moveTo(point(p->start()));r.lineTo(point(p->end()));
    } else if (auto p=std::dynamic_pointer_cast<const lc::entity::Arc>(entity)) {
        r.arc(point(p->center()),p->radius(),p->startAngle(),p->endAngle(),!p->CCW());
    } else if (auto p=std::dynamic_pointer_cast<const lc::entity::Circle>(entity)) {
        r.circle(point(p->center()),p->radius());
    } else if (auto p=std::dynamic_pointer_cast<const lc::entity::Ellipse>(entity)) {
        // Its 3D length and XY angle must not silently project a nonplanar axis.
        point(p->majorP());
        const bool closed=!p->isArc();
        // Honor the CAD model's full-ellipse classification and tolerance.
        // Capture the periodic seam explicitly for dashes/joins.
        r.ellipse(point(p->center()),p->majorRadius(),p->minorRadius(),p->startAngle(),closed?p->startAngle():p->endAngle(),p->getAngle(),p->isReversed());
        if (closed) r.closePath();
    } else if (auto p=std::dynamic_pointer_cast<const lc::entity::LWPolyline>(entity)) {
        const auto& vertices=p->vertex();check(vertices.size()<=limits.elements,"vertex-limit");
        check(p->elevation()==0 && p->extrusionDirection().x()==0 && p->extrusionDirection().y()==0 &&
              p->extrusionDirection().z()==1,"unsupported-hatch-plane");
        check(boundary || p->width()==0,"unsupported-hatch-width");
        std::vector<Point> points;std::vector<double> bulges;
        for (std::size_t i=0;i<vertices.size();++i) {
            const auto& v=vertices[i];check(boundary || (v.startWidth()==0 && v.endWidth()==0),"unsupported-hatch-width");
            points.push_back(point(v.location()));
            if (i+1<vertices.size() || p->closed()) bulges.push_back(v.bulge());
        }
        r.polyline(points,bulges,p->closed());
    } else if (auto p=std::dynamic_pointer_cast<const lc::entity::Point>(entity)) {
        check(!boundary,"unsupported-region-entity");
        r.point(point(*p),3,PointUnits::Physical);return;
    } else if (auto p=std::dynamic_pointer_cast<const lc::entity::Spline>(entity)) {
        check(p->controlPoints().size()<=limits.elements && p->knotPoints().size()<=limits.elements &&
              p->fitPoints().size()<=limits.elements && p->bezierCount()<=limits.elements,"vertex-limit");
        check(p->bezierCount()>0,"unsupported-hatch-spline");
        check((unsigned(p->flags())&unsigned(lc::geo::Spline::RATIONAL))==0,"unsupported-rational-provider");
        check(!p->closed() && (unsigned(p->flags())&unsigned(lc::geo::Spline::PERIODIC))==0,"unsupported-hatch-spline");
        Point previous{};
        for (std::size_t i=0;i<p->bezierCount();++i) {
            const auto& span=p->bezierAt(i);check(bool(span),"invalid-hatch-entity");
            // The current provider creates only polynomial quadratic/cubic
            // spans. Do not copy an unknown provider's unbounded control array.
            check(bool(std::dynamic_pointer_cast<const lc::geo::Bezier>(span)) ||
                  bool(std::dynamic_pointer_cast<const lc::geo::CubicBezier>(span)),"unsupported-hatch-spline");
            const auto controls=span->getCP();check(controls.size()==3 || controls.size()==4,"unsupported-hatch-spline");
            std::vector<Point> points;for (const auto& c:controls) points.push_back(point(c));
            // One CAD spline is one traversal: keep dash phase and endpoint
            // caps across its provider spans. No snapping error is certified by
            // HatchTile, so a nonidentical provider seam must fail, not bridge.
            if (!i) r.moveTo(points.front());
            else check(previous.x==points.front().x && previous.y==points.front().y,"disconnected-hatch-spline");
            if (points.size()==3) r.quadraticTo(points[1],points[2]);
            else r.cubicTo(points[1],points[2],points[3]);
            previous=points.back();
        }
    } else throw std::invalid_argument("unsupported-hatch-entity");
    r.stroke();
}
void inherit(PathRecorder& recorder,const PaintState& state) {
    state.transform.validate();
    recorder.source(state.rgba[0],state.rgba[1],state.rgba[2],state.rgba[3]);
    recorder.lineWidth(state.lineWidth);recorder.lineWidthCompensation(state.lineWidthCompensation);
    recorder.dash(state.dash,state.dashOffset,state.dashScaled);recorder.antialias(state.antialias);
}
bool same(Point a,Point b) {return a.x==b.x && a.y==b.y;}
struct RegionEdge {Point start;PathElement end;double startRound=0,endRound=0;bool used=false;};
struct RegionBuilder {
    RegionLimits limits;RecordedRegion out;std::size_t stored=0;
    void tick(std::size_t n=1) {check(n<=limits.work-out.visited,"region-work-limit");out.visited+=n;}
    double round(Point a,Point b) const {
        return 128*std::numeric_limits<double>::epsilon()*(1+std::max({std::abs(a.x),std::abs(a.y),std::abs(b.x),std::abs(b.y)}));
    }
    double generationRound(const lc::entity::CADEntity_CSPtr& entity,const Transform& transform) const {
        double radius=0;
        if (auto p=std::dynamic_pointer_cast<const lc::entity::Arc>(entity)) radius=p->radius();
        else if (auto p=std::dynamic_pointer_cast<const lc::entity::Circle>(entity)) radius=p->radius();
        else if (auto p=std::dynamic_pointer_cast<const lc::entity::Ellipse>(entity)) radius=std::max(p->majorRadius(),p->minorRadius());
        const auto u=transform.distance({radius,0}),v=transform.distance({0,radius});
        const double bound=128*std::numeric_limits<double>::epsilon()*(std::abs(u.x)+std::abs(u.y)+std::abs(v.x)+std::abs(v.y));
        check(std::isfinite(bound),"precision-limit");return bound;
    }
    bool near(Point a,Point b,double rounding) const {
        if (same(a,b)) return true;
        const double d=std::hypot(a.x-b.x,a.y-b.y);
        return std::isfinite(d) && d<=rounding && d+rounding<=limits.joinError;
    }
    void join(Point a,Point b,double rounding) {
        check(near(a,b,rounding),"disconnected-region-loop");
        if (!same(a,b)) out.positionBound=std::max(out.positionBound,std::nextafter(std::hypot(a.x-b.x,a.y-b.y)+rounding,std::numeric_limits<double>::infinity()));
        check(out.positionBound<=limits.joinError,"precision-limit");
    }
    void append(PathElement e) {
        check(out.boundary.path.size()<limits.recording.elements,"vertex-limit");out.boundary.path.push_back(e);
    }
    void loop(const lc::geo::Loop& loop,const PaintState& state) {
        // A DXF hatch may declare a loop with no edges; it encloses no area.
        // A loop whose edges record nothing is still rejected below.
        const auto& entities=loop.entities();if (entities.empty()) return;
        check(entities.size()<=limits.recording.elements-stored,"vertex-limit");std::vector<RegionEdge> edges;
        for (const auto& entity:entities) {
            tick();auto budget=limits.recording;budget.elements=std::min(budget.elements-stored,limits.work-out.visited);budget.commands=1;
            // Join before adding the camera translation. A pan must not turn
            // source-generation roundoff into either a real gap or a snap.
            auto local=state.transform;local.tx=local.ty=0;
            PathRecorder part(budget);part.setTransform(local);record(part,entity,budget,true);
            check(part.commands().size()==1,"invalid-region-loop");const auto& path=part.commands()[0].path;
            check(path.size()<=limits.recording.elements-stored,"vertex-limit");stored+=path.size();
            const double generated=generationRound(entity,local);
            Point current{},first{};double currentRound=0,firstRound=0;bool started=false;
            for (const auto& e:path) {
                tick();const double endpointRound=round(e.point,e.point)+generated;
                if (e.verb==PathVerb::Move) {current=first=e.point;currentRound=firstRound=endpointRound;started=true;continue;}
                check(started,"invalid-region-loop");auto end=e;
                if (e.verb==PathVerb::Close) {end.verb=PathVerb::Line;end.point=first;}
                const double endRound=e.verb==PathVerb::Close?firstRound:endpointRound;
                if (!same(current,end.point) || end.verb!=PathVerb::Line) edges.push_back({current,end,currentRound,endRound,false});
                current=end.point;currentRound=endRound;
            }
        }
        check(!edges.empty(),"invalid-region-loop");
        // First stored edge fixes winding. Uniqueness, not nearest-neighbor
        // guessing, determines the rest; comparisons share a bounded work cap.
        const Point first=edges[0].start;append({PathVerb::Move,first});append(edges[0].end);edges[0].used=true;
        Point current=edges[0].end.point;double currentRound=edges[0].endRound;
        for (std::size_t i=1;i<edges.size();++i) {
            std::size_t found=edges.size();bool reverse=false;
            for (std::size_t j=0;j<edges.size();++j) {
                tick();if (edges[j].used) continue;
                for (bool back:{false,true}) if (near(current,back?edges[j].end.point:edges[j].start,
                        currentRound+(back?edges[j].endRound:edges[j].startRound))) {
                    check(found==edges.size(),"ambiguous-region-loop");found=j;reverse=back;
                }
            }
            check(found<edges.size(),"disconnected-region-loop");auto& edge=edges[found];edge.used=true;
            join(current,reverse?edge.end.point:edge.start,currentRound+(reverse?edge.endRound:edge.startRound));auto end=edge.end;
            if (reverse) {end.point=edge.start;if (end.verb==PathVerb::Cubic) std::swap(end.control1,end.control2);}
            append(end);current=end.point;currentRound=reverse?edge.startRound:edge.endRound;
        }
        join(current,first,currentRound+edges[0].startRound);out.boundary.path.back().point=first;append({PathVerb::Close,first});
    }
    void place(const Transform& transform) {
        double rounding=0;
        auto point=[&](Point p) {
            detail::Sum x,y;x.add(p.x);x.add(transform.tx);y.add(p.y);y.add(transform.ty);
            rounding=std::max(rounding,x.error()+y.error());return Point{x.value(),y.value()};
        };
        for (auto& e:out.boundary.path) {
            tick();e.point=point(e.point);
            if (e.verb==PathVerb::Quadratic || e.verb==PathVerb::Cubic || e.verb==PathVerb::Conic) e.control1=point(e.control1);
            if (e.verb==PathVerb::Cubic) e.control2=point(e.control2);
        }
        if (rounding) out.positionBound=std::nextafter(out.positionBound+rounding,std::numeric_limits<double>::infinity());
        check(out.positionBound<=limits.joinError,"precision-limit");
    }
};
void chargeRegion(const RecordedRegion& input,std::size_t vertices,std::size_t& work) {
    check(vertices<=MaxFrameElements && work<=4*MaxFrameElements,"invalid-limits");
    check(input.visited<=work,"region-work-limit");work-=input.visited;
}
}
HatchTile recordHatchTile(const lc::objects::Pattern& pattern,const PaintState& state,RecordingLimits limits) {
    PathRecorder recorder(limits);inherit(recorder,state);
    // One source entity is one submitted paint at most. Aggregate element,
    // command and dash budgets remain enforced by the SAME recorder instance.
    check(pattern.entities.size()<=limits.commands && pattern.entities.size()<=limits.elements,"command-limit");
    HatchTile out;out.state=state;
    if (!pattern.entities.empty()) {
        out.minimum=point(pattern.boundingBox.minP());out.maximum=point(pattern.boundingBox.maxP());
        // Existing DXF pattern cells are anchored at (0,0), as in LCVHatch.
        // Empty margins are part of the repetition, not cropped ink bounds.
        out.period=out.maximum;
        check(std::isfinite(out.maximum.x-out.minimum.x) && std::isfinite(out.maximum.y-out.minimum.y) &&
              out.maximum.x>out.minimum.x && out.maximum.y>out.minimum.y &&
              out.period.x>0 && out.period.y>0,"invalid-hatch-tile");
    }
    for (const auto& entity:pattern.entities) record(recorder,entity,limits);
    out.commands=recorder.commands();return out;
}
RecordedRegion recordRegion(const lc::geo::Region& region,const PaintState& state,RegionLimits limits) {
    check(limits.work<=4*MaxFrameElements && std::isfinite(limits.joinError) && limits.joinError>=0 && limits.joinError<=1.0/64,"invalid-limits");
    PathRecorder validation(limits.recording);inherit(validation,state);
    check(limits.recording.commands>0,"command-limit");check(state.dash.size()<=limits.recording.elements,"state-limit");
    check(region.loopList().size()<=limits.recording.elements,"vertex-limit");
    RegionBuilder builder{limits,{},0};builder.out.boundary.operation=PaintOperation::Fill;builder.out.boundary.state=state;
    builder.tick(state.dash.size());for (const auto& loop:region.loopList()) {builder.tick();builder.loop(loop,state);}
    builder.place(state.transform);
    return std::move(builder.out);
}
PreparedFill prepareRegionFill(const RecordedRegion& input,FillRule rule,FillLimits limits,const LinearGradient* gradient) {
    chargeRegion(input,limits.vertices,limits.work);
    auto out=prepareFill(input.boundary,rule,limits,gradient,input.positionBound);out.visited+=input.visited;return out;
}
PreparedFill prepareRegionFill(const lc::geo::Region& region,const PaintState& state,FillRule rule,FillLimits limits,const LinearGradient* gradient) {
    RegionLimits recording;recording.recording.elements=limits.vertices;recording.work=limits.work;
    return prepareRegionFill(recordRegion(region,state,recording),rule,limits,gradient);
}
PreparedTiledHatch prepareRegionHatch(const RecordedRegion& input,const HatchTile& tile,double scale,double angle,FillRule rule,StrokeStyle style,TiledHatchLimits limits) {
    chargeRegion(input,limits.geometry.vertices,limits.geometry.work);
    auto out=prepareTiledHatch(input.boundary,tile,scale,angle,rule,style,limits,input.positionBound);out.mask.visited+=input.visited;out.visited+=input.visited;return out;
}
PreparedTiledHatch prepareRegionHatch(const lc::geo::Region& region,const HatchTile& tile,double scale,double angle,FillRule rule,StrokeStyle style,TiledHatchLimits limits) {
    RegionLimits recording;recording.recording.elements=limits.geometry.vertices;recording.work=limits.geometry.work;
    return prepareRegionHatch(recordRegion(region,tile.state,recording),tile,scale,angle,rule,style,limits);
}
}}}

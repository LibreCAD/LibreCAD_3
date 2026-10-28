#include "pathgeometry.h"
#include "pathnumeric_p.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>

namespace lc { namespace viewer { namespace render {
namespace {
constexpr double eps=std::numeric_limits<double>::epsilon();
constexpr double infinity=std::numeric_limits<double>::infinity();
void check(bool ok,const char* code) {if (!ok) throw std::invalid_argument(code);}
double up(double x) {return std::nextafter(x,infinity);}
using detail::Sum;
bool identity(const Transform& t) {
    return t.xx==1 && t.xy==0 && t.yx==0 && t.yy==1 && t.tx==0 && t.ty==0 && t.origin.x==0 && t.origin.y==0;
}
bool sameStyle(const PaintState& a,const PaintState& b) {
    return a.rgba==b.rgba && a.lineWidth==b.lineWidth && a.lineWidthCompensation==b.lineWidthCompensation &&
        a.dash==b.dash && a.dashOffset==b.dashOffset && a.dashScaled==b.dashScaled && a.antialias==b.antialias;
}
struct Box {
    Point lo{},hi{};bool empty=true;
    void add(Point p) {
        check(std::isfinite(p.x) && std::isfinite(p.y),"precision-limit");
        if (empty) {lo=hi=p;empty=false;} else {lo={std::min(lo.x,p.x),std::min(lo.y,p.y)};hi={std::max(hi.x,p.x),std::max(hi.y,p.y)};}
    }
    void pad(double p) {lo={lo.x-p,lo.y-p};hi={hi.x+p,hi.y+p};}
};
Box mapped(const Box& b,const Transform& inverse) {
    Box out;for (double x:{b.lo.x,b.hi.x}) for (double y:{b.lo.y,b.hi.y}) out.add(inverse.distance({x,y}));return out;
}
bool overlaps(const PreparedStroke& s,Point shift,const Box& mask,double error) {
    return s.maximum.x+shift.x+error>=mask.lo.x && s.minimum.x+shift.x-error<=mask.hi.x &&
           s.maximum.y+shift.y+error>=mask.lo.y && s.minimum.y+shift.y-error<=mask.hi.y;
}
struct Lattice {
    Transform root,rotation,linear;double w,h,linearError=0;
    Lattice(const HatchTile& tile,double scale,double angle):root(tile.state.transform),w(tile.period.x),h(tile.period.y) {
        const double c=std::cos(angle)*scale,s=std::sin(angle)*scale;
        rotation.xx=c;rotation.xy=-s;rotation.yx=s;rotation.yy=c;rotation.validate();
        auto coefficient=[&](double a,double b,double c0,double d) {Sum x;x.product(a,b);x.product(c0,d);linearError=up(linearError+x.error());return x.value();};
        linear.xx=coefficient(root.xx,c,root.xy,s);linear.xy=coefficient(root.xx,-s,root.xy,c);
        linear.yx=coefficient(root.yx,c,root.yy,s);linear.yy=coefficient(root.yx,-s,root.yy,c);linear.validate();
    }
    Point shift(double i,double j,bool camera,double& error) const {
        Sum x,y;x.product(i,w);y.product(j,h);
        Sum u=x.scaled(rotation.xx),v=x.scaled(rotation.yx);
        u.append(y.scaled(rotation.xy));v.append(y.scaled(rotation.yy));
        if (camera) {u.add(-root.origin.x);v.add(-root.origin.y);}
        Sum px=u.scaled(root.xx),py=u.scaled(root.yx);px.append(v.scaled(root.xy));py.append(v.scaled(root.yy));
        if (camera) {px.add(root.tx);py.add(root.ty);}
        error=up(px.error()+py.error());return {px.value(),py.value()};
    }
};
}
PreparedTiledHatch prepareTiledHatch(const PathCommand& boundary,const HatchTile& tile,double scale,double angle,
                                   FillRule rule,StrokeStyle style,TiledHatchLimits limits,double inputPositionBound) {
    check(std::isfinite(scale) && scale>0 && std::isfinite(angle),"invalid-hatch-scale");
    check(limits.cells<=MaxFrameElements && limits.instances<=MaxFrameElements,"invalid-limits");
    check(tile.commands.size()<=MaxFrameElements && tile.state.dash.size()<=1024,"state-limit");
    tile.state.transform.validate();
    // Validate even empty tiles/styles without silently accepting invalid state.
    PathCommand empty;empty.operation=PaintOperation::Stroke;empty.state=tile.state;
    prepareStroke(empty,style,limits.geometry);
    std::size_t input=0;for (const auto& c:tile.commands) {
        check(c.path.size()<=MaxFrameElements-input,"vertex-limit");input+=c.path.size();
        check(c.state.dash.size()<=1024 && c.state.dash.size()<=MaxFrameElements-input,"state-limit");input+=c.state.dash.size();
        check(identity(c.state.transform) && sameStyle(c.state,tile.state) &&
              (c.operation==PaintOperation::Stroke || c.operation==PaintOperation::Point),"invalid-hatch-tile");
    }
    PreparedTiledHatch out;out.ink.state=tile.state;
    auto tick=[&](std::size_t n) {check(n<=limits.geometry.work-out.visited,"hatch-work-limit");out.visited+=n;};
    tick(input);tick(tile.commands.size());out.mask=prepareFill(boundary,rule,{limits.geometry.vertices,limits.geometry.work-out.visited},nullptr,inputPositionBound);tick(out.mask.visited);
    if (tile.commands.empty()) {out.positionBound=out.mask.positionBound;return out;}
    check(std::isfinite(tile.minimum.x) && std::isfinite(tile.minimum.y) && std::isfinite(tile.maximum.x) && std::isfinite(tile.maximum.y),"invalid-hatch-tile");
    check(tile.maximum.x>tile.minimum.x && tile.maximum.y>tile.minimum.y,"invalid-hatch-tile");
    Lattice lattice(tile,scale,angle);
    check(std::isfinite(lattice.w) && std::isfinite(lattice.h) && lattice.w>0 && lattice.h>0,"invalid-hatch-tile");
    const Point center={(out.mask.minimum.x+out.mask.maximum.x)/2,(out.mask.minimum.y+out.mask.maximum.y)/2};
    const Point model=lattice.rotation.inverseDistance(lattice.root.inversePoint(center));
    const double ai=std::floor(model.x/lattice.w),aj=std::floor(model.y/lattice.h),integerLimit=4503599627370496.;
    check(std::isfinite(ai) && std::isfinite(aj) && std::abs(ai)<integerLimit && std::abs(aj)<integerLimit,"precision-limit");
    double anchorError=0;const Point anchor=lattice.shift(ai,aj,true,anchorError);
    check(anchorError<=1.0/128,"precision-limit");
    Transform base=lattice.linear;base.tx=anchor.x;base.ty=anchor.y;
    std::vector<PreparedStroke> meshes;meshes.reserve(tile.commands.size());Box bounds;std::size_t cached=0;
    const std::size_t maskVertices=3*out.mask.triangles.size();double inkError=0;
    for (const auto& source:tile.commands) {
        PathCommand c=source;c.state.transform=base;double magnitude=0;
        auto point=[&](Point p) {magnitude=std::max(magnitude,std::abs(p.x)+std::abs(p.y));return base.point(p);};
        for (auto& e:c.path) {
            tick(1);e.point=point(e.point);
            if (e.verb==PathVerb::Quadratic || e.verb==PathVerb::Cubic || e.verb==PathVerb::Conic) e.control1=point(e.control1);
            if (e.verb==PathVerb::Cubic) e.control2=point(e.control2);
        }
        const double numeric=up(anchorError+lattice.linearError*magnitude+128*eps*(1+std::abs(anchor.x)+std::abs(anchor.y)+magnitude*(std::abs(base.xx)+std::abs(base.xy)+std::abs(base.yx)+std::abs(base.yy))));
        check(numeric<=1.0/64,"precision-limit");
        auto budget=limits.geometry;budget.work-=out.visited;budget.vertices-=maskVertices+cached;
        auto mesh=prepareStroke(c,style,budget);tick(mesh.visited);cached+=3*mesh.triangles.size();
        mesh.positionBound=up(mesh.positionBound+numeric);inkError=std::max(inkError,mesh.positionBound);
        if (!mesh.triangles.empty()) {bounds.add(mesh.minimum);bounds.add(mesh.maximum);}
        meshes.push_back(std::move(mesh));
    }
    out.positionBound=up(out.mask.positionBound+inkError);check(out.positionBound<=.25,"precision-limit");
    if (bounds.empty || out.mask.triangles.empty()) return out;
    // Coverage culling includes full stroke geometry, its numeric bound and one
    // physical pixel of AA support, not just finite primitive centerlines.
    Box mask;mask.add(out.mask.minimum);mask.add(out.mask.maximum);mask.pad(1+out.positionBound);bounds.pad(1+inkError);
    Transform basis=lattice.linear;basis.xx*=lattice.w;basis.yx*=lattice.w;basis.xy*=lattice.h;basis.yy*=lattice.h;basis.validate();
    const auto x=basis.inverseDistance({1,0}),y=basis.inverseDistance({0,1});Transform inverse;
    inverse.xx=x.x;inverse.yx=x.y;inverse.xy=y.x;inverse.yy=y.y;
    auto m=mapped(mask,inverse),b=mapped(bounds,inverse);
    const double inverseNorm=std::abs(x.x)+std::abs(x.y)+std::abs(y.x)+std::abs(y.y);
    const double forwardNorm=std::abs(basis.xx)+std::abs(basis.xy)+std::abs(basis.yx)+std::abs(basis.yy);
    const double mag=1+std::max({std::abs(mask.lo.x),std::abs(mask.lo.y),std::abs(mask.hi.x),std::abs(mask.hi.y),std::abs(bounds.lo.x),std::abs(bounds.lo.y),std::abs(bounds.hi.x),std::abs(bounds.hi.y)});
    const double rangeError=up(256*eps*inverseNorm*mag*(1+inverseNorm*forwardNorm));
    check(std::isfinite(rangeError) && rangeError*forwardNorm<=1.0/128,"precision-limit");m.pad(rangeError);b.pad(rangeError);
    const double firstX=std::ceil(m.lo.x-b.hi.x),lastX=std::floor(m.hi.x-b.lo.x),firstY=std::ceil(m.lo.y-b.hi.y),lastY=std::floor(m.hi.y-b.lo.y);
    for (double i:{firstX,lastX}) check(std::abs(i)<integerLimit && std::abs(ai+i)<integerLimit,"precision-limit");
    for (double j:{firstY,lastY}) check(std::abs(j)<integerLimit && std::abs(aj+j)<integerLimit,"precision-limit");
    if (lastX<firstX || lastY<firstY) return out;
    const double nx=lastX-firstX+1,ny=lastY-firstY+1;
    check(nx<=double(limits.cells) && ny<=double(limits.cells) && nx*ny<=double(limits.cells),"hatch-cell-limit");
    out.cells=std::size_t(nx)*std::size_t(ny);
    check(tile.commands.size()<=limits.instances/out.cells,"hatch-instance-limit");
    struct Placement {std::size_t mesh;Point shift;};std::vector<Placement> placements;
    std::size_t vertices=maskVertices;double translationError=0;
    for (double i=firstX;i<=lastX;++i) for (double j=firstY;j<=lastY;++j) {
        tick(1);double error=0;const Point shift=lattice.shift(i,j,false,error);
        error=up(error+128*eps*(1+mag+std::abs(shift.x)+std::abs(shift.y)));
        check(error<=1.0/128,"precision-limit");translationError=std::max(translationError,error);
        for (std::size_t k=0;k<meshes.size();++k) {
            tick(1);const auto& mesh=meshes[k];
            if (mesh.triangles.empty() || !overlaps(mesh,shift,mask,error+inkError)) continue;
            const auto count=3*mesh.triangles.size();check(count<=limits.geometry.vertices-vertices,"vertex-limit");vertices+=count;
            placements.push_back({k,shift});
        }
    }
    out.instances=placements.size();out.positionBound=up(out.positionBound+translationError);check(out.positionBound<=.25,"precision-limit");
    out.ink.triangles.reserve((vertices-maskVertices)/3);Box ink;
    for (const auto& placement:placements) for (const auto& triangle:meshes[placement.mesh].triangles) {
        tick(1);std::array<Point,3> moved;
        for (unsigned k=0;k<3;++k) {moved[k]={triangle[k].x+placement.shift.x,triangle[k].y+placement.shift.y};ink.add(moved[k]);}
        const double ax=moved[1].x-moved[0].x,ay=moved[1].y-moved[0].y,bx=moved[2].x-moved[0].x,by=moved[2].y-moved[0].y;
        check(ax*by-ay*bx>16*eps*(std::abs(ax*by)+std::abs(ay*bx)),"precision-limit");
        out.ink.triangles.push_back(moved);
    }
    if (!ink.empty) {ink.pad(inkError+translationError);out.ink.minimum=ink.lo;out.ink.maximum=ink.hi;}
    out.ink.positionBound=up(inkError+translationError);out.ink.visited=out.visited-out.mask.visited;return out;
}
}}}

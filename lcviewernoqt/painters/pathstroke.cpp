#include "pathgeometry.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <utility>

namespace lc { namespace viewer { namespace render {
namespace {
constexpr double pi = 3.141592653589793238462643383279502884;
constexpr double eps = std::numeric_limits<double>::epsilon();
void check(bool ok, const char* code) { if (!ok) throw std::invalid_argument(code); }
void validate(StrokeStyle style, StrokeLimits limits) {
    check((style.cap==StrokeCap::Butt || style.cap==StrokeCap::Round || style.cap==StrokeCap::Square) &&
          (style.join==StrokeJoin::Miter || style.join==StrokeJoin::Round || style.join==StrokeJoin::Bevel) &&
          (style.widthUnits==WidthUnits::Physical || style.widthUnits==WidthUnits::Model) &&
          std::isfinite(style.miterLimit) && style.miterLimit>0,"invalid-stroke");
    check(std::isfinite(limits.positionError) && limits.positionError>0 && limits.positionError<=1.0/16 &&
          std::isfinite(limits.pointDiameter) && limits.pointDiameter>0 && limits.pointDiameter<=MaxFrameElements &&
          limits.vertices<=MaxFrameElements && limits.work<=4*MaxFrameElements,"invalid-limits");
}
Point valid(Point p) { check(std::isfinite(p.x) && std::isfinite(p.y),"invalid-stroke"); return p; }
Point add(Point a, Point b) { return valid({a.x+b.x,a.y+b.y}); }
Point sub(Point a, Point b) { return valid({a.x-b.x,a.y-b.y}); }
Point mul(Point a, double v) { return valid({a.x*v,a.y*v}); }
double norm(Point p) { const double n=std::hypot(p.x,p.y); check(std::isfinite(n),"invalid-stroke"); return n; }
bool same(Point a, Point b) { return a.x==b.x && a.y==b.y; }
double cross(Point a, Point b) { return a.x*b.y-a.y*b.x; }
double matrixNorm(const Transform& t, bool inverse) {
    const auto a=inverse?t.inverseDistance({1,0}):t.distance({1,0});
    const auto b=inverse?t.inverseDistance({0,1}):t.distance({0,1});
    const double n=std::hypot(norm(a),norm(b)); check(std::isfinite(n),"invalid-stroke"); return n;
}
struct Run {
    std::vector<Point> points;
    bool closed=false;
    Point direction{};
    bool directionInMetric=false;
    std::vector<CurveTangents> tangents;
};
struct Builder {
    const PathCommand& command;
    StrokeStyle style;
    StrokeLimits limits;
    DashPattern dash;
    PreparedStroke result;
    double radius, stretch, inverseStretch, dashStretch, distanceRound=0;
    bool model;
    Point anchor;
    std::size_t runPoints=0;
    Builder(const PathCommand& c, StrokeStyle s, StrokeLimits l) : command(c),style(s),limits(l),dash(normalizeDash(c.state)) {
        result.state=c.state;
        const double width=c.state.lineWidth+c.state.lineWidthCompensation;
        check(std::isfinite(c.state.lineWidth) && c.state.lineWidth>=0 && std::isfinite(c.state.lineWidthCompensation) &&
              std::isfinite(width) && width>=0,"invalid-line-width");
        model=s.widthUnits==WidthUnits::Model && width!=0;
        radius=width==0?0.5:width/2;
        check(radius>0,"precision-limit");
        const double forward=matrixNorm(c.state.transform,false);
        stretch=model?forward:1;
        inverseStretch=matrixNorm(c.state.transform,true);
        dashStretch=c.state.dashScaled?1:forward;
        check(std::isfinite(radius*stretch),"invalid-stroke");
        if (!dash.values.empty()) roundDistance(64*eps*dash.period*dashStretch);
        for (double v:c.state.rgba) check(std::isfinite(v) && v>=0 && v<=1,"invalid-color");
    }
    void tick() { check(result.visited<limits.work,"subdivision-limit"); ++result.visited; }
    void roundDistance(double value) {
        distanceRound=std::nextafter(distanceRound+value,std::numeric_limits<double>::infinity());
        check(std::isfinite(distanceRound) && distanceRound<=1.0/32,"precision-limit");
    }
    void append(Run& run, Point p) {
        valid(p); check(runPoints<limits.vertices,"vertex-limit");run.points.push_back(p);++runPoints;
    }
    Point metric(Point p) const {
        p=sub(p,anchor);return model?command.state.transform.inverseDistance(p):p;
    }
    Point physical(Point p) const {
        const double mag=1+std::max(std::abs(anchor.x),std::abs(anchor.y))+stretch*std::max(std::abs(p.x),std::abs(p.y));
        check(128*eps*mag<limits.positionError/4,"precision-limit");
        return add(anchor,model?command.state.transform.distance(p):p);
    }
    void triangle(Point a, Point b, Point c) {
        tick();check(result.triangles.size()<limits.vertices/3,"vertex-limit");
        a=physical(a);b=physical(b);c=physical(c);
        const Point ab=sub(b,a),ac=sub(c,a);
        const double area=cross(ab,ac);check(std::isfinite(area),"invalid-stroke");
        // Nearly collinear faces can acquire a spurious nonzero area through
        // cancellation. Their thickness is below the guarded numeric budget;
        // do not publish an uncertifiable winding or a degenerate coverage face.
        const double areaRound=16*eps*(std::abs(ab.x*ac.y)+std::abs(ab.y*ac.x));
        if (std::abs(area)<=areaRound) return;
        if (area<0) std::swap(b,c);
        if (result.triangles.empty()) result.minimum=result.maximum=a;
        for (Point p:{a,b,c}) {
            result.minimum={std::min(result.minimum.x,p.x),std::min(result.minimum.y,p.y)};
            result.maximum={std::max(result.maximum.x,p.x),std::max(result.maximum.y,p.y)};
        }
        result.triangles.push_back({{a,b,c}});
    }
    void quad(Point a,Point b,Point c,Point d) {
        const auto before=result.triangles.size();triangle(a,b,c);triangle(a,c,d);
        check(result.triangles.size()>before,"precision-limit");
    }
    void sector(Point p,Point from,Point to,double sweep) {
        const double r=radius*stretch;
        const double angle=4*std::asin(std::sqrt(std::min(limits.positionError/2,r)/(2*r)));
        check(std::isfinite(angle) && angle>0,"precision-limit");
        const double count=std::max(std::ceil(std::abs(sweep)/(pi/2)),std::ceil(std::abs(sweep)/angle));
        check(count<=double((limits.vertices/3)-result.triangles.size()),"vertex-limit");
        const auto n=static_cast<std::size_t>(count);
        // Shared strip/join endpoints must be bit-identical. Re-evaluating a
        // normal through sin/cos creates cracks along the sector's diameter.
        const double start=std::atan2(from.y,from.x);
        Point first=add(p,mul(from,radius)),a=first;
        for (std::size_t i=1;i<=n;++i) {
            const double t=start+sweep*(double(i)/double(n));
            const Point b=i==n?add(p,mul(to,radius)):add(p,mul({std::cos(t),std::sin(t)},radius));
            triangle(p,a,b);a=b;
        }
    }
    void disc(Point p) {sector(p,{1,0},{1,0},2*pi);}
    void cap(Point p,Point direction) {
        if (style.cap==StrokeCap::Round) sector(p,{direction.y,-direction.x},{-direction.y,direction.x},pi);
        else if (style.cap==StrokeCap::Square) {
            const Point n=mul({-direction.y,direction.x},radius),e=mul(direction,radius);
            quad(add(p,n),add(add(p,n),e),add(sub(p,n),e),sub(p,n));
        }
    }
    void join(Point p,Point u,Point v,bool internal=false) {
        const double turn=cross(u,v),dot=std::max(-1.,std::min(1.,u.x*v.x+u.y*v.y));
        if (turn==0 && dot>0) return;
        // The inside normal fan is part of continuity, not a decorative miter.
        // It can be visible when adjacent segments are shorter than the width.
        const double inner=turn>=0?1.:-1.;
        sector(p,mul({-u.y,u.x},inner),mul({-v.y,v.x},inner),turn==0?pi:std::atan2(turn,dot));
        if (internal || style.join==StrokeJoin::Round) {
            const double side=turn>=0?-1.:1.;
            const Point n=mul({-u.y,u.x},side);
            const Point end=mul({-v.y,v.x},side);
            sector(p,n,end,turn==0?pi:std::atan2(turn,dot));return;
        }
        if (turn==0) return; // exact reversal: bevel, not an unbounded miter
        const double side=turn>0?-1.:1.;
        const Point a=mul({-u.y,u.x},side*radius),b=mul({-v.y,v.x},side*radius);
        const double denominator=1+dot;
        if (style.join==StrokeJoin::Miter && denominator>0 && std::sqrt(2/denominator)<=style.miterLimit) {
            const Point m=mul(add(a,b),1/denominator);
            triangle(p,add(p,a),add(p,m));triangle(p,add(p,m),add(p,b));
        } else triangle(p,add(p,a),add(p,b));
    }
    void mesh(const Run& run) {
        if (run.points.empty()) return;
        anchor=run.points.front();
        if (run.points.size()==1) {
            const auto before=result.triangles.size();
            const bool ink=style.cap==StrokeCap::Round || (style.cap==StrokeCap::Square && !same(run.direction,{}));
            if (style.cap==StrokeCap::Round) disc({});
            else if (style.cap==StrokeCap::Square && !same(run.direction,{})) {
                const Point d=model && !run.directionInMetric?command.state.transform.inverseDistance(run.direction):run.direction;
                const Point u=mul(d,radius/norm(d)),n={-u.y,u.x};
                quad(add(u,n),sub(n,u),mul(add(u,n),-1),sub(u,n));
            }
            check(!ink || result.triangles.size()>before,"precision-limit");
            return;
        }
        const std::size_t edges=run.points.size()-(run.closed?0:1);
        Point previous{},first{};
        for (std::size_t i=0;i<edges;++i) {
            tick();const Point a=metric(run.points[i]),b=metric(run.points[(i+1)%run.points.size()]);
            const Point d=sub(b,a);const double length=norm(d);check(length>0,"precision-limit");
            const Point u=run.tangents.empty()?mul(d,1/length):run.tangents[i].start;
            const Point v=run.tangents.empty()?u:run.tangents[i].end;
            const Point n=mul({-u.y,u.x},radius),m=mul({-v.y,v.x},radius);
            quad(add(a,n),add(b,m),sub(b,m),sub(a,n));
            if (i) join(a,previous,u,!run.tangents.empty() && run.tangents[i].internal);else {first=u;if (!run.closed) cap(a,mul(u,-1));}
            previous=v;
            if (i+1==edges) {if (run.closed) join(b,v,first);else cap(b,v);}
        }
    }
    void path(const Run& input) {
        if (input.points.empty()) return;
        if (dash.values.empty()) {mesh(input);return;}
        std::vector<Run> runs;Run current;
        std::size_t index=0;double remaining=dash.values[0],phase=dash.phase;
        const auto advance=[&] { index=(index+1)%dash.values.size();remaining=dash.values[index]; };
        const auto finish=[&] { if (!current.points.empty()) {runs.push_back(std::move(current));current=Run{};} };
        while (phase>0) {tick();if (phase>=remaining) {phase-=remaining;advance();}else {remaining-=phase;phase=0;}}
        const auto zeros=[&](Point p,Point direction) {
            bool dot=false;
            while (remaining==0) {tick();dot|=index%2==0;advance();}
            if (dot) {Run point;point.direction=direction;point.directionInMetric=!input.tangents.empty();append(point,p);runs.push_back(std::move(point));}
            if (index%2!=0) finish();
        };
        const auto phaseStable=[&] {
            if (input.tangents.empty()) return;
            const double uncertainty=distanceRound/dashStretch;
            check(remaining>uncertainty && dash.values[index]-remaining>uncertainty,"precision-limit");
        };
        if (input.points.size()==1) {
            zeros(input.points.front(),{});
            if (index%2==0) {Run dot;append(dot,input.points.front());runs.push_back(std::move(dot));}
            for (const auto& run:runs) mesh(run);
            return;
        }
        const std::size_t edges=input.points.size()-(input.closed?0:1);
        for (std::size_t i=0;i<edges;++i) {
            tick();const Point a=input.points[i],b=input.points[(i+1)%input.points.size()],delta=sub(b,a);
            const double physicalLength=norm(delta);
            const double length=command.state.dashScaled?physicalLength:norm(command.state.transform.inverseDistance(delta));
            check(length>0,"precision-limit");
            roundDistance(64*eps*physicalLength*(command.state.dashScaled?1:inverseStretch)*dashStretch);
            double at=0;Point p=a;
            const auto tangent=[&](double t) {
                if (input.tangents.empty()) return delta;
                const auto& n=input.tangents[i];if (t==0) return n.start;if (t>=1) return n.end;
                const Point v=add(mul(n.start,1-t),mul(n.end,t));const double length=norm(v);
                check(length>0,"precision-limit");return mul(v,1/length);
            };
            while (at<length) {
                tick();zeros(p,tangent(at/length));
                if (at==0 && i && !input.tangents.empty() && !same(input.tangents[i-1].end,input.tangents[i].start)) phaseStable();
                const double amount=std::min(remaining,length-at);
                roundDistance(16*eps*(at+amount+remaining+length)*dashStretch);
                const double next=at+amount;check(next>at,"precision-limit");
                const Point q=next>=length?b:add(a,mul(delta,next/length));
                check(!same(p,q),"precision-limit");
                if (index%2==0) {
                    if (current.points.empty()) append(current,p);
                    if (!same(q,current.points.back())) {
                        append(current,q);if (!input.tangents.empty()) current.tangents.push_back({tangent(at/length),tangent(next/length),at==0 && input.tangents[i].internal});
                    }
                }
                else finish();
                if (amount==remaining) advance();else remaining-=amount;
                at=next;p=q;
            }
        }
        const Point end=input.closed?input.points.front():input.points.back();
        zeros(end,input.tangents.empty()?sub(end,input.closed?input.points.back():input.points[input.points.size()-2]):input.tangents.back().end);
        phaseStable();finish();
        if (runs.empty()) return;
        if (input.closed) {
            auto firstRun=std::find_if(runs.begin(),runs.end(),[](const Run& r){return r.points.size()>1;});
            auto lastRun=std::find_if(runs.rbegin(),runs.rend(),[](const Run& r){return r.points.size()>1;});
            if (firstRun!=runs.end() && lastRun!=runs.rend() && same(firstRun->points.front(),end) && same(lastRun->points.back(),end)) {
                auto& first=*firstRun;auto& last=*lastRun;
                if (&first==&last) {first.points.pop_back();first.closed=true;}
                else {
                    check(first.points.size()-1<=limits.vertices-last.points.size(),"vertex-limit");
                    last.points.insert(last.points.end(),first.points.begin()+1,first.points.end());
                    last.tangents.insert(last.tangents.end(),first.tangents.begin(),first.tangents.end());
                    first.points.clear();
                }
            }
        }
        for (const auto& run:runs) mesh(run);
    }
};
}
DashPattern normalizeDash(const PaintState& state) {
    check(state.dash.size()<=1024 && std::isfinite(state.dashOffset),"invalid-dash");
    DashPattern result;result.values=state.dash;
    for (double v:state.dash) {check(std::isfinite(v) && v>=0,"invalid-dash");result.period+=v;}
    check(std::isfinite(result.period) && (state.dash.empty() || result.period>0),"invalid-dash");
    if (state.dash.size()%2) {result.values.insert(result.values.end(),state.dash.begin(),state.dash.end());result.period*=2;}
    check(std::isfinite(result.period),"invalid-dash");
    // Match legacy Cairo's public normalized state: a zero internal interval
    // merges its neighbors. Keep endpoint zeros; all-zero gaps mean solid.
    std::vector<double> canonical;
    for (double value:result.values) {
        if (canonical.size()>=2 && canonical.back()==0) {
            canonical[canonical.size()-2]+=value;canonical.pop_back();
        } else canonical.push_back(value);
    }
    bool gap=false;for (std::size_t i=1;i<canonical.size();i+=2) gap|=canonical[i]>0;
    result.values=std::move(canonical);
    if (!gap) {result.values.clear();result.period=0;}
    if (result.period) {result.phase=std::fmod(state.dashOffset,result.period);if (result.phase<0) result.phase+=result.period;}
    return result;
}
PreparedStroke prepareStroke(const PathCommand& command, StrokeStyle style, StrokeLimits limits) {
    check(command.operation==PaintOperation::Stroke || command.operation==PaintOperation::Point,"invalid-stroke");command.state.transform.validate();
    validate(style,limits);
    check(command.path.size()<=MaxFrameElements,"vertex-limit");
    if (command.operation==PaintOperation::Point) {
        check(command.path.size()==1 && command.path[0].verb==PathVerb::Move &&
              (command.pointUnits==PointUnits::Physical || command.pointUnits==PointUnits::Model) &&
              std::isfinite(command.pointDiameter) && command.pointDiameter>=0,"invalid-point");
        valid(command.path[0].point);check(limits.vertices>=1,"vertex-limit");
        double diameter=command.pointDiameter;
        if (command.pointUnits==PointUnits::Model && diameter>0) {
            const auto& t=command.state.transform;
            const double s=std::max({std::abs(t.xx),std::abs(t.xy),std::abs(t.yx),std::abs(t.yy)});
            const double a=t.xx/s,b=t.xy/s,c=t.yx/s,d=t.yy/s;
            // Largest singular value, without squaring/overflow or a loose
            // Frobenius bound that would reject an identity-sized limit mark.
            diameter*=s*((std::hypot(a+d,c-b)+std::hypot(a-d,c+b))/2);
        }
        check(std::isfinite(diameter) && diameter<=limits.pointDiameter,"point-size-limit");
        PathCommand ink=command;ink.state.lineWidth=command.pointDiameter?command.pointDiameter:1;
        ink.state.lineWidthCompensation=0;ink.state.dash.clear();ink.state.dashOffset=0;
        StrokeStyle round;round.cap=StrokeCap::Round;
        round.widthUnits=command.pointUnits==PointUnits::Model?WidthUnits::Model:WidthUnits::Physical;
        Builder b(ink,round,limits);b.result.state=command.state;b.tick();
        if (command.pointDiameter>0) {
            Run mark;b.append(mark,command.path[0].point);b.mesh(mark);
            b.result.positionBound=limits.positionError;
            b.result.minimum=sub(b.result.minimum,{limits.positionError,limits.positionError});
            b.result.maximum=add(b.result.maximum,{limits.positionError,limits.positionError});
        }
        return std::move(b.result);
    }
    Builder b(command,style,limits);double curvePosition=0,curveNormal=0;
    const bool curved=std::any_of(command.path.begin(),command.path.end(),[](const PathElement& e){return e.verb==PathVerb::Quadratic || e.verb==PathVerb::Cubic || e.verb==PathVerb::Conic;});
    if (curved) {
        Transform inverse;const Point x=command.state.transform.inverseDistance({1,0}),y=command.state.transform.inverseDistance({0,1});
        inverse.xx=x.x;inverse.yx=x.y;inverse.xy=y.x;inverse.yy=y.y;
        CurveMetrics metrics;metrics.physicalRadius=b.radius*b.stretch;
        metrics.dashed=!b.dash.values.empty();
        if (b.model) metrics.tangent=inverse;
        if (!b.dash.values.empty() && !command.state.dashScaled) {metrics.length=inverse;metrics.lengthScale=b.dashStretch;}
        CurveLimits curveLimits;curveLimits.vertices=limits.vertices;curveLimits.work=limits.work;curveLimits.lengthError=1.0/64;
        auto flat=flatten(command,curveLimits,metrics);b.result.visited=flat.visited;
        curvePosition=flat.positionBound;curveNormal=flat.normalBound;
        if (!b.dash.values.empty()) b.roundDistance((flat.lengthUpper-flat.lengthLower)*metrics.lengthScale);
        for (auto& line:flat.subpaths) {
            Run run;for (std::size_t i=0;i<line.points.size();++i) {
                b.tick();if (i && same(line.points[i-1],line.points[i])) continue;
                b.append(run,line.points[i]);if (i) run.tangents.push_back(line.tangents[i-1]);
            }
            run.closed=line.closed;
            if (run.closed && run.points.size()>1 && same(run.points.front(),run.points.back())) run.points.pop_back();
            if (!run.tangents.empty() || line.points.size()>1) b.path(run);
        }
    } else {
    Run path;bool closed=false,hasEdge=false;
    const auto finish=[&] {if (hasEdge) b.path(path);path=Run{};hasEdge=false;closed=false;};
    for (const auto& e:command.path) {
        b.tick();valid(e.point);
        if (e.verb==PathVerb::Move) {finish();b.append(path,e.point);continue;}
        check(!path.points.empty() && !closed,"invalid-path");
        check(e.verb==PathVerb::Line || e.verb==PathVerb::Close,"invalid-path");hasEdge=true;
        if (e.verb==PathVerb::Close) {
            if (path.points.size()>1 && same(path.points.front(),path.points.back())) path.points.pop_back();
            path.closed=closed=true;
        }
        else if (!same(path.points.back(),e.point)) b.append(path,e.point);
    }
    finish();
    }
    if (!b.result.triangles.empty()) {
        b.result.positionBound=limits.positionError+b.distanceRound+curvePosition+curveNormal;
        check(b.result.positionBound<=.25,"precision-limit");
        b.result.minimum=sub(b.result.minimum,{b.result.positionBound,b.result.positionBound});
        b.result.maximum=add(b.result.maximum,{b.result.positionBound,b.result.positionBound});
    }
    return std::move(b.result);
}
std::vector<PreparedStroke> prepareStrokes(const std::vector<PathCommand>& commands, StrokeStyle style, StrokeLimits limits) {
    validate(style,limits);
    check(commands.size()<=MaxFrameElements,"command-limit");
    check(commands.size()<=limits.work,"subdivision-limit");
    std::size_t input=0,state=0,vertices=0,work=commands.size();
    for (const auto& c:commands) {
        check(c.path.size()<=limits.vertices-input,"vertex-limit");input+=c.path.size();
        check(c.state.dash.size()<=limits.vertices-state,"state-limit");state+=c.state.dash.size();
    }
    std::vector<PreparedStroke> result;
    for (const auto& c:commands) {
        auto remaining=limits;remaining.vertices-=vertices;remaining.work-=work;
        auto stroke=prepareStroke(c,style,remaining);
        // prepareStroke enforces these remaining caps before allocating faces.
        vertices+=3*stroke.triangles.size();work+=stroke.visited;
        result.push_back(std::move(stroke));
    }
    return result;
}
}}}

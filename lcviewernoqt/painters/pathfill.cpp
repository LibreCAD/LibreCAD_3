#include "pathgeometry.h"
#include "pathnumeric_p.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>

namespace lc { namespace viewer { namespace render {
namespace {
constexpr double eps = std::numeric_limits<double>::epsilon();
void check(bool ok, const char* code) { if (!ok) throw std::invalid_argument(code); }
bool same(Point a, Point b) { return a.x==b.x && a.y==b.y; }
Point sub(Point a, Point b) { return {a.x-b.x,a.y-b.y}; }
double dot(Point a, Point b) { return a.x*b.x+a.y*b.y; }
void valid(Point p) {
    // Bounded double domain, not a claim about arbitrary CAD world coordinates.
    // Append-time local origins permit large world offsets without large pixels.
    for (double v:{p.x,p.y}) check(std::isfinite(v) && std::abs(v)<=1e9 &&
                                  (v==0 || std::abs(v)>=1e-80),"precision-limit");
}
void validate(FillRule rule, FillLimits limits) {
    check(rule==FillRule::EvenOdd || rule==FillRule::NonZero,"invalid-fill-rule");
    check(limits.vertices<=MaxFrameElements && limits.work<=4*MaxFrameElements,"invalid-limits");
}
// Exact sign for the bounded input-double domain: two-difference, FMA product
// residuals, and fixed-size error-free expansion sums. No epsilon winding test,
// extended precision, allocation, or platform-specific long-double assumption.
std::array<double,2> difference(double a,double b) {
    const double x=a-b,z=x-a;
    return {{(a-(x-z))-(b+z),x}};
}
int orient(Point a,Point b,Point c) {
    const auto x=difference(b.x,a.x),y=difference(b.y,a.y);
    const auto u=difference(c.x,a.x),v=difference(c.y,a.y);
    detail::Sum e;
    for (unsigned i=0;i<2;++i) for (unsigned j=0;j<2;++j) {
        const double p=x[i]*v[j],q=y[i]*u[j];
        e.add(std::fma(x[i],v[j],-p));e.add(p);
        e.add(-std::fma(y[i],u[j],-q));e.add(-q);
    }
    return e.value()>0?1:(e.value()<0?-1:0);
}
struct Refine {};
struct Edge { Point a,b; std::size_t loop,index,count; Polyline::Edge curve; };
bool boxesMeet(const Edge& a,const Edge& b,double pad=0) {
    return std::max(a.a.x,a.b.x)+pad>=std::min(b.a.x,b.b.x) &&
           std::max(b.a.x,b.b.x)+pad>=std::min(a.a.x,a.b.x) &&
           std::max(a.a.y,a.b.y)+pad>=std::min(b.a.y,b.b.y) &&
           std::max(b.a.y,b.b.y)+pad>=std::min(a.a.y,a.b.y);
}
double distance(Point p,const Edge& e) {
    const Point d=sub(e.b,e.a),v=sub(p,e.a);
    const double length=std::hypot(d.x,d.y),t=std::max(0.,std::min(length,dot(v,{d.x/length,d.y/length})));
    return std::hypot(v.x-t*d.x/length,v.y-t*d.y/length);
}
struct Builder {
    FillLimits limits;
    PreparedFill result;
    std::vector<Edge> edges;
    std::vector<double> levels;
    double round=0;
    void tick() {check(result.visited<limits.work,"fill-work-limit");++result.visited;}
    void triangle(Point a,Point b,Point c) {
        tick();valid(a);valid(b);valid(c);
        const int sign=orient(a,b,c);if (!sign) return;
        check(sign>0,"precision-limit");
        check(result.triangles.size()<limits.vertices/3,"vertex-limit");
        if (result.triangles.empty()) result.minimum=result.maximum=a;
        for (Point p:{a,b,c}) {
            result.minimum={std::min(result.minimum.x,p.x),std::min(result.minimum.y,p.y)};
            result.maximum={std::max(result.maximum.x,p.x),std::max(result.maximum.y,p.y)};
        }
        result.triangles.push_back({{a,b,c}});
    }
    double x(const Edge& e,double y) const {
        if (y==e.a.y) return e.a.x;
        if (y==e.b.y) return e.b.x;
        return e.a.x+(e.b.x-e.a.x)*((y-e.a.y)/(e.b.y-e.a.y));
    }
    bool separated(const Edge& a,const Edge& b,Point common) {
        // Positive Bezier/conic pieces stay in their control hulls. A separating
        // line through the shared endpoint certifies adjacency without giving
        // a whole curved span an unchecked exception from crossing rejection.
        for (const auto* candidate:{&a.curve,&b.curve}) for (unsigned k=0;k<=candidate->degree;++k) {
            const Point axis=candidate->controls[k];if (same(axis,common)) continue;
            int lowA=0,highA=0,lowB=0,highB=0;
            for (unsigned i=0;i<=a.curve.degree;++i) {tick();const int s=orient(common,axis,a.curve.controls[i]);lowA=std::min(lowA,s);highA=std::max(highA,s);}
            for (unsigned i=0;i<=b.curve.degree;++i) {tick();const int s=orient(common,axis,b.curve.controls[i]);lowB=std::min(lowB,s);highB=std::max(highB,s);}
            if (!((lowA>=0 && highB<=0) || (lowB>=0 && highA<=0))) continue;
            bool overlap=false;
            for (unsigned i=0;i<=a.curve.degree;++i) for (unsigned j=0;j<=b.curve.degree;++j) {
                tick();const Point u=a.curve.controls[i],v=b.curve.controls[j];
                if (same(u,common) || same(v,common) || orient(common,axis,u) || orient(common,axis,v)) continue;
                // Collinear control rays may meet only at the common endpoint.
                overlap|=u.x!=common.x ? (u.x>common.x)==(v.x>common.x) : (u.y>common.y)==(v.y>common.y);
            }
            if (!overlap) return true;
        }
        return false;
    }
    void topology(double inputPositionBound) {
        const auto padding=[&](const Edge& e){return e.curve.positionBound+inputPositionBound+2*round;};
        std::vector<std::size_t> order,active;order.reserve(edges.size());
        for (std::size_t i=0;i<edges.size();++i) {tick();order.push_back(i);}
        const auto low=[&](const Edge& e){return std::min(e.a.x,e.b.x)-padding(e);};
        std::sort(order.begin(),order.end(),[&](std::size_t a,std::size_t b){tick();return low(edges[a])<low(edges[b]);});
        // Sweep padded boxes. Ordinary finely flattened curves have only a few
        // simultaneous candidates; adversarial overlap still consumes the SAME
        // global work cap rather than getting a quadratic uncharged preflight.
        for (auto i:order) {
            const auto& a=edges[i];const double left=low(a);
            active.erase(std::remove_if(active.begin(),active.end(),[&](std::size_t j){tick();const auto& b=edges[j];return std::max(b.a.x,b.b.x)+padding(b)<left;}),active.end());
            for (auto j:active) {
                tick();const auto& b=edges[j];const double clearance=padding(a)+padding(b);
                if (!boxesMeet(a,b,clearance)) continue;
                const bool adjacent=a.loop==b.loop && (a.index==b.index+1 || b.index==a.index+1 ||
                    (a.index==a.count-1 && b.index==0) || (b.index==b.count-1 && a.index==0));
                if (adjacent) {
                    const Point common=same(a.a,b.b)?a.a:b.a;
                    const Point u=same(a.a,common)?a.b:a.a,v=same(b.a,common)?b.b:b.a;
                    check(orient(u,common,v)!=0 || dot(sub(u,common),sub(v,common))<0,"unsupported-fill-topology");
                    if ((a.curve.degree>1 || b.curve.degree>1) && !separated(a,b,common)) throw Refine{};
                    continue;
                }
                const int x1=orient(a.a,a.b,b.a),x2=orient(a.a,a.b,b.b);
                const int y1=orient(b.a,b.b,a.a),y2=orient(b.a,b.b,a.b);
                check(!(boxesMeet(a,b) && x1*x2<=0 && y1*y2<=0),"unsupported-fill-topology");
                const double gap=std::min({distance(a.a,b),distance(a.b,b),distance(b.a,a),distance(b.b,a)});
                if (gap<=clearance) {
                    if (a.curve.degree>1 || b.curve.degree>1) throw Refine{};
                    check(false,"precision-limit");
                }
            }
            active.push_back(i);
        }
    }
    PreparedFill run(const PathCommand& command,FillRule rule,double inputPositionBound,double positionError) {
        check(command.operation==PaintOperation::Fill,"invalid-fill");
        command.state.transform.validate();
        for (double c:command.state.rgba) check(std::isfinite(c) && c>=0 && c<=1,"invalid-color");
        check(command.state.dash.size()<=1024 && command.state.dash.size()<=limits.work,"fill-work-limit");
        result.visited=command.state.dash.size();
        result.state=command.state;result.rule=rule;
        CurveLimits curve;curve.vertices=limits.vertices;curve.work=limits.work-result.visited;curve.positionError=positionError;
        const auto flat=flatten(command,curve);result.visited+=flat.visited;
        result.positionBound=flat.positionBound+inputPositionBound;
        for (const auto& path:flat.subpaths) for (Point p:path.points) {
            tick();valid(p);round=std::max(round,256*eps*(1+std::max(std::abs(p.x),std::abs(p.y))));
            levels.push_back(p.y);
        }
        std::sort(levels.begin(),levels.end(),[&](double a,double b){tick();return a<b;});
        // Canonicalize only roundoff-scale Y levels, charging the maximum move
        // to the position bound. Otherwise adjacent doubles have no interior
        // sample (ordinary symmetric conic extrema can produce such bands).
        levels.erase(std::unique(levels.begin(),levels.end(),[&](double a,double b){return b-a<=round;}),levels.end());
        std::size_t stored=0,loop=0;
        for (const auto& path:flat.subpaths) {
            bool originalArea=false;std::size_t distinct=1;
            while (distinct<path.points.size() && same(path.points[0],path.points[distinct])) {tick();++distinct;}
            for (std::size_t i=distinct+1;i<path.points.size();++i) {
                tick();originalArea|=orient(path.points[0],path.points[distinct],path.points[i])!=0;
            }
            std::vector<Point> p;std::vector<Polyline::Edge> certificates;
            for (std::size_t i=0;i<path.points.size();++i) {
                Point v=path.points[i];
                tick();const auto upper=std::upper_bound(levels.begin(),levels.end(),v.y,[&](double a,double b){tick();return a<b;});
                check(upper!=levels.begin(),"precision-limit");v.y=*(upper-1);
                if (!i) {check(stored<limits.vertices,"vertex-limit");p.push_back(v);++stored;continue;}
                auto edge=path.edges[i-1];
                if (std::hypot(p.back().x-v.x,p.back().y-v.y)<=round) {
                    if (edge.degree>1) for (unsigned k=0;k<=edge.degree;++k) if (!same(edge.controls[k],edge.controls[0])) throw Refine{};
                    continue;
                }
                edge.controls[0]=p.back();edge.controls[edge.degree]=v;
                check(stored<limits.vertices,"vertex-limit");p.push_back(v);++stored;certificates.push_back(edge);
            }
            if (p.size()>1 && std::hypot(p.front().x-p.back().x,p.front().y-p.back().y)<=round) {
                auto& end=certificates.back();end.controls[end.degree]=p.front();p.pop_back();
            }
            else if (p.size()>1) certificates.push_back({{{p.back(),p.front(),{},{}}},1,0});
            bool curvedArea=false;
            for (const auto& e:certificates) {
                unsigned first=1;while (first<=e.degree && same(e.controls[0],e.controls[first])) {tick();++first;}
                for (unsigned k=first+1;k<=e.degree;++k) {tick();curvedArea|=orient(e.controls[0],e.controls[first],e.controls[k])!=0;}
            }
            if (p.size()<3) {if (curvedArea) throw Refine{};check(!originalArea,"precision-limit");continue;}
            bool noncollinear=false;
            for (std::size_t i=2;i<p.size();++i) {tick();noncollinear|=orient(p[0],p[1],p[i])!=0;}
            if (!noncollinear) {if (curvedArea) throw Refine{};check(!originalArea,"precision-limit");continue;} // exact line/point contours have no ink
            for (std::size_t i=0;i<p.size();++i) {
                const auto& e=certificates[i];
                if (e.degree>1) {
                    // Monotone controls in either coordinate prove a leaf is
                    // injective. Otherwise a subpixel cubic loop could hide
                    // inside ONE chord, before inter-edge checks ever see it.
                    bool monotone=false;
                    for (bool y:{false,true}) {
                        const auto value=[&](Point p){return y?p.y:p.x;};
                        const double first=value(e.controls[0]),last=value(e.controls[e.degree]);
                        bool ordered=first!=last;
                        for (unsigned k=0;k<e.degree;++k) {tick();ordered&=first<last?value(e.controls[k])<=value(e.controls[k+1]):value(e.controls[k])>=value(e.controls[k+1]);}
                        monotone|=ordered;
                    }
                    if (!monotone) throw Refine{};
                }
                edges.push_back({p[i],p[(i+1)%p.size()],loop,i,p.size(),certificates[i]});
            }
            ++loop;
        }
        result.positionBound+=2*round;check(result.positionBound<=1.0/8,"precision-limit");
        topology(inputPositionBound);
        std::vector<const Edge*> starts,active;
        for (const auto& e:edges) {tick();if (e.a.y!=e.b.y) starts.push_back(&e);}
        std::sort(starts.begin(),starts.end(),[&](const Edge* a,const Edge* b){tick();return std::min(a->a.y,a->b.y)<std::min(b->a.y,b->b.y);});
        std::size_t next=0;
        for (std::size_t i=1;i<levels.size();++i) {
            const double low=levels[i-1],high=levels[i],mid=low+(high-low)/2;
            check(mid>low && mid<high,"precision-limit");
            active.erase(std::remove_if(active.begin(),active.end(),[&](const Edge* e){tick();return std::max(e->a.y,e->b.y)<=low;}),active.end());
            while (next<starts.size() && std::min(starts[next]->a.y,starts[next]->b.y)<=low) {tick();if (std::max(starts[next]->a.y,starts[next]->b.y)>low) active.push_back(starts[next]);++next;}
            std::sort(active.begin(),active.end(),[&](const Edge* a,const Edge* b){tick();return x(*a,mid)<x(*b,mid);});
            int winding=0;const Edge* left=nullptr;
            for (std::size_t j=0;j<active.size();++j) {
                tick();const auto& e=*active[j];
                if (j) check(x(e,mid)-x(*active[j-1],mid)>2*round,"precision-limit");
                const bool before=rule==FillRule::EvenOdd?(winding%2!=0):(winding!=0);
                winding+=e.b.y>e.a.y?1:-1;
                const bool after=rule==FillRule::EvenOdd?(winding%2!=0):(winding!=0);
                if (!before && after) left=&e;
                if (before && !after) {
                    check(left!=nullptr,"unsupported-fill-topology");
                    const Point a{x(*left,low),low},b{x(e,low),low},c{x(e,high),high},d{x(*left,high),high};
                    triangle(a,b,c);triangle(a,c,d);left=nullptr;
                }
            }
            check(winding==0 && left==nullptr,"unsupported-fill-topology");
        }
        return std::move(result);
    }
};
bool validateGradient(const LinearGradient& gradient,std::size_t stopLimit) {
    check(stopLimit<=1024,"invalid-limits");
    check(gradient.stops.size()<=stopLimit,"gradient-stop-limit");
    check(!gradient.stops.empty(),"empty-gradient");valid(gradient.start);valid(gradient.end);
    check(!same(gradient.start,gradient.end),"degenerate-gradient");
    bool ordered=true;double previous=0;
    for (const auto& s:gradient.stops) {
        check(std::isfinite(s.offset) && s.offset>=0 && s.offset<=1,"invalid-gradient-stop");
        for (double c:s.rgba) check(std::isfinite(c) && c>=0 && c<=1,"invalid-gradient-stop");
        ordered&=previous<=s.offset;previous=s.offset;
    }
    return ordered;
}
void sortStops(LinearGradient& gradient) {
    // At most1024 stops: insertion sort is compact, stable, allocation-free
    // after the owned copy, and has an explicit cross-library quadratic bound.
    for (std::size_t i=1;i<gradient.stops.size();++i)
        for (std::size_t j=i;j>0 && gradient.stops[j].offset<gradient.stops[j-1].offset;--j)
            std::swap(gradient.stops[j],gradient.stops[j-1]);
}
std::array<double,4> sampleStops(const LinearGradient& g,Point point) {
    const auto axis=sub(g.end,g.start),p=sub(point,g.start);
    const double t=dot(p,axis)/dot(axis,axis);check(std::isfinite(t),"precision-limit");
    const auto right=std::upper_bound(g.stops.begin(),g.stops.end(),t,[](double x,const GradientStop& s){return x<s.offset;});
    if (right==g.stops.begin()) return right->rgba;
    if (right==g.stops.end()) return g.stops.back().rgba;
    const auto& a=*(right-1);const auto& b=*right;const double f=(t-a.offset)/(b.offset-a.offset);
    std::array<double,4> out;for (unsigned i=0;i<4;++i) out[i]=a.rgba[i]*(1-f)+b.rgba[i]*f;return out;
}
}
LinearGradient prepareLinearGradient(const LinearGradient& gradient,std::size_t stopLimit) {
    const bool ordered=validateGradient(gradient,stopLimit);auto out=gradient;
    if (!ordered) sortStops(out);
    return out;
}
std::array<double,4> sampleGradient(const LinearGradient& prepared,Point point) {
    // Public values remain mutable: validate every call, but sorted prepared
    // input needs no allocation/copy/sort. Unsorted input gets ONE owned fallback.
    const bool ordered=validateGradient(prepared,1024);valid(point);
    if (ordered) return sampleStops(prepared,point);
    auto sorted=prepared;sortStops(sorted);return sampleStops(sorted,point);
}
PreparedFill prepareFill(const PathCommand& command,FillRule rule,FillLimits limits,const LinearGradient* gradient,double inputPositionBound) {
    check(std::isfinite(inputPositionBound) && inputPositionBound>=0 && inputPositionBound<=1.0/64,"invalid-boundary-error");
    validate(rule,limits);LinearGradient paint;std::size_t work=0;
    if (gradient) {
        check(gradient->stops.size()<=limits.work,"fill-work-limit");
        check(gradient->stops.size()<=1024,"gradient-stop-limit");
        // Reserve validation/outer iterations plus every possible comparison
        // BEFORE copying/sorting; independent of std::stable_sort fallbacks.
        const std::size_t n=gradient->stops.size();work=n*(n+3)/2;
        check(work<=limits.work,"fill-work-limit");limits.work-=work;
        paint=prepareLinearGradient(*gradient);
    }
    double positionError=1.0/16;
    for (unsigned attempt=0;;++attempt) {
        Builder builder{limits,{}, {}, {},0};
        try {
            auto out=builder.run(command,rule,inputPositionBound,positionError);
            out.visited+=work;out.hasGradient=gradient!=nullptr;out.gradient=std::move(paint);return out;
        } catch (Refine) {
            // Refine uncertainty, not the acceptance tolerances. Every discarded
            // attempt is charged before retrying under the original total cap.
            check(attempt<32,"precision-limit");
            check(builder.result.visited<limits.work,"fill-work-limit");
            limits.work-=builder.result.visited;work+=builder.result.visited;positionError/=2;
        }
    }
}
std::vector<PreparedFill> prepareFills(const std::vector<PathCommand>& commands,FillRule rule,FillLimits limits) {
    validate(rule,limits);check(commands.size()<=limits.work && commands.size()<=limits.vertices,"fill-work-limit");
    std::vector<PreparedFill> out;
    for (const auto& c:commands) {
        check(limits.work>0,"fill-work-limit");--limits.work;
        auto fill=prepareFill(c,rule,limits);limits.work-=fill.visited;limits.vertices-=fill.triangles.size()*3;
        ++fill.visited;out.push_back(std::move(fill));
    }
    return out;
}
PreparedHatch prepareHatch(const PathCommand& boundary,const std::vector<HatchFamily>& families,
                          double scale,FillRule rule,HatchLimits limits) {
    check(std::isfinite(scale) && scale>0,"invalid-hatch-scale");validate(rule,limits.fill);
    check(limits.lines<=MaxFrameElements && limits.segments<=MaxFrameElements,"invalid-limits");
    check(families.size()<=limits.fill.work && families.size()<=limits.lines,"hatch-expansion-limit");
    // Validate even empty/degenerate boundaries before returning successful no ink.
    for (const auto& f:families) {
        valid(f.origin);check(std::isfinite(f.angle) && std::isfinite(f.spacing) && f.spacing>0,"invalid-hatch-pattern");
    }
    auto maskLimits=limits.fill;maskLimits.work-=families.size();
    PreparedHatch out;out.mask=prepareFill(boundary,rule,maskLimits);out.visited=out.mask.visited+families.size();
    const std::size_t remainingVertices=limits.fill.vertices-out.mask.triangles.size()*3;
    check(boundary.state.dash.empty(),"unsupported-hatch-pattern");
    const double width=boundary.state.lineWidth+boundary.state.lineWidthCompensation;
    check(std::isfinite(boundary.state.lineWidth) && boundary.state.lineWidth>=0 &&
          std::isfinite(boundary.state.lineWidthCompensation) && std::isfinite(width) && width>=0,"invalid-line-width");
    const double radius=width==0?0.5:width/2;
    auto tick=[&]{check(out.visited<limits.fill.work,"hatch-expansion-limit");++out.visited;};
    std::size_t lines=0;
    for (const auto& f:families) {
        tick();const Point origin=boundary.state.transform.point({f.origin.x*scale,f.origin.y*scale});valid(origin);
        const Point axis=boundary.state.transform.distance({std::cos(f.angle),std::sin(f.angle)});
        const Point delta=boundary.state.transform.distance({-std::sin(f.angle)*f.spacing*scale,std::cos(f.angle)*f.spacing*scale});
        valid(axis);valid(delta);
        const double length=std::hypot(axis.x,axis.y);check(length>0,"precision-limit");
        const Point u{axis.x/length,axis.y/length},n{-u.y,u.x};
        const double spacing=dot(delta,n);check(std::isfinite(spacing) && spacing!=0,"precision-limit");
        if (out.mask.triangles.empty()) continue;
        double low=std::numeric_limits<double>::infinity(),high=-low,alongLow=low,alongHigh=high;
        for (const auto& t:out.mask.triangles) for (Point p:t) {
            tick();const Point v=sub(p,origin);const double k=dot(v,n)/spacing,q=dot(v,u);
            low=std::min(low,k);high=std::max(high,k);alongLow=std::min(alongLow,q);alongHigh=std::max(alongHigh,q);
        }
        const double pad=radius+512*eps*(1+std::max({std::abs(alongLow),std::abs(alongHigh),std::abs(low*spacing),std::abs(high*spacing)}));
        low-=pad/std::abs(spacing);high+=pad/std::abs(spacing);
        const double first=std::ceil(low),last=std::floor(high),count=std::max(0.,last-first+1);
        check(std::abs(first)<4503599627370496. && std::abs(last)<4503599627370496.,"precision-limit");
        check(count<=double(limits.lines-lines),"hatch-expansion-limit");lines+=static_cast<std::size_t>(count);
        for (std::size_t i=0;i<static_cast<std::size_t>(count);++i) {
            tick();const double offset=(first+double(i))*spacing;check(std::isfinite(offset),"precision-limit");
            check(out.segments.size()<limits.segments && out.segments.size()<remainingVertices/2,"hatch-expansion-limit");
            auto p=[&](double t){Point v{origin.x+u.x*t+n.x*offset,origin.y+u.y*t+n.y*offset};valid(v);return v;};
            out.segments.push_back({{p(alongLow-pad),p(alongHigh+pad)}});
        }
    }
    return out;
}
}}}

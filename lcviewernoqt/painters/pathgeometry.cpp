#include "pathgeometry.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <utility>

namespace lc { namespace viewer { namespace render {
namespace {
void check(bool ok, const char* code) { if (!ok) throw std::invalid_argument(code); }
Point valid(Point p) {
    check(std::isfinite(p.x) && std::isfinite(p.y), "invalid-curve"); return p;
}
Point mix(Point a, Point b, double t = 0.5) {
    return valid({a.x*(1-t)+b.x*t, a.y*(1-t)+b.y*t});
}
double distance(Point a, Point b) {
    const double d = std::hypot(a.x-b.x,a.y-b.y); check(std::isfinite(d),"invalid-curve"); return d;
}
double segmentDistance(Point p, Point a, Point b) {
    const double length = distance(a,b);
    if (length == 0) return distance(p,a);
    const double dx = (b.x-a.x)/length, dy = (b.y-a.y)/length;
    const double along = (p.x-a.x)*dx+(p.y-a.y)*dy;
    check(std::isfinite(along),"invalid-curve");
    if (along <= 0) return distance(p,a);
    if (along >= length) return distance(p,b);
    return std::abs((p.x-a.x)*dy-(p.y-a.y)*dx);
}
Point difference(Point a,Point b) {return valid({a.x-b.x,a.y-b.y});}
bool zero(Point p) {return p.x==0 && p.y==0;}
bool exactDifference(double a,double b) {const double x=a-b,z=x-a;return (a-(x-z))-(b+z)==0;}
double size(Point p) {return distance(p,{});}
double mapNorm(const Transform& t) {return std::hypot(size(t.distance({1,0})),size(t.distance({0,1})));}
Point unit(Point p) {const double n=size(p);return n?Point{p.x/n,p.y/n}:Point{};}
struct Builder {
    CurveLimits limits;
    CurveMetrics metrics;
    FlattenedPath result;
    std::size_t vertices = 0;
    double tangentNorm,lengthNorm;
    Builder(CurveLimits l,CurveMetrics m) : limits(l),metrics(m),tangentNorm(mapNorm(m.tangent)),lengthNorm(mapNorm(m.length)) {}
    void tick() {check(result.visited<limits.work,"subdivision-limit");++result.visited;}
    void point(Point p) {
        valid(p); check(vertices < limits.vertices,"vertex-limit");
        result.subpaths.back().points.push_back(p); ++vertices;
    }
    void length(double low, double high) {
        result.lengthLower = std::max(0.,std::nextafter(result.lengthLower+low,-std::numeric_limits<double>::infinity()));
        result.lengthUpper = std::nextafter(result.lengthUpper+high,std::numeric_limits<double>::infinity());
        check(std::isfinite(result.lengthUpper),"invalid-curve");
    }
    Point direction(Point p) const {
        const Point mapped=metrics.tangent.distance(p);
        check(!metrics.physicalRadius || zero(p) || !zero(mapped),"precision-limit");return unit(mapped);
    }
    Point tangent(const std::array<Point,4>& p,unsigned degree,bool end) const {
        for (unsigned i=1;i<=degree;++i) {
            const Point d=end?difference(p[degree],p[degree-i]):difference(p[i],p[0]);
            if (!zero(d)) return direction(d);
        }
        return {};
    }
    void endpoint(Point p,CurveTangents tangents) {
        point(p);if (metrics.physicalRadius) result.subpaths.back().tangents.push_back(tangents);
    }
    void line(Point from,Point end,double budget,double round,Point knownTangent={}) {
        const double d=size(metrics.length.distance(difference(end,from)));
        round*=lengthNorm;check(2*round<budget,"precision-limit");
        length(std::max(0.,d-round),d+round);const Point t=zero(knownTangent)?direction(difference(end,from)):knownTangent;endpoint(end,{t,t});
    }
    // A true collinear span is a line traversal, possibly with reversals. Solve
    // its scalar stationary parameters rather than recursively approaching an
    // irrational cusp from both sides forever. Exact predicates are deliberately
    // conservative; a merely almost-collinear curve must use the normal bound.
    bool collinear(const std::array<Point,4>& p,unsigned degree) const {
        bool horizontal=true,vertical=true;
        for (unsigned i=0;i<=degree;++i) {
            valid(p[i]);horizontal&=p[i].y==p[0].y;vertical&=p[i].x==p[0].x;
        }
        // Exact coordinate equality proves these lines without requiring a
        // possibly inexact subtraction from a distant first control point.
        if (horizontal || vertical) return true;
        Point axis{};
        for (unsigned i=1;i<=degree;++i) {
            const Point d=difference(p[i],p[0]);
            if (!exactDifference(p[i].x,p[0].x) || !exactDifference(p[i].y,p[0].y)) return false;
            if (zero(d)) continue;
            if (zero(axis)) {axis=d;continue;}
            const auto scaled=[](Point p) {
                int exponent=0;std::frexp(std::max(std::abs(p.x),std::abs(p.y)),&exponent);
                return Point{std::ldexp(p.x,-exponent),std::ldexp(p.y,-exponent)};
            };
            const Point u=scaled(axis),v=scaled(d);
            if ((axis.x!=0 && u.x==0) || (axis.y!=0 && u.y==0) || (d.x!=0 && v.x==0) || (d.y!=0 && v.y==0)) return false;
            const double a=u.x*v.y,b=u.y*v.x;
            if (a!=b || std::fma(u.x,v.y,-a)!=std::fma(u.y,v.x,-b)) return false;
        }
        return true;
    }
    void straightCurve(const std::array<Point,4>& p,unsigned degree,double weight,double budget) {
        tick();double magnitude=1;for (unsigned i=0;i<=degree;++i) magnitude=std::max({magnitude,std::abs(p[i].x),std::abs(p[i].y)});
        const double round=1024*std::numeric_limits<double>::epsilon()*magnitude;
        check(round<limits.positionError,"precision-limit");result.positionBound=std::max(result.positionBound,round);
        Point axis{};for (unsigned i=1;i<=degree && zero(axis);++i) axis=difference(p[i],p[0]);
        const Point mappedAxis=metrics.tangent.distance(axis),axisUnit=unit(mappedAxis);
        if (!zero(axis)) {
            check(size(mappedAxis)>0,"precision-limit");
            const double normalRound=256*std::numeric_limits<double>::epsilon()*tangentNorm*size(axis)/size(mappedAxis)*metrics.physicalRadius;
            check(normalRound<=metrics.normalError,"precision-limit");result.normalBound=std::max(result.normalBound,normalRound);
        }
        const auto scalar=[&](Point v){return std::abs(axis.x)>=std::abs(axis.y)?v.x:v.y;};
        bool exact=true,increasing=true,decreasing=true;
        std::array<double,3> d{};for (unsigned i=0;i<degree;++i) {
            d[i]=scalar(difference(p[i+1],p[i]));exact&=exactDifference(scalar(p[i+1]),scalar(p[i]));
            increasing&=d[i]>=0;decreasing&=d[i]<=0;
        }
        unsigned k=degree-1;
        if (weight!=1) {d={{weight*d[0],scalar(difference(p[2],p[0]))/2,weight*d[1]}};k=2;exact=false;}
        double scale=0;for (unsigned i=0;i<=k;++i) scale=std::max(scale,std::abs(d[i]));
        if (scale) {
            int exponent=0;std::frexp(scale,&exponent);
            for (unsigned i=0;i<=k;++i) {
                const double value=std::ldexp(d[i],-exponent);check(d[i]==0 || value!=0,"precision-limit");
                exact&=std::ldexp(value,exponent)==d[i];d[i]=value;
            }
        }
        std::vector<double> cuts;
        const auto root=[&](double t){if (std::isfinite(t) && t>0 && t<1) cuts.push_back(t);};
        bool monotone=increasing || decreasing;
        if (k==2 && exact && ((d[0]>0 && d[2]>0) || (d[0]<0 && d[2]<0))) {
            // A quadratic Bernstein derivative cannot reverse when d0*d2>d1^2.
            // Strict ordering of rounded products, or their FMA residuals when
            // equal, proves this sign even with underflow. Ties prove nothing;
            // never assume extra long-double precision or an epsilon sign.
            const double p=d[0]*d[2],q=d[1]*d[1];
            monotone|=p>q || (p==q && std::fma(d[0],d[2],-p)>std::fma(d[1],d[1],-q));
        }
        if (!monotone && k==1) {if (d[0]!=d[1]) root(d[0]/(d[0]-d[1]));}
        else if (!monotone && k==2) {
            const double a=d[2]-2*d[1]+d[0],b=2*(d[1]-d[0]),c=d[0];
            if (a==0) {if (b) root(-c/b);}
            else {
                const double disc=std::fma(b,b,-4*a*c);
                exact&=exactDifference(d[2],2*d[1]) && exactDifference(d[2]-2*d[1],-d[0]) && exactDifference(d[1],d[0]);
                const double p=b*b,q=4*a*c;
                const bool repeated=exact && p==q && std::fma(b,b,-p)==std::fma(4*a,c,-q);
                const double uncertainty=64*std::numeric_limits<double>::epsilon()*(p+std::abs(q));
                check(repeated || std::abs(disc)>uncertainty,"precision-limit");
                // A provably repeated root does not reverse a collinear span.
                if (!repeated && disc>0) {const double v=-.5*(b+std::copysign(std::sqrt(disc),b));root(v/a);if(v) root(c/v);}
            }
        }
        std::sort(cuts.begin(),cuts.end());cuts.erase(std::unique(cuts.begin(),cuts.end()),cuts.end());cuts.push_back(1);
        Point from=p[0];const double share=budget/double(cuts.size());
        for (double t:cuts) {
            tick();Point end;
            if (t==1) end=p[degree];
            else if (weight==1) {auto q=p;for (unsigned n=degree;n;--n) for (unsigned i=0;i<n;++i) q[i]=mix(q[i],q[i+1],t);end=q[0];}
            else {
                const double u=1-t,w=u*u+2*weight*u*t+t*t;
                check(w>0 && std::isfinite(w),"precision-limit");
                end=valid({(u*u*p[0].x+2*weight*u*t*p[1].x+t*t*p[2].x)/w,
                           (u*u*p[0].y+2*weight*u*t*p[1].y+t*t*p[2].y)/w});
            }
            Point tangentDirection=axisUnit;
            if (!zero(axis)) {
                const double delta=scalar(difference(end,from));check(std::abs(delta)>2*round,"precision-limit");
                if ((delta>0)!=(scalar(axis)>0)) tangentDirection={-tangentDirection.x,-tangentDirection.y};
            }
            line(from,end,share,round,tangentDirection);from=end;
        }
    }
    void curve(std::array<Point,4> p, unsigned degree, double weight, double budget, unsigned depth,Point start,Point end) {
        tick();
        double upper = 0, error = 0, magnitude = 1;
        for (unsigned i = 0; i <= degree; ++i) {
            valid(p[i]); magnitude = std::max({magnitude,std::abs(p[i].x),std::abs(p[i].y)});
            if (i) upper += size(metrics.length.distance(difference(p[i],p[i-1])));
            if (i && i < degree) error = std::max(error,segmentDistance(p[i],p[0],p[degree]));
        }
        check(std::isfinite(upper),"invalid-curve");
        const Point chord=difference(p[degree],p[0]);
        const double lower = size(metrics.length.distance(chord));
        // Fail rather than certify detail below the coordinate domain's double
        // resolution. Local-origin recording keeps ordinary CAD detail small.
        const double round = 128*(depth+1)*std::numeric_limits<double>::epsilon()*magnitude;
        check(round < limits.positionError,"precision-limit");
        const double lengthRound = 8*round*lengthNorm;
        check(2*lengthRound < budget,"precision-limit");
        double normal=0,positionRound=round;
        if (metrics.physicalRadius) {
            // Parametric chord bounds let the same (t,s) domain certify both
            // the normal sweep and its two-triangle strip, including folds.
            error=0;double metricError=0;
            const auto defect=[&](Point d) {
                error=std::max(error,size(d));
                if (metrics.dashed) metricError=std::max(metricError,size(metrics.length.distance(d)));
            };
            if (weight==1) for (unsigned i=1;i<degree;++i) defect(difference(p[i],mix(p[0],p[degree],double(i)/degree)));
            else {
                const Point a=difference(p[1],p[0]),b=difference(p[1],p[2]);
                defect(valid({(2*weight*a.x-chord.x)/(3*weight),(2*weight*a.y-chord.y)/(3*weight)}));
                defect(valid({(2*weight*b.x+chord.x)/(3*weight),(2*weight*b.y+chord.y)/(3*weight)}));
            }
            // A dash's arc-length parameter is not the curve's t. Account for
            // the oblique metric projection as well as physical displacement.
            if (metrics.dashed) {error+=metricError*metrics.lengthScale;positionRound*=1+lengthNorm*metrics.lengthScale;}
            check(std::isfinite(error) && positionRound<limits.positionError,"precision-limit");
            const Point axis=direction(chord);double cone=0,minEdge=std::numeric_limits<double>::infinity();
            for (unsigned i=1;i<=degree;++i) {
                const Point raw=difference(p[i],p[i-1]),v=metrics.tangent.distance(raw);const double n=size(v);
                check(zero(raw) || n>0,"precision-limit");
                if (n==0) continue;
                minEdge=std::min(minEdge,n);const Point u=unit(v);
                cone=std::max(cone, u.x*axis.x+u.y*axis.y<=0?2.:distance(u,axis));
            }
            cone=std::max({cone,distance(start,axis),distance(end,axis)});
            cone=std::min(2.,cone+8*round*tangentNorm/minEdge);
            normal=4*metrics.physicalRadius*cone;
            if (normal>metrics.normalError) check(minEdge>8*round*tangentNorm,"precision-limit");
            if (zero(chord)) normal=std::numeric_limits<double>::infinity();
        }
        if (error+positionRound <= limits.positionError && normal<=metrics.normalError && upper-lower+2*lengthRound <= budget) {
            result.positionBound = std::max(result.positionBound,error+positionRound);
            result.normalBound=std::max(result.normalBound,normal);
            length(std::max(0.,lower-lengthRound),std::max(lower,upper)+lengthRound);endpoint(p[degree],{start,end});return;
        }
        check(depth < limits.depth,"subdivision-limit");
        std::array<Point,4> left{}, right{};
        double childWeight = 1;
        if (weight != 1) {
            // Homogeneous bisection, reparameterized to unit endpoint weights.
            // Every child remains a positive convex conic; no cubic arc error.
            left[0] = p[0]; right[2] = p[2];
            left[1] = mix(p[0],p[1],weight/(1+weight));
            right[1] = mix(p[1],p[2],1/(1+weight));
            left[2] = right[0] = mix(left[1],right[1]);
            childWeight = std::sqrt((1+weight)/2);
        } else {
            auto levels = p; left[0] = p[0]; right[degree] = p[degree];
            for (unsigned n = degree; n; --n) {
                for (unsigned i = 0; i < n; ++i) levels[i] = mix(levels[i],levels[i+1]);
                left[degree-n+1] = levels[0]; right[n-1] = levels[n-1];
            }
        }
        const Point shared=difference(right[1],left[degree-1]);
        const Point a=zero(shared)?tangent(left,degree,true):direction(shared);
        const Point b=zero(shared)?tangent(right,degree,false):a;
        curve(left,degree,childWeight,budget/2,depth+1,start,a);
        curve(right,degree,childWeight,budget/2,depth+1,b,end);
    }
};
}
FlattenedPath flatten(const PathCommand& command, CurveLimits limits,CurveMetrics metrics) {
    check(std::isfinite(limits.positionError) && limits.positionError > 0 && limits.positionError <= 1.0/16 &&
          std::isfinite(limits.lengthError) && limits.lengthError > 0 && limits.lengthError <= 1.0/32 &&
          limits.vertices <= MaxFrameElements && limits.work <= 4*MaxFrameElements && limits.depth <= 32,"invalid-limits");
    metrics.tangent.validate();metrics.length.validate();
    check(std::isfinite(metrics.physicalRadius) && metrics.physicalRadius>=0 && std::isfinite(metrics.normalError) &&
          metrics.normalError>0 && metrics.normalError<=1.0/32 && std::isfinite(metrics.lengthScale) && metrics.lengthScale>0,"invalid-limits");
    check(command.path.size() <= MaxFrameElements,"vertex-limit");
    // Bound even the preflight scan and move/line-only paths. Recursive curve
    // nodes consume the same budget in addition to these input elements.
    check(command.path.size() <= limits.work,"subdivision-limit");
    std::size_t edges = 0;
    for (const auto& e : command.path) if (e.verb != PathVerb::Move) ++edges;
    const double total=limits.lengthError/metrics.lengthScale;
    const double budget = edges ? total/(2*double(edges)) : total;
    Builder b(limits,metrics); bool current = false;
    for (const auto& e : command.path) {
        b.tick();
        if (e.verb == PathVerb::Move) {
            check(b.result.subpaths.size() < limits.vertices,"vertex-limit");
            b.result.subpaths.push_back({}); b.point(e.point); current = true; continue;
        }
        check(current && !b.result.subpaths.back().closed,"invalid-path");
        const Point from = b.result.subpaths.back().points.back();
        if (e.verb == PathVerb::Line || e.verb == PathVerb::Close) {
            valid(e.point);
            const Point end = e.verb == PathVerb::Close ? b.result.subpaths.back().points.front() : e.point;
            const double mag = std::max({1.,std::abs(from.x),std::abs(from.y),std::abs(end.x),std::abs(end.y)});
            const double round = 1024*std::numeric_limits<double>::epsilon()*mag;
            b.line(from,end,budget,round);b.result.subpaths.back().closed = e.verb == PathVerb::Close;
        } else {
            check(e.verb == PathVerb::Quadratic || e.verb == PathVerb::Cubic || e.verb == PathVerb::Conic,"invalid-path");
            const bool cubic = e.verb == PathVerb::Cubic;
            check(e.verb != PathVerb::Conic || (std::isfinite(e.weight) && e.weight > 0 && e.weight <= 1),"invalid-curve");
            const std::array<Point,4> p = cubic ? std::array<Point,4>{{from,e.control1,e.control2,e.point}} :
                std::array<Point,4>{{from,e.control1,e.point,{}}};
            const unsigned degree=cubic?3:2;const double w=e.verb==PathVerb::Conic?e.weight:1;
            const auto before=b.result.subpaths.back().tangents.size();
            if (metrics.physicalRadius && b.collinear(p,degree)) b.straightCurve(p,degree,w,budget);
            else b.curve(p,degree,w,budget,0,b.tangent(p,degree,false),b.tangent(p,degree,true));
            // Only subdivisions of this input segment receive cusp continuity.
            // User joins and dash endpoints retain their selected decorations.
            auto& tangents=b.result.subpaths.back().tangents;
            for (std::size_t i=before+1;i<tangents.size();++i) tangents[i].internal=true;
        }
    }
    check((b.result.lengthUpper-b.result.lengthLower)*metrics.lengthScale <= limits.lengthError,"precision-limit");
    return std::move(b.result);
}
}}}

#pragma once
#include <array>
#include <cmath>
#include <limits>
#include <stdexcept>

namespace lc { namespace viewer { namespace render { namespace detail {
// Bounded error-free expansion for affine products and origin cancellation.
// No extended-precision assumption, heap allocation or per-consumer arithmetic.
struct Sum {
    explicit Sum(const char* diagnostic = "precision-limit") : diagnostic(diagnostic) {}
    std::array<double,64> a{};
    unsigned n = 0;
    const char* diagnostic;
    bool products = false;
    void check(bool ok) const { if (!ok) throw std::invalid_argument(diagnostic); }
    void add(double x) {
        check(std::isfinite(x)); unsigned m=0;
        for (unsigned i=0;i<n;++i) {
            const double v=a[i],s=x+v,b=s-x,r=(x-(s-b))+(v-b);
            check(std::isfinite(s));
            if (r) {check(m<a.size());a[m++]=r;} x=s;
        }
        if (x || !m) {check(m<a.size());a[m++]=x;} n=m;
    }
    void product(double x,double y) {
        const double p=x*y;
        check(std::isfinite(p));
        // Zero/sign/identity products are exact even for subnormal controls.
        if (!x || !y || std::abs(x)==1 || std::abs(y)==1) {add(p);return;}
        check(std::abs(p)>=std::numeric_limits<double>::min());products=true;
        add(std::fma(x,y,-p));add(p);
    }
    Sum scaled(double x) const {Sum out(diagnostic);for (unsigned i=0;i<n;++i) out.product(a[i],x);return out;}
    void append(const Sum& s) {for (unsigned i=0;i<s.n;++i) add(s.a[i]);products|=s.products;}
    double value() const {return n?a[n-1]:0;}
    double error() const {
        double e=products?std::numeric_limits<double>::denorm_min()*1048576:0;
        for (unsigned i=0;i+1<n;++i) e=std::nextafter(e+std::abs(a[i]),std::numeric_limits<double>::infinity());
        return e;
    }
};
}}}}

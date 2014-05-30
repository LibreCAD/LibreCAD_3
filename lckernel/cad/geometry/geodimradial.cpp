
#include "geodimradial.h"

using namespace lc;
using namespace geo;

DimRadial::DimRadial(const Dimension& dimension, const double leader) : _leader(leader) {
}

double DimRadial::leader() const {
    return _leader;
}

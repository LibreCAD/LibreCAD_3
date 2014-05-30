#include "geodimdiametric.h"

using namespace lc;
using namespace geo;

DimDiametric::DimDiametric(const Dimension& dimension, const double leader) : _leader(leader){
}

double DimDiametric::leader() const {
    return _leader;
}

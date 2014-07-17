#ifndef ENTITYTEST_H_
#define ENTITYTEST_H_
#include <vector>
#include <cad/primitive/line.h>
#include <cad/meta/layer.h>
#include "cad/primitive/coordinate.h"
#include "cad/meta/color.h"
#include "cad/primitive/line.h"
#include "cad/primitive/circle.h"
#include "cad/primitive/arc.h"
#include "cad/primitive/ellipse.h"
#include "cad/meta/linewidth.h"
#include "cad/interface/metatype.h"
#include "cad/document/selectionmanager.h"
#include "cad/dochelpers/storagemanagerimpl.h"
#include "cad/dochelpers/undomanagerimpl.h"
#include "cad/dochelpers/documentimpl.h"
#include "cad/operations/documentoperation.h"
#include <cad/document/selectionmanager.h>
#include <cad/document/storagemanager.h>
#include <cad/operations/builder.h>
#include "cad/document/document.h"
#include "cad/document/undomanager.h"

using namespace std;
using namespace lc;
class entitytest {
    public:
        entitytest();

        Line_CSPtr LineMove(geo::Coordinate &);
        Line_CSPtr LineCopy(geo::Coordinate &);
        Line_CSPtr LineRotate(geo::Coordinate &, double);
        Line_CSPtr LineScale(geo::Coordinate &, geo::Coordinate &);

        Circle_CSPtr CircleMove(geo::Coordinate &);
        Circle_CSPtr CircleCopy(geo::Coordinate &);
        Circle_CSPtr CircleRotate(geo::Coordinate &, double);
        Circle_CSPtr CircleScale(geo::Coordinate &, geo::Coordinate &);

        Arc_CSPtr ArcMove(geo::Coordinate &);
        Arc_CSPtr ArcCopy(geo::Coordinate &);
        Arc_CSPtr ArcRotate(geo::Coordinate &, double);
        Arc_CSPtr ArcScale(geo::Coordinate &, geo::Coordinate &);

        Ellipse_CSPtr EllipseMove(geo::Coordinate &);
        Ellipse_CSPtr EllipseCopy(geo::Coordinate &);
        Ellipse_CSPtr EllipseRotate(geo::Coordinate &, double);
        Ellipse_CSPtr EllipseScale(geo::Coordinate &, geo::Coordinate &);

};

#endif

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

        std::vector<Line_CSPtr> LineMove(geo::Coordinate &);
        std::vector<Line_CSPtr> LineCopy(geo::Coordinate &);
        std::vector<Line_CSPtr> LineRotate(geo::Coordinate &, double);
        std::vector<Line_CSPtr> LineScale(geo::Coordinate &, geo::Coordinate &);

        std::vector<Circle_CSPtr> CircleMove(geo::Coordinate &);
        std::vector<Circle_CSPtr> CircleCopy(geo::Coordinate &);
        std::vector<Circle_CSPtr> CircleRotate(geo::Coordinate &, double);
        std::vector<Circle_CSPtr> CircleScale(geo::Coordinate &, geo::Coordinate &);

        std::vector<Arc_CSPtr> ArcMove(geo::Coordinate &);
        std::vector<Arc_CSPtr> ArcCopy(geo::Coordinate &);
        std::vector<Arc_CSPtr> ArcRotate(geo::Coordinate &, double);
        std::vector<Arc_CSPtr> ArcScale(geo::Coordinate &, geo::Coordinate &);

        std::vector<Ellipse_CSPtr> EllipseMove(geo::Coordinate &);
        std::vector<Ellipse_CSPtr> EllipseCopy(geo::Coordinate &);
        std::vector<Ellipse_CSPtr> EllipseRotate(geo::Coordinate &, double);
        std::vector<Ellipse_CSPtr> EllipseScale(geo::Coordinate &, geo::Coordinate &);

};

#endif

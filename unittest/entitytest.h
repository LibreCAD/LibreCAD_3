#pragma once

#include <vector>
#include <cad/primitive/line.h>
#include <cad/meta/layer.h>
#include "cad/primitive/coordinate.h"
#include "cad/meta/color.h"
#include "cad/primitive/line.h"
#include "cad/primitive/circle.h"
#include "cad/primitive/arc.h"
#include "cad/primitive/ellipse.h"
#include "cad/meta/metalinewidth.h"
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

        std::vector<Line_CSPtr> LineMove();
        std::vector<Line_CSPtr> LineCopy();
        std::vector<Line_CSPtr> LineRotate();
        std::vector<Line_CSPtr> LineScale();

        std::vector<Circle_CSPtr> CircleMove();
        std::vector<Circle_CSPtr> CircleCopy();
        std::vector<Circle_CSPtr> CircleRotate();
        std::vector<Circle_CSPtr> CircleScale();

        std::vector<Arc_CSPtr> ArcMove();
        std::vector<Arc_CSPtr> ArcCopy();
        std::vector<Arc_CSPtr> ArcRotate();
        std::vector<Arc_CSPtr> ArcScale();

        std::vector<Ellipse_CSPtr> EllipseMove();
        std::vector<Ellipse_CSPtr> EllipseCopy();
        std::vector<Ellipse_CSPtr> EllipseRotate();
        std::vector<Ellipse_CSPtr> EllipseScale();

};

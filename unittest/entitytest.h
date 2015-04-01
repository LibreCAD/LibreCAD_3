#pragma once

#include <vector>
#include <cad/primitive/line.h>
#include <cad/meta/layer.h>
#include <cad/primitive/point.h>
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

class entitytest {
    public:
        entitytest();

        std::vector<lc::entity::Line_CSPtr> LineMove();
        std::vector<lc::entity::Line_CSPtr> LineCopy();
        std::vector<lc::entity::Line_CSPtr> LineRotate();
        std::vector<lc::entity::Line_CSPtr> LineScale();

        std::vector<lc::entity::Circle_CSPtr> CircleMove();
        std::vector<lc::entity::Circle_CSPtr> CircleCopy();
        std::vector<lc::entity::Circle_CSPtr> CircleRotate();
        std::vector<lc::entity::Circle_CSPtr> CircleScale();

        std::vector<lc::entity::Arc_CSPtr> ArcMove();
        std::vector<lc::entity::Arc_CSPtr> ArcCopy();
        std::vector<lc::entity::Arc_CSPtr> ArcRotate();
        std::vector<lc::entity::Arc_CSPtr> ArcScale();

        std::vector<lc::entity::Ellipse_CSPtr> EllipseMove();
        std::vector<lc::entity::Ellipse_CSPtr> EllipseCopy();
        std::vector<lc::entity::Ellipse_CSPtr> EllipseRotate();
        std::vector<lc::entity::Ellipse_CSPtr> EllipseScale();

};

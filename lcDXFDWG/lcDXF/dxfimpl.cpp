#include "dxfimpl.h"

#include <cad/primitive/circle.h>
#include <cad/primitive/arc.h>
#include <cad/primitive/ellipse.h>
#include <cad/primitive/text.h>
#include <cad/primitive/dimradial.h>
#include <cad/primitive/dimdiametric.h>
#include <cad/primitive/dimlinear.h>
#include <cad/primitive/dimaligned.h>
#include <cad/primitive/dimangular.h>
#include <cad/primitive/point.h>
#include <cad/primitive/spline.h>
#include <cad/primitive/lwpolyline.h>
#include <cad/operations/builder.h>
#include <cad/meta/layer.h>
#include <cad/operations/layerops.h>
#include <cad/meta/color.h>
#include <cad/meta/icolor.h>
#include "cad/base/metainfo.h"
#include "cad/meta/metacolor.h"
#include "cad/meta/metalinewidth.h"

DXFimpl::DXFimpl(lc::Document* d, lc::operation::Builder_SPtr builder) : _document(d), _builder(builder), _blockHandle(-1) {
    _intToLineWidth[0] = std::make_shared<lc::MetaLineWidth>(0.00);
    _intToLineWidth[1] = std::make_shared<lc::MetaLineWidth>(0.05);
    _intToLineWidth[2] = std::make_shared<lc::MetaLineWidth>(0.09);
    _intToLineWidth[3] = std::make_shared<lc::MetaLineWidth>(0.13);
    _intToLineWidth[4] = std::make_shared<lc::MetaLineWidth>(0.15);
    _intToLineWidth[5] = std::make_shared<lc::MetaLineWidth>(0.18);
    _intToLineWidth[6] = std::make_shared<lc::MetaLineWidth>(0.20);
    _intToLineWidth[7] = std::make_shared<lc::MetaLineWidth>(0.25);
    _intToLineWidth[8] = std::make_shared<lc::MetaLineWidth>(0.30);
    _intToLineWidth[9] = std::make_shared<lc::MetaLineWidth>(0.35);
    _intToLineWidth[10] = std::make_shared<lc::MetaLineWidth>(0.40);
    _intToLineWidth[11] = std::make_shared<lc::MetaLineWidth>(0.50);
    _intToLineWidth[12] = std::make_shared<lc::MetaLineWidth>(0.53);
    _intToLineWidth[13] = std::make_shared<lc::MetaLineWidth>(0.60);
    _intToLineWidth[14] = std::make_shared<lc::MetaLineWidth>(0.70);
    _intToLineWidth[15] = std::make_shared<lc::MetaLineWidth>(0.80);
    _intToLineWidth[16] = std::make_shared<lc::MetaLineWidth>(0.90);
    _intToLineWidth[17] = std::make_shared<lc::MetaLineWidth>(1.00);
    _intToLineWidth[18] = std::make_shared<lc::MetaLineWidth>(1.06);
    _intToLineWidth[19] = std::make_shared<lc::MetaLineWidth>(1.20);
    _intToLineWidth[20] = std::make_shared<lc::MetaLineWidth>(1.40);
    _intToLineWidth[21] = std::make_shared<lc::MetaLineWidth>(1.58);
    _intToLineWidth[22] = std::make_shared<lc::MetaLineWidth>(2.0);
    _intToLineWidth[23] = std::make_shared<lc::MetaLineWidth>(2.11);

}

void DXFimpl::setBlock(const int _blockHandle) {
    std::cout << "setBlock " << _blockHandle << "\n";
}
void DXFimpl::addBlock(const DRW_Block& data) {
    _blockHandle = data.handle;
    std::cout << "addBlock " << data.name << " " << data.handle << "\n";
}
void DXFimpl::endBlock() {
    std::cout << "endBlock " << "\n";
    _blockHandle = -1;
}


void DXFimpl::addLine(const DRW_Line& data) {
    if (_blockHandle != -1) {
        return;
    }

    std::shared_ptr<lc::MetaInfo> mf = getMetaInfo(data);
    auto layer = _document->layerByName(data.layer);
    _builder->append(std::make_shared<lc::entity::Line>(coord(data.basePoint), coord(data.secPoint), layer, mf));
}

void DXFimpl::addCircle(const DRW_Circle& data) {
    if (_blockHandle != -1) {
        return;
    }

    std::shared_ptr<lc::MetaInfo> mf = getMetaInfo(data);
    auto layer = _document->layerByName(data.layer);
    _builder->append(std::make_shared<lc::entity::Circle>(coord(data.basePoint), data.radious, layer, mf));
}

void DXFimpl::addArc(const DRW_Arc& data) {
    if (_blockHandle != -1) {
        return;
    }

    std::shared_ptr<lc::MetaInfo> mf = getMetaInfo(data);
    auto layer = _document->layerByName(data.layer);
    _builder->append(std::make_shared<lc::entity::Arc>(coord(data.basePoint),
                                               data.radious,
                                               data.staangle, data.endangle, layer, mf));

}

void DXFimpl::addEllipse(const DRW_Ellipse& data) {
    if (_blockHandle != -1) {
        return;
    }

    std::shared_ptr<lc::MetaInfo> mf = getMetaInfo(data);
    auto layer = _document->layerByName(data.layer);
    _builder->append(std::make_shared<lc::entity::Ellipse>(coord(data.basePoint),
                                                   coord(data.secPoint),
                                                   coord(data.basePoint).distanceTo(coord(data.secPoint)) / data.ratio,
                                                   data.staparam, data.endparam,
                                                   layer, mf));
}

void DXFimpl::addLayer(const DRW_Layer& data) {
    auto col = icol.intToColor(data.color);

    if (col == nullptr) {
        col = icol.intToColor(255);
    }

    auto lw = getLcLineWidth(data.lWeight);

    if (lw == nullptr) {
        lw = getLcLineWidth(DRW_LW_Conv::lineWidth::width00);
    }

    auto layer = std::make_shared<lc::Layer>(data.name, lw->width(), col->color());
    auto al = std::make_shared<lc::operation::AddLayer>(_document, layer);
    al->execute();

}

void DXFimpl::addSpline(const DRW_Spline& data) {
    if (_blockHandle != -1) {
        return;
    }

    std::shared_ptr<lc::MetaInfo> mf = getMetaInfo(data);
    auto layer = _document->layerByName(data.layer);

    _builder->append(std::make_shared<lc::entity::Spline>(coords(data.controllist),
            data.knotslist,
            coords(data.fitlist),
            data.degree,
            false,
            data.tolfit,
            data.tgsx, data.tgsy, data.tgsz,
            data.tgex, data.tgey, data.tgez,
            data.ex, data.ey, data.ez, layer, mf));

}

void DXFimpl::addText(const DRW_Text& data) {
    if (_blockHandle != -1) {
        return;
    }

    std::shared_ptr<lc::MetaInfo> mf = getMetaInfo(data);
    auto layer = _document->layerByName(data.layer);
    _builder->append(std::make_shared<lc::entity::Text>(coord(data.basePoint),
            data.text, data.height,
            data.angle, data.style,
            lc::TextConst::DrawingDirection(data.textgen),
            lc::TextConst::HAlign(data.alignH),
            lc::TextConst::VAlign(data.alignV),
            layer, mf));
}

void DXFimpl::addPoint(const DRW_Point& data) {
    if (_blockHandle != -1) {
        return;
    }

    std::shared_ptr<lc::MetaInfo> mf = getMetaInfo(data);
    auto layer = _document->layerByName(data.layer);
    _builder->append(std::make_shared<lc::entity::Point>(coord(data.basePoint), layer, mf));
}

void DXFimpl::addDimAlign(const DRW_DimAligned* data) {
    if (_blockHandle != -1) {
        return;
    }

    auto mf = getMetaInfo(*data);
    auto layer = _document->layerByName(data->layer);

    _builder->append(std::make_shared<lc::entity::DimAligned>(
                         coord(data->getDefPoint()),
                         coord(data->getTextPoint()),
                         static_cast<lc::TextConst::AttachmentPoint>(data->getAlign()),
                         data->getDir(),
                         data->getTextLineFactor(),
                         static_cast<lc::TextConst::LineSpacingStyle>(data->getTextLineStyle()),
                         data->getText(),
                         coord(data->getDef1Point()),
                         coord(data->getDef2Point()),
                         layer, mf));
}

void DXFimpl::addDimLinear(const DRW_DimLinear* data) {
    if (_blockHandle != -1) {
        return;
    }

    auto mf = getMetaInfo(*data);
    auto layer = _document->layerByName(data->layer);

    _builder->append(std::make_shared<lc::entity::DimLinear>(
                         coord(data->getDefPoint()),
                         coord(data->getTextPoint()),
                         static_cast<lc::TextConst::AttachmentPoint>(data->getAlign()),
                         data->getDir(),
                         data->getTextLineFactor(),
                         static_cast<lc::TextConst::LineSpacingStyle>(data->getTextLineStyle()),
                         data->getText(),
                         coord(data->getDef1Point()),
                         coord(data->getDef2Point()),
                         data->getAngle(),
                         data->getOblique(),
                         layer, mf));

}

void DXFimpl::addDimRadial(const DRW_DimRadial* data) {
    if (_blockHandle != -1) {
        return;
    }

    auto mf = getMetaInfo(*data);
    auto layer = _document->layerByName(data->layer);
    _builder->append(std::make_shared<lc::entity::DimRadial>(
                         coord(data->getCenterPoint()),
                         coord(data->getTextPoint()),
                         static_cast<lc::TextConst::AttachmentPoint>(data->getAlign()),
                         data->getDir(),
                         data->getTextLineFactor(),
                         static_cast<lc::TextConst::LineSpacingStyle>(data->getTextLineStyle()),
                         data->getText(),
                         coord(data->getDiameterPoint()),
                         data->getLeaderLength(),
                         layer, mf));

}

void DXFimpl::addDimDiametric(const DRW_DimDiametric* data) {

    if (_blockHandle != -1) {
        return;
    }

    auto mf = getMetaInfo(*data);
    auto layer = _document->layerByName(data->layer);

    _builder->append(std::make_shared<lc::entity::DimDiametric>(
                         coord(data->getDiameter1Point()),
                         coord(data->getTextPoint()),
                         static_cast<lc::TextConst::AttachmentPoint>(data->getAlign()),
                         data->getDir(),
                         data->getTextLineFactor(),
                         static_cast<lc::TextConst::LineSpacingStyle>(data->getTextLineStyle()),
                         data->getText(),
                         coord(data->getDiameter2Point()),
                         data->getLeaderLength(),
                         layer, mf));

}

void DXFimpl::addDimAngular(const DRW_DimAngular* data) {
    if (_blockHandle != -1) {
        return;
    }

    auto mf = getMetaInfo(*data);
    auto layer = _document->layerByName(data->layer);

    _builder->append(std::make_shared<lc::entity::DimAngular>(
                         coord(data->getDefPoint()),
                         coord(data->getTextPoint()),
                         static_cast<lc::TextConst::AttachmentPoint>(data->getAlign()),
                         data->getDir(),
                         data->getTextLineFactor(),
                         static_cast<lc::TextConst::LineSpacingStyle>(data->getTextLineStyle()),
                         data->getText(),
                         coord(data->getFirstLine1()),
                         coord(data->getFirstLine2()),
                         coord(data->getSecondLine1()),
                         coord(data->getSecondLine2()),
                         layer, mf));
}

void DXFimpl::addDimAngular3P(const DRW_DimAngular3p* data) {
}

void DXFimpl::addDimOrdinate(const DRW_DimOrdinate* data) {
}

void DXFimpl::addLWPolyline(const DRW_LWPolyline& data) {
    if (_blockHandle != -1) {
        return;
    }

    auto mf = getMetaInfo(data);
    auto layer = _document->layerByName(data.layer);

    std::vector<lc::entity::LWVertex2D> points;
    for (auto i : data.vertlist) {
        points.emplace_back(lc::geo::Coordinate(i->x, i->y), i->bulge, i->stawidth, i->endwidth);
    }

    auto isCLosed = data.flags&0x01;

    _builder->append(std::make_shared<lc::entity::LWPolyline>(
            points, data.width, data.elevation, data.thickness,  isCLosed, coord(data.extPoint),layer, mf
    ));
}

void DXFimpl::addPolyline(const DRW_Polyline& data) {
}

void DXFimpl::addSpline(const DRW_Spline* data) {

//    data->
}


void DXFimpl::addMText(const DRW_MText& data) {
}

void DXFimpl::addHatch(const DRW_Hatch* data) {
}

std::shared_ptr<lc::MetaLineWidth> DXFimpl::getLcLineWidth(DRW_LW_Conv::lineWidth lw) const {
    std::shared_ptr<lc::MetaLineWidth> mlw = nullptr;

    switch (lw) {
        case DRW_LW_Conv::lineWidth::widthDefault:
        case DRW_LW_Conv::lineWidth::widthByLayer:
        case DRW_LW_Conv::lineWidth::widthByBlock: // TODO understand how 'by block' works
            return nullptr;

        default:
            if (lw >= 0 && lw < 24) {
                mlw = DXFimpl::_intToLineWidth[lw];
            }
    }

    return mlw;
}

lc::MetaInfo_SPtr DXFimpl::getMetaInfo(const DRW_Entity& data) const {
    std::shared_ptr<lc::MetaInfo> mf = nullptr;

    auto lw = getLcLineWidth(data.lWeight);

    if (lw != nullptr) {
        if (mf == nullptr) {
            mf = lc::MetaInfo::create();
        }

        mf->add(lw);
    }

    auto col = icol.intToColor(data.color);

    if (col != nullptr) {
        if (mf == nullptr) {
            mf = lc::MetaInfo::create();
        }

        mf->add(col);
    }
    return mf;
}

lc::geo::Coordinate DXFimpl::coord(DRW_Coord const& coord) const {
    return lc::geo::Coordinate(coord.x, coord.y, coord.z);
}

std::vector<lc::geo::Coordinate> DXFimpl::coords(std::vector<DRW_Coord *> coordList) const {
    std::vector<lc::geo::Coordinate> coords;
    for (const DRW_Coord *ptr : coordList) {
        coords.emplace_back(ptr->x,ptr->y, ptr->z);
    }
    return coords;
}

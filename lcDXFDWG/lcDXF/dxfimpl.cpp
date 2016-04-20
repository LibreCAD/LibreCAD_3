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
#include <cad/operations/linepatternops.h>
#include <cad/meta/color.h>
#include <cad/meta/icolor.h>
#include <cad/base/metainfo.h>
#include <cad/meta/metacolor.h>
#include <cad/meta/metalinewidth.h>
#include <cad/meta/dxflinepattern.h>
#include <cad/functions/string_helper.h>

DXFimpl::DXFimpl(std::shared_ptr<lc::Document> document, lc::operation::Builder_SPtr builder) : _document(document), _builder(builder), _blockHandle(-1) {
    _intToLineWidth[0] = std::make_shared<lc::MetaLineWidthByValue>(0.00);
    _intToLineWidth[1] = std::make_shared<lc::MetaLineWidthByValue>(0.05);
    _intToLineWidth[2] = std::make_shared<lc::MetaLineWidthByValue>(0.09);
    _intToLineWidth[3] = std::make_shared<lc::MetaLineWidthByValue>(0.13);
    _intToLineWidth[4] = std::make_shared<lc::MetaLineWidthByValue>(0.15);
    _intToLineWidth[5] = std::make_shared<lc::MetaLineWidthByValue>(0.18);
    _intToLineWidth[6] = std::make_shared<lc::MetaLineWidthByValue>(0.20);
    _intToLineWidth[7] = std::make_shared<lc::MetaLineWidthByValue>(0.25);
    _intToLineWidth[8] = std::make_shared<lc::MetaLineWidthByValue>(0.30);
    _intToLineWidth[9] = std::make_shared<lc::MetaLineWidthByValue>(0.35);
    _intToLineWidth[10] = std::make_shared<lc::MetaLineWidthByValue>(0.40);
    _intToLineWidth[11] = std::make_shared<lc::MetaLineWidthByValue>(0.50);
    _intToLineWidth[12] = std::make_shared<lc::MetaLineWidthByValue>(0.53);
    _intToLineWidth[13] = std::make_shared<lc::MetaLineWidthByValue>(0.60);
    _intToLineWidth[14] = std::make_shared<lc::MetaLineWidthByValue>(0.70);
    _intToLineWidth[15] = std::make_shared<lc::MetaLineWidthByValue>(0.80);
    _intToLineWidth[16] = std::make_shared<lc::MetaLineWidthByValue>(0.90);
    _intToLineWidth[17] = std::make_shared<lc::MetaLineWidthByValue>(1.00);
    _intToLineWidth[18] = std::make_shared<lc::MetaLineWidthByValue>(1.06);
    _intToLineWidth[19] = std::make_shared<lc::MetaLineWidthByValue>(1.20);
    _intToLineWidth[20] = std::make_shared<lc::MetaLineWidthByValue>(1.40);
    _intToLineWidth[21] = std::make_shared<lc::MetaLineWidthByValue>(1.58);
    _intToLineWidth[22] = std::make_shared<lc::MetaLineWidthByValue>(2.0);
    _intToLineWidth[23] = std::make_shared<lc::MetaLineWidthByValue>(2.11);

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
    // std::cerr << data.staangle << ":" << data.endangle << ":" << data.isccw << "\n";
    _builder->append(std::make_shared<lc::entity::Arc>(coord(data.basePoint),
                                               data.radious,
                                               data.staangle, data.endangle, data.isccw, layer, mf));

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

    auto lw = getLcLineWidth<lc::MetaLineWidthByValue>(data.lWeight);

    if (lw == nullptr) {
        lw = getLcLineWidth<lc::MetaLineWidthByValue>(DRW_LW_Conv::lineWidth::width00);
    }

    // If a layer starts with a * it's a special layer we don't process yet
    if (data.name.length()>0 && data.name.compare(0,1,"*")) {
        auto layer = std::make_shared<lc::Layer>(data.name, lw->width(), col->color());
        auto al = std::make_shared<lc::operation::AddLayer>(_document, layer);
        //_builder->appendMetaData(layer);
        al->execute();
    }
}

void DXFimpl::addSpline(const DRW_Spline* data) {
    if (_blockHandle != -1) {
        return;
    }

    auto layer = _document->layerByName(data->layer);
    if (layer==nullptr) {
        return;
    }
    std::shared_ptr<lc::MetaInfo> mf = getMetaInfo(*data);

    _builder->append(std::make_shared<lc::entity::Spline>(coords(data->controllist),
            data->knotslist,
            coords(data->fitlist),
            data->degree,
            false,
            data->tolfit,
            data->tgStart.x, data->tgStart.y, data->tgStart.z,
            data->tgEnd.x, data->tgEnd.y, data->tgEnd.z,
            data->normalVec.x, data->normalVec.y, data->normalVec.z, layer, mf));
}

void DXFimpl::addText(const DRW_Text& data) {
    if (_blockHandle != -1) {
        return;
    }

    auto layer = _document->layerByName(data.layer);
    if (layer==nullptr) {
        return;
    }
    std::shared_ptr<lc::MetaInfo> mf = getMetaInfo(data);

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

    auto layer = _document->layerByName(data.layer);
    if (layer==nullptr) {
        return;
    }
    std::shared_ptr<lc::MetaInfo> mf = getMetaInfo(data);

    _builder->append(std::make_shared<lc::entity::Point>(coord(data.basePoint), layer, mf));
}

void DXFimpl::addDimAlign(const DRW_DimAligned* data) {
    if (_blockHandle != -1) {
        return;
    }

    auto layer = _document->layerByName(data->layer);
    if (layer==nullptr) {
        return;
    }
    std::shared_ptr<lc::MetaInfo> mf = getMetaInfo(*data);

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

    auto layer = _document->layerByName(data->layer);
    if (layer==nullptr) {
        return;
    }
    std::shared_ptr<lc::MetaInfo> mf = getMetaInfo(*data);

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

    auto layer = _document->layerByName(data->layer);
    if (layer==nullptr) {
        return;
    }
    std::shared_ptr<lc::MetaInfo> mf = getMetaInfo(*data);

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

    auto layer = _document->layerByName(data->layer);
    if (layer==nullptr) {
        return;
    }
    std::shared_ptr<lc::MetaInfo> mf = getMetaInfo(*data);

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

    auto layer = _document->layerByName(data->layer);
    if (layer==nullptr) {
        return;
    }
    std::shared_ptr<lc::MetaInfo> mf = getMetaInfo(*data);

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

    auto layer = _document->layerByName(data.layer);
    if (layer==nullptr) {
        return;
    }
    std::shared_ptr<lc::MetaInfo> mf = getMetaInfo(data);

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

void DXFimpl::addMText(const DRW_MText& data) {
}

void DXFimpl::addHatch(const DRW_Hatch* data) {
}


lc::MetaInfo_SPtr DXFimpl::getMetaInfo(const DRW_Entity& data) const {
    std::shared_ptr<lc::MetaInfo> mf = nullptr;

    // Try to find a entities meta line weight
    auto lw = getLcLineWidth<lc::EntityMetaType>(data.lWeight);
    if (lw != nullptr) {
        if (mf == nullptr) {
            mf = lc::MetaInfo::create();
        }

        mf->add(lw);
    }

    // Try to find a entities meta color
    auto col = icol.intToColor(data.color);
    if (col != nullptr) {
        if (mf == nullptr) {
            mf = lc::MetaInfo::create();
        }

        mf->add(col);
    }

    // Most likely a lot of entities within a drawing will be 'BYLAYER' and with the CONTINUOUS linetype.
    // These are the default's for LibreCAD
    // One thing we need to solve is when entities within a block are loaded and use the BY_LAYER line type and styles
    // I will solve that during block importing.
    if (!(lc::StringHelper::cmpCaseInsensetive()(data.lineType, SKIP_BYLAYER) || lc::StringHelper::cmpCaseInsensetive()(data.lineType, SKIP_CONTINUOUS))) {
        if (mf == nullptr) {
            mf = lc::MetaInfo::create();
        }

        mf->add(_document->linePatternByName(data.lineType));
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

void DXFimpl::addLType(const DRW_LType& data) {
    std::make_shared<lc::operation::AddLinePattern>(_document, std::make_shared<lc::DxfLinePattern>(data.name, data.desc, data.path, data.length))->execute();
}


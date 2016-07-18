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

inline int DXFimpl::widthToInt(double wid) const {
    for (int i = 0; i < 24; i++) {
        if (_intToLineWidth[i]->width() == wid) {
            return i;
        }
    }

    return -1;
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

    // http://discourse.mcneel.com/t/creating-on-nurbscurve-from-control-points-and-knot-vector/12928/3
    auto knotList = data->knotslist;
    if (knotList.size()>=2) {
        knotList.erase(knotList.begin());
        knotList.pop_back();
    }

    _builder->append(std::make_shared<lc::entity::Spline>(coords(data->controllist),
            knotList,
            coords(data->fitlist),
            data->degree,
            false,
            data->tolfit,
            data->tgStart.x, data->tgStart.y, data->tgStart.z,
            data->tgEnd.x, data->tgEnd.y, data->tgEnd.z,
            data->normalVec.x, data->normalVec.y, data->normalVec.z, static_cast<lc::geo::Spline::splineflag>(data->flags), layer, mf));
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
    // Loop->objlist contains the 3 entities (copied) that define the hatch areas are the entities selected during hatch
    // loopList seems to contain the same entities, why??
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

/**
 * I am not sure the order of addImage and linkImage, if they can be swapped around or not
 * so if we see missing images, it could be that we first get calls to linImage and then to addImage
 * if linkImage isn't called as last, we miss a image during import
 */
void DXFimpl::addImage(const DRW_Image *data) {
    imageMapCache.emplace_back(*data);
}

void DXFimpl::linkImage(const DRW_ImageDef *data) {
    if (_blockHandle != -1) {
        return;
    }

    for( auto image = imageMapCache.cbegin(); image != imageMapCache.cend() /* not hoisted */; /* no increment */ ) {
        if (image->ref == data->handle) {
            auto layer = _document->layerByName(image->layer);
            if (layer == nullptr) {
                return;
            }

            std::shared_ptr<lc::MetaInfo> mf = getMetaInfo(*image);
            const lc::geo::Coordinate base(coord(image->basePoint));
            const lc::geo::Coordinate uv(coord(image->secPoint));
            const lc::geo::Coordinate vv(coord(image->vVector));

            _builder->append(
                    std::make_shared<lc::entity::Image>(data->name, base, uv, vv, image->sizeu, image->sizev,
                                                        image->brightness, image->contrast, image->fade, layer, mf));
            image = imageMapCache.erase( image ) ; // advances iter
        } else {
            image++;
        }
    }
}


/*********************************************
 * Write DXF Implementation BELOW
 *********************************************/

void DXFimpl::writeLayers() {
    auto layers = _document->allLayers();
    for(const auto &layer: layers) {
        writeLayer(layer.second);
    }
}

void DXFimpl::writeLayer(const std::shared_ptr<const lc::Layer> layer) {
    DRW_Layer lay;
    lc::iColor icol_inst;

    auto col = layer->color();
    lay.name = layer->name();
    lay.color = icol_inst.colorToInt(col);
    auto wid = layer->lineWidth().width();
    std::cout << wid;
//    auto val = widthToInt();

//    lay.lWeight = static_cast<DRW_LW_Conv::lineWidth>();
    lay.flags = layer->isFrozen() ? 0x01 : 0x00;

    dxfW->writeLayer(&lay);
}

bool DXFimpl::writeDXF(std::string& filename, DXF::version type) {
    dxfW = new dxfRW(filename.c_str());

    //Default setting
    DRW::Version exportVersion;

    switch(type) {
    case DXF::version::R12:
        exportVersion = DRW::AC1009;
        break;
    case DXF::version::R14:
        exportVersion = DRW::AC1014;
        break;
    case DXF::version::R2000:
        exportVersion = DRW::AC1015;
        break;
    case DXF::version::R2004:
        exportVersion = DRW::AC1018;
        break;
    case DXF::version::R2007:
        exportVersion = DRW::AC1021;
        break;
    case DXF::version::R2010:
        exportVersion = DRW::AC1024;
        break;
    case DXF::version::R2013:
        exportVersion = DRW::AC1027;
        break;
    default:
        exportVersion = DRW::AC1024;
        break;
    }

    bool success = dxfW->write(this, exportVersion, false); //ascii
    delete dxfW;

    if (!success) {
        std::cout << "can't write file";
        return false;
    }
}

void DXFimpl::writePoint(const lc::entity::Point_CSPtr p) {
    DRW_Point point;
    getEntityAttributes(&point, p);
    point.basePoint.x = p->x();
    point.basePoint.y = p->y();
    dxfW->writePoint(&point);
}

void DXFimpl::writeLine(const lc::entity::Line_CSPtr l) {
    DRW_Line line;
    getEntityAttributes(&line, l);
    line.basePoint.x = l->start().x();
    line.basePoint.y = l->start().y();
    line.secPoint.x = l->end().x();
    line.secPoint.y = l->end().y();
    dxfW->writeLine(&line);
}

void DXFimpl::writeCircle(const lc::entity::Circle_CSPtr c) {
    DRW_Circle circle;
    getEntityAttributes(&circle, c);
    circle.basePoint.x = c->center().x();
    circle.basePoint.y = c->center().y();
    circle.radious = c->radius();
    dxfW->writeCircle(&circle);
}

void DXFimpl::writeArc(const lc::entity::Arc_CSPtr a) {
    DRW_Arc arc;
    getEntityAttributes(&arc, a);
    arc.basePoint.x = a->center().x();
    arc.basePoint.y = a->center().y();
    arc.radious = a->radius();
    if (a->CCW()) {
        arc.staangle = a->startAngle();
        arc.endangle = a->endAngle();
    } else {
        arc.staangle = a->endAngle();
        arc.endangle = a->startAngle();
    }
    dxfW->writeArc(&arc);
}

void DXFimpl::writeEllipse(const lc::entity::Ellipse_CSPtr s) {
    DRW_Ellipse el;
    getEntityAttributes(&el, s);
    el.basePoint.x = s->center().x();
    el.basePoint.y = s->center().y();
    el.secPoint.x = s->majorP().x();
    el.secPoint.y = s->majorP().y();
    el.ratio = s->ratio();
    if (s->isReversed()) {
        el.staparam = s->startAngle();
        el.endparam = s->endAngle();
    } else {
        el.staparam = s->endAngle();
        el.endparam = s->startAngle();
    }
    dxfW->writeEllipse(&el);
}

void DXFimpl::getEntityAttributes(DRW_Entity *ent, lc::entity::CADEntity_CSPtr entity) {
    auto layer_  = entity->layer();
    auto metaPen_ = entity->metaInfo<lc::DxfLinePattern>(lc::DxfLinePattern::LCMETANAME());
    auto metaWidth_ = entity->metaInfo<lc::MetaLineWidthByValue>(lc::MetaLineWidthByValue::LCMETANAME());
    auto metaColor_ = entity->metaInfo<lc::MetaColor>(lc::MetaColor::LCMETANAME());

    ent->layer = layer_->name();

    if(metaColor_!=nullptr) {
        lc::iColor col;
        auto color_ = col.colorToInt(metaColor_);
        ent->color = color_;
    }
    if(metaPen_!=nullptr) {
        ent->lineType = metaPen_->name();
    }
    if(metaWidth_!=nullptr) {
        ent->lWeight = static_cast<DRW_LW_Conv::lineWidth>(widthToInt(metaWidth_->width()));
    }
}

void DXFimpl::writeLTypes() {
    DRW_LType ltype;
    // Standard linetypes for LibreCAD / AutoCAD
    ltype.name = "CONTINUOUS";
    ltype.desc = "Solid line";
    dxfW->writeLineType(&ltype);
    ltype.name = "ByLayer";
    dxfW->writeLineType(&ltype);
    ltype.name = "ByBlock";
    dxfW->writeLineType(&ltype);

    ltype.name = "DOT";
    ltype.desc = "Dot . . . . . . . . . . . . . . . . . . . . . .";
    ltype.size = 2;
    ltype.length = 6.35;
    ltype.path.push_back(0.0);
    ltype.path.push_back(-6.35);
    dxfW->writeLineType(&ltype);

    ltype.path.clear();
    ltype.name = "DOTTINY";
    ltype.desc = "Dot (.15x) .....................................";
    ltype.size = 2;
    ltype.length = 0.9525;
    ltype.path.push_back(0.0);
    ltype.path.push_back(-0.9525);
    dxfW->writeLineType(&ltype);

    ltype.path.clear();
    ltype.name = "DOT2";
    ltype.desc = "Dot (.5x) .....................................";
    ltype.size = 2;
    ltype.length = 3.175;
    ltype.path.push_back(0.0);
    ltype.path.push_back(-3.175);
    dxfW->writeLineType(&ltype);

    ltype.path.clear();
    ltype.name = "DOTX2";
    ltype.desc = "Dot (2x) .  .  .  .  .  .  .  .  .  .  .  .  .";
    ltype.size = 2;
    ltype.length = 12.7;
    ltype.path.push_back(0.0);
    ltype.path.push_back(-12.7);
    dxfW->writeLineType(&ltype);

    ltype.path.clear();
    ltype.name = "DASHED";
    ltype.desc = "Dashed _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _";
    ltype.size = 2;
    ltype.length = 19.05;
    ltype.path.push_back(12.7);
    ltype.path.push_back(-6.35);
    dxfW->writeLineType(&ltype);

    ltype.path.clear();
    ltype.name = "DASHEDTINY";
    ltype.desc = "Dashed (.15x) _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _";
    ltype.size = 2;
    ltype.length = 2.8575;
    ltype.path.push_back(1.905);
    ltype.path.push_back(-0.9525);
    dxfW->writeLineType(&ltype);

    ltype.path.clear();
    ltype.name = "DASHED2";
    ltype.desc = "Dashed (.5x) _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _";
    ltype.size = 2;
    ltype.length = 9.525;
    ltype.path.push_back(6.35);
    ltype.path.push_back(-3.175);
    dxfW->writeLineType(&ltype);

    ltype.path.clear();
    ltype.name = "DASHEDX2";
    ltype.desc = "Dashed (2x) ____  ____  ____  ____  ____  ___";
    ltype.size = 2;
    ltype.length = 38.1;
    ltype.path.push_back(25.4);
    ltype.path.push_back(-12.7);
    dxfW->writeLineType(&ltype);

    ltype.path.clear();
    ltype.name = "DASHDOT";
    ltype.desc = "Dash dot __ . __ . __ . __ . __ . __ . __ . __";
    ltype.size = 4;
    ltype.length = 25.4;
    ltype.path.push_back(12.7);
    ltype.path.push_back(-6.35);
    ltype.path.push_back(0.0);
    ltype.path.push_back(-6.35);
    dxfW->writeLineType(&ltype);

    ltype.path.clear();
    ltype.name = "DASHDOTTINY";
    ltype.desc = "Dash dot (.15x) _._._._._._._._._._._._._._._.";
    ltype.size = 4;
    ltype.length = 3.81;
    ltype.path.push_back(1.905);
    ltype.path.push_back(-0.9525);
    ltype.path.push_back(0.0);
    ltype.path.push_back(-0.9525);
    dxfW->writeLineType(&ltype);

    ltype.path.clear();
    ltype.name = "DASHDOT2";
    ltype.desc = "Dash dot (.5x) _._._._._._._._._._._._._._._.";
    ltype.size = 4;
    ltype.length = 12.7;
    ltype.path.push_back(6.35);
    ltype.path.push_back(-3.175);
    ltype.path.push_back(0.0);
    ltype.path.push_back(-3.175);
    dxfW->writeLineType(&ltype);

    ltype.path.clear();
    ltype.name = "DASHDOTX2";
    ltype.desc = "Dash dot (2x) ____  .  ____  .  ____  .  ___";
    ltype.size = 4;
    ltype.length = 50.8;
    ltype.path.push_back(25.4);
    ltype.path.push_back(-12.7);
    ltype.path.push_back(0.0);
    ltype.path.push_back(-12.7);
    dxfW->writeLineType(&ltype);

    ltype.path.clear();
    ltype.name = "DIVIDE";
    ltype.desc = "Divide ____ . . ____ . . ____ . . ____ . . ____";
    ltype.size = 6;
    ltype.length = 31.75;
    ltype.path.push_back(12.7);
    ltype.path.push_back(-6.35);
    ltype.path.push_back(0.0);
    ltype.path.push_back(-6.35);
    ltype.path.push_back(0.0);
    ltype.path.push_back(-6.35);
    dxfW->writeLineType(&ltype);

    ltype.path.clear();
    ltype.name = "DIVIDETINY";
    ltype.desc = "Divide (.15x) __..__..__..__..__..__..__..__.._";
    ltype.size = 6;
    ltype.length = 4.7625;
    ltype.path.push_back(1.905);
    ltype.path.push_back(-0.9525);
    ltype.path.push_back(0.0);
    ltype.path.push_back(-0.9525);
    ltype.path.push_back(0.0);
    ltype.path.push_back(-0.9525);
    dxfW->writeLineType(&ltype);

    ltype.path.clear();
    ltype.name = "DIVIDE2";
    ltype.desc = "Divide (.5x) __..__..__..__..__..__..__..__.._";
    ltype.size = 6;
    ltype.length = 15.875;
    ltype.path.push_back(6.35);
    ltype.path.push_back(-3.175);
    ltype.path.push_back(0.0);
    ltype.path.push_back(-3.175);
    ltype.path.push_back(0.0);
    ltype.path.push_back(-3.175);
    dxfW->writeLineType(&ltype);

    ltype.path.clear();
    ltype.name = "DIVIDEX2";
    ltype.desc = "Divide (2x) ________  .  .  ________  .  .  _";
    ltype.size = 6;
    ltype.length = 63.5;
    ltype.path.push_back(25.4);
    ltype.path.push_back(-12.7);
    ltype.path.push_back(0.0);
    ltype.path.push_back(-12.7);
    ltype.path.push_back(0.0);
    ltype.path.push_back(-12.7);
    dxfW->writeLineType(&ltype);

    ltype.path.clear();
    ltype.name = "BORDER";
    ltype.desc = "Border __ __ . __ __ . __ __ . __ __ . __ __ .";
    ltype.size = 6;
    ltype.length = 44.45;
    ltype.path.push_back(12.7);
    ltype.path.push_back(-6.35);
    ltype.path.push_back(12.7);
    ltype.path.push_back(-6.35);
    ltype.path.push_back(0.0);
    ltype.path.push_back(-6.35);
    dxfW->writeLineType(&ltype);

    ltype.path.clear();
    ltype.name = "BORDERTINY";
    ltype.desc = "Border (.15x) __.__.__.__.__.__.__.__.__.__.__.";
    ltype.size = 6;
    ltype.length = 6.6675;
    ltype.path.push_back(1.905);
    ltype.path.push_back(-0.9525);
    ltype.path.push_back(1.905);
    ltype.path.push_back(-0.9525);
    ltype.path.push_back(0.0);
    ltype.path.push_back(-0.9525);
    dxfW->writeLineType(&ltype);

    ltype.path.clear();
    ltype.name = "BORDER2";
    ltype.desc = "Border (.5x) __.__.__.__.__.__.__.__.__.__.__.";
    ltype.size = 6;
    ltype.length = 22.225;
    ltype.path.push_back(6.35);
    ltype.path.push_back(-3.175);
    ltype.path.push_back(6.35);
    ltype.path.push_back(-3.175);
    ltype.path.push_back(0.0);
    ltype.path.push_back(-3.175);
    dxfW->writeLineType(&ltype);

    ltype.path.clear();
    ltype.name = "BORDERX2";
    ltype.desc = "Border (2x) ____  ____  .  ____  ____  .  ___";
    ltype.size = 6;
    ltype.length = 88.9;
    ltype.path.push_back(25.4);
    ltype.path.push_back(-12.7);
    ltype.path.push_back(25.4);
    ltype.path.push_back(-12.7);
    ltype.path.push_back(0.0);
    ltype.path.push_back(-12.7);
    dxfW->writeLineType(&ltype);

    ltype.path.clear();
    ltype.name = "CENTER";
    ltype.desc = "Center ____ _ ____ _ ____ _ ____ _ ____ _ ____";
    ltype.size = 4;
    ltype.length = 50.8;
    ltype.path.push_back(31.75);
    ltype.path.push_back(-6.35);
    ltype.path.push_back(6.35);
    ltype.path.push_back(-6.35);
    dxfW->writeLineType(&ltype);

    ltype.path.clear();
    ltype.name = "CENTERTINY";
    ltype.desc = "Center (.15x) ___ _ ___ _ ___ _ ___ _ ___ _ ___";
    ltype.size = 4;
    ltype.length = 7.62;
    ltype.path.push_back(4.7625);
    ltype.path.push_back(-0.9525);
    ltype.path.push_back(0.9525);
    ltype.path.push_back(-0.9525);
    dxfW->writeLineType(&ltype);

    ltype.path.clear();
    ltype.name = "CENTER2";
    ltype.desc = "Center (.5x) ___ _ ___ _ ___ _ ___ _ ___ _ ___";
    ltype.size = 4;
    ltype.length = 28.575;
    ltype.path.push_back(19.05);
    ltype.path.push_back(-3.175);
    ltype.path.push_back(3.175);
    ltype.path.push_back(-3.175);
    dxfW->writeLineType(&ltype);

    ltype.path.clear();
    ltype.name = "CENTERX2";
    ltype.desc = "Center (2x) ________  __  ________  __  _____";
    ltype.size = 4;
    ltype.length = 101.6;
    ltype.path.push_back(63.5);
    ltype.path.push_back(-12.7);
    ltype.path.push_back(12.7);
    ltype.path.push_back(-12.7);
    dxfW->writeLineType(&ltype);
}

void DXFimpl::writeAppId(){
    DRW_AppId ai;
    ai.name ="LibreCad";
    dxfW->writeAppId(&ai);
}

void DXFimpl::writeDimension(const lc::entity::Dimension_CSPtr d) {

}

void DXFimpl::writeLWPolyline(const lc::entity::LWPolyline_CSPtr p) {

}

void DXFimpl::writeImage(const lc::entity::Image_CSPtr i) {

}

void DXFimpl::writeText(const lc::entity::Text_CSPtr t) {

}

void DXFimpl::writeEntities(){
    for(const auto e :_document->entityContainer().asVector()) {
        writeEntity(e);
    }
}

void DXFimpl::writeEntity(lc::entity::CADEntity_CSPtr entity) {
    const auto Line = std::dynamic_pointer_cast<const lc::entity::Line>(entity);
    if (Line != nullptr) {
        writeLine(Line);
        return;
    }

    const auto Circle = std::dynamic_pointer_cast<const lc::entity::Circle>(entity);
    if (Circle != nullptr) {
        writeCircle(Circle);
        return;
    }

    const auto Arc = std::dynamic_pointer_cast<const lc::entity::Arc>(entity);
    if (Arc != nullptr) {
        writeArc(Arc);
        return;
    }

    const auto Ellipse = std::dynamic_pointer_cast<const lc::entity::Ellipse>(entity);
    if (Ellipse != nullptr) {
        writeEllipse(Ellipse);
        return;
    }

    const auto Image = std::dynamic_pointer_cast<const lc::entity::Image>(entity);
    if (Image != nullptr) {
        writeImage(Image);
        return;
    }

    const auto LWPolyline = std::dynamic_pointer_cast<const lc::entity::LWPolyline>(entity);
    if (LWPolyline != nullptr) {
        writeLWPolyline(LWPolyline);
        return;
    }

    const auto Text = std::dynamic_pointer_cast<const lc::entity::Text>(entity);
    if (Text != nullptr) {
        writeText(Text);
        return;
    }

    const auto ellipse = std::dynamic_pointer_cast<const lc::entity::Ellipse>(entity);
    if (ellipse != nullptr) {
        writeEllipse(ellipse);
        return;
    }

    // const auto ellipse = std::dynamic_pointer_cast<const lc::entity::Ellipse>(entity);
    // if (ellipse != nullptr) {
    //     writeEllipse(ellipse);
    //     return;
    // }
}


/*****************************************
 * EXTRA Utilites
 *****************************************/
lc::AngleFormat DXFimpl::numberToAngleFormat(int num) {

    lc::AngleFormat af;

    switch (num) {
    default:
    case 0:
        af = lc::AngleFormat::DegreesDecimal;
        break;
    case 1:
        af = lc::AngleFormat::DegreesMinutesSeconds;
        break;
    case 2:
        af = lc::AngleFormat::Gradians;
        break;
    case 3:
        af = lc::AngleFormat::Radians;
        break;
    case 4:
        af = lc::AngleFormat::Surveyors;
        break;
    }

    return af;
}


/**
 * Converts AngleFormat enum to DXF number.
 */
int DXFimpl::angleFormatToNumber(lc::AngleFormat af) {

    int num;

    switch (af) {
    default:
    case lc::AngleFormat::DegreesDecimal:
        num = 0;
        break;
    case lc::AngleFormat::DegreesMinutesSeconds:
        num = 1;
        break;
    case lc::AngleFormat::Gradians:
        num = 2;
        break;
    case lc::AngleFormat::Radians:
        num = 3;
        break;
    case lc::AngleFormat::Surveyors:
        num = 4;
        break;
    }

    return num;
}



/**
 * converts a DXF units setting (e.g. INSUNITSs) to a units enum.
 */
lc::Units DXFimpl::numberToUnit(int num) {
    switch (num) {
    default:
    case  0:
        return lc::Units::None;
        break;
    case  1:
        return lc::Units::Inch;
        break;
    case  2:
        return lc::Units::Foot;
        break;
    case  3:
        return lc::Units::Mile;
        break;
    case  4:
        return lc::Units::Millimeter;
        break;
    case  5:
        return lc::Units::Centimeter;
        break;
    case  6:
        return lc::Units::Meter;
        break;
    case  7:
        return lc::Units::Kilometer;
        break;
    case  8:
        return lc::Units::Microinch;
        break;
    case  9:
        return lc::Units::Mil;
        break;
    case 10:
        return lc::Units::Yard;
        break;
    case 11:
        return lc::Units::Angstrom;
        break;
    case 12:
        return lc::Units::Nanometer;
        break;
    case 13:
        return lc::Units::Micron;
        break;
    case 14:
        return lc::Units::Decimeter;
        break;
    case 15:
        return lc::Units::Decameter;
        break;
    case 16:
        return lc::Units::Hectometer;
        break;
    case 17:
        return lc::Units::Gigameter;
        break;
    case 18:
        return lc::Units::Astro;
        break;
    case 19:
        return lc::Units::Lightyear;
        break;
    case 20:
        return lc::Units::Parsec;
        break;
    }

    return lc::Units::None;
}



/**
 * Converst a units enum into a DXF units number e.g. for INSUNITSs.
 */
int DXFimpl::unitToNumber(lc::Units unit) {
    switch (unit) {
    default:
    case lc::Units::None:
        return  0;
        break;
    case lc::Units::Inch:
        return  1;
        break;
    case lc::Units::Foot:
        return  2;
        break;
    case lc::Units::Mile:
        return  3;
        break;
    case lc::Units::Millimeter:
        return  4;
        break;
    case lc::Units::Centimeter:
        return  5;
        break;
    case lc::Units::Meter:
        return  6;
        break;
    case lc::Units::Kilometer:
        return  7;
        break;
    case lc::Units::Microinch:
        return  8;
        break;
    case lc::Units::Mil:
        return  9;
        break;
    case lc::Units::Yard:
        return 10;
        break;
    case lc::Units::Angstrom:
        return 11;
        break;
    case lc::Units::Nanometer:
        return 12;
        break;
    case lc::Units::Micron:
        return 13;
        break;
    case lc::Units::Decimeter:
        return 14;
        break;
    case lc::Units::Decameter:
        return 15;
        break;
    case lc::Units::Hectometer:
        return 16;
        break;
    case lc::Units::Gigameter:
        return 17;
        break;
    case lc::Units::Astro:
        return 18;
        break;
    case lc::Units::Lightyear:
        return 19;
        break;
    case lc::Units::Parsec:
        return 20;
        break;
    }

    return 0;
}

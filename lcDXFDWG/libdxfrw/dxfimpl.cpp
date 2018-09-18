#include "dxfimpl.h"
#include "../generic/helpers.h"

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
#include <cad/operations/entitybuilder.h>
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
#include <cad/meta/block.h>
#include <cad/primitive/insert.h>
#include <cad/operations/blockops.h>
#include <cad/meta/customentitystorage.h>

DXFimpl::DXFimpl(std::shared_ptr<lc::Document> document, lc::operation::Builder_SPtr builder) : 
        _document(document), 
        _builder(std::move(builder)),
        _entityBuilder(std::make_shared<lc::operation::EntityBuilder>(document)),
        _currentBlock(nullptr),
        dxfW(nullptr) {
    _builder->append(_entityBuilder);
}

inline int DXFimpl::widthToInt(double wid) const {
    for (int i = 0; i < 24; i++) {
        if (lc::FileHelpers::intToLW(i).width() == wid) {
            return i;
        }
    }

    return -1;
}

void DXFimpl::setBlock(const int handle) {
    std::cout << "setBlock " << handle << "\n";
}

void DXFimpl::addBlock(const DRW_Block& data) {
    _currentBlock = nullptr;

    auto base = coord(data.basePoint);

    auto appData = data.appData;
    auto it = appData.begin();
    std::string appName;

    while(it != appData.end()) {
        if(!it->empty()) {
            appName = *(it->begin()->content.s);

            if(appName == APP_NAME) {
                break;
            }
        }

        it++;
    }

    if(it != appData.end() && it->size() >= 3) {
        auto it2 = it->begin();

        it2++;
        auto pluginName = *(it2->content.s);

        it2++;
        auto entityName = *(it2->content.s);

        it2++;

        std::map<std::string, std::string> params;
        while(it2 != it->end()) {
            auto key = *(it2->content.s);

            it2++;

            if(it2 == it->end()) {
                break;
            }

            auto value = *(it2->content.s);
            params[key] = value;
            it2++;
        }

        _currentBlock = std::make_shared<lc::CustomEntityStorage>(pluginName, entityName, base, params);
    }

    if(_currentBlock == nullptr) {
        _currentBlock = std::make_shared<lc::Block>(data.name, base);
    }
    _builder->append(std::make_shared<lc::operation::AddBlock>(_document, _currentBlock));

    _blocks.insert(std::pair<std::string, lc::Block_CSPtr>(data.name, _currentBlock));
}

void DXFimpl::endBlock() {
    _currentBlock = nullptr;
}

void DXFimpl::addLine(const DRW_Line& data) {
    lc::builder::LineBuilder builder;

    builder.setMetaInfo(getMetaInfo(data));
    builder.setBlock(_currentBlock);
    builder.setLayer(_document->layerByName(data.layer));
    builder.setStart(coord(data.basePoint));
    builder.setEnd(coord(data.secPoint));

    _entityBuilder->appendEntity(builder.build());
}

void DXFimpl::addCircle(const DRW_Circle& data) {
    lc::builder::CircleBuilder builder;

    builder.setMetaInfo(getMetaInfo(data));
    builder.setLayer(_document->layerByName(data.layer));
    builder.setCenter(coord(data.basePoint));
    builder.setRadius(data.radious);
    builder.setBlock(_currentBlock);

    _entityBuilder->appendEntity(builder.build());
}

void DXFimpl::addArc(const DRW_Arc& data) {
    lc::builder::ArcBuilder builder;

    builder.setMetaInfo(getMetaInfo(data));
    builder.setLayer(_document->layerByName(data.layer));
    builder.setBlock(_currentBlock);
    builder.setCenter(coord(data.basePoint));
    builder.setRadius(data.radious);
    builder.setStartAngle(data.staangle);
    builder.setEndAngle(data.endangle);
    builder.setIsCCW((bool) data.isccw);

    _entityBuilder->appendEntity(builder.build());
}

void DXFimpl::addEllipse(const DRW_Ellipse& data) {
    std::shared_ptr<lc::MetaInfo> mf = getMetaInfo(data);
    auto layer = _document->layerByName(data.layer);

    auto secPoint = coord(data.secPoint);
    auto lcEllipse = std::make_shared<lc::entity::Ellipse>(coord(data.basePoint),
                                                           secPoint,
                                                           secPoint.magnitude() * data.ratio,
                                                           data.staparam,
                                                           data.endparam,
                                                           data.isccw,
                                                           layer,
                                                           mf,
                                                           _currentBlock
    );

    _entityBuilder->appendEntity(lcEllipse);
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

    auto lp = _document->linePatternByName(data.lineType);
    auto isFrozen = (bool) (data.flags & 1u);

    auto layer = std::make_shared<lc::Layer>(data.name, lw->width(), col->color(), lp, isFrozen);
    // If a layer starts with a * it's a special layer we don't process yet
    if(data.name == "0") {
        auto al = std::make_shared<lc::operation::ReplaceLayer>(_document, _document->layerByName("0"), layer);
        _builder->append(al);
    }
    else if (data.name.length() > 0 && (data.name.compare(0,1,"*") != 0)) {
        auto al = std::make_shared<lc::operation::AddLayer>(_document, layer);
        _builder->append(al);
    }
}

void DXFimpl::addSpline(const DRW_Spline* data) {
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
    auto lcSpline = std::make_shared<lc::entity::Spline>(coords(data->controllist),
                                                         knotList,
                                                         coords(data->fitlist),
                                                         data->degree,
                                                         false,
                                                         data->tolfit,
                                                         data->tgStart.x, data->tgStart.y, data->tgStart.z,
                                                         data->tgEnd.x, data->tgEnd.y, data->tgEnd.z,
                                                         data->normalVec.x, data->normalVec.y, data->normalVec.z,
                                                         static_cast<lc::geo::Spline::splineflag>(data->flags),
                                                         layer,
                                                         mf,
                                                         _currentBlock
    );

    _entityBuilder->appendEntity(lcSpline);
}

void DXFimpl::addText(const DRW_Text& data) {
    auto layer = _document->layerByName(data.layer);
    if (layer==nullptr) {
        return;
    }
    std::shared_ptr<lc::MetaInfo> mf = getMetaInfo(data);
    auto lcText = std::make_shared<lc::entity::Text>(coord(data.basePoint),
                                                     data.text, data.height,
                                                     data.angle, data.style,
                                                     lc::TextConst::DrawingDirection(data.textgen),
                                                     lc::TextConst::HAlign(data.alignH),
                                                     lc::TextConst::VAlign(data.alignV),
                                                     layer,
                                                     mf,
                                                     _currentBlock
    );

    _entityBuilder->appendEntity(lcText);
}

void DXFimpl::addPoint(const DRW_Point& data) {
    auto layer = _document->layerByName(data.layer);
    if (layer==nullptr) {
        return;
    }
    std::shared_ptr<lc::MetaInfo> mf = getMetaInfo(data);
    auto lcPoint = std::make_shared<lc::entity::Point>(coord(data.basePoint),
                                                       layer,
                                                       mf,
                                                       _currentBlock
    );

    _entityBuilder->appendEntity(lcPoint);
}

void DXFimpl::addDimAlign(const DRW_DimAligned* data) {
    auto layer = _document->layerByName(data->layer);
    if (layer==nullptr) {
        return;
    }
    std::shared_ptr<lc::MetaInfo> mf = getMetaInfo(*data);
    auto lcDimAligned = std::make_shared<lc::entity::DimAligned>(
            coord(data->getDefPoint()),
            coord(data->getTextPoint()),
            static_cast<lc::TextConst::AttachmentPoint>(data->getAlign()),
            data->getDir(),
            data->getTextLineFactor(),
            static_cast<lc::TextConst::LineSpacingStyle>(data->getTextLineStyle()),
            data->getText(),
            coord(data->getDef1Point()),
            coord(data->getDef2Point()),
            layer,
            mf,
            _currentBlock
    );

    _entityBuilder->appendEntity(lcDimAligned);
}

void DXFimpl::addDimLinear(const DRW_DimLinear* data) {
    auto layer = _document->layerByName(data->layer);
    if (layer==nullptr) {
        return;
    }
    std::shared_ptr<lc::MetaInfo> mf = getMetaInfo(*data);
    auto lcDimLinear = std::make_shared<lc::entity::DimLinear>(
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
            layer,
            mf,
            _currentBlock
    );

    _entityBuilder->appendEntity(lcDimLinear);
}

void DXFimpl::addDimRadial(const DRW_DimRadial* data) {
    auto layer = _document->layerByName(data->layer);
    if (layer==nullptr) {
        return;
    }
    std::shared_ptr<lc::MetaInfo> mf = getMetaInfo(*data);
    auto  lcDimRadial = std::make_shared<lc::entity::DimRadial>(
             coord(data->getCenterPoint()),
             coord(data->getTextPoint()),
             static_cast<lc::TextConst::AttachmentPoint>(data->getAlign()),
             data->getDir(),
             data->getTextLineFactor(),
             static_cast<lc::TextConst::LineSpacingStyle>(data->getTextLineStyle()),
             data->getText(),
             coord(data->getDiameterPoint()),
             data->getLeaderLength(),
             layer,
             mf,
             _currentBlock
    );

    _entityBuilder->appendEntity(lcDimRadial);
}

void DXFimpl::addDimDiametric(const DRW_DimDiametric* data) {
    auto layer = _document->layerByName(data->layer);
    if (layer==nullptr) {
        return;
    }
    std::shared_ptr<lc::MetaInfo> mf = getMetaInfo(*data);
    auto lcDimDiametric = std::make_shared<lc::entity::DimDiametric>(
             coord(data->getDiameter1Point()),
             coord(data->getTextPoint()),
             static_cast<lc::TextConst::AttachmentPoint>(data->getAlign()),
             data->getDir(),
             data->getTextLineFactor(),
             static_cast<lc::TextConst::LineSpacingStyle>(data->getTextLineStyle()),
             data->getText(),
             coord(data->getDiameter2Point()),
             data->getLeaderLength(),
             layer,
             mf,
             _currentBlock
    );

    _entityBuilder->appendEntity(lcDimDiametric);
}

void DXFimpl::addDimAngular(const DRW_DimAngular* data) {
    auto layer = _document->layerByName(data->layer);
    if (layer==nullptr) {
        return;
    }
    std::shared_ptr<lc::MetaInfo> mf = getMetaInfo(*data);
    auto lcDimAngular = std::make_shared<lc::entity::DimAngular>(
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
             layer,
             mf,
             _currentBlock
    );

    _entityBuilder->appendEntity(lcDimAngular);
}

void DXFimpl::addDimAngular3P(const DRW_DimAngular3p* data) {
    if (_currentBlock == nullptr) {
    }
}

void DXFimpl::addDimOrdinate(const DRW_DimOrdinate* data) {
    if (_currentBlock == nullptr) {
    }
}

void DXFimpl::addLWPolyline(const DRW_LWPolyline& data) {
    auto layer = _document->layerByName(data.layer);
    if (layer==nullptr) {
        return;
    }
    std::shared_ptr<lc::MetaInfo> mf = getMetaInfo(data);

    std::vector<lc::entity::LWVertex2D> points;
    for (const auto& i : data.vertlist) {
        points.emplace_back(lc::geo::Coordinate(i->x, i->y), i->bulge, i->stawidth, i->endwidth);
    }

    auto isCLosed = data.flags & 0x01u;
    auto lcLWPolyline = std::make_shared<lc::entity::LWPolyline>(
            points,
            data.width,
            data.elevation,
            data.thickness,
            isCLosed,
            coord(data.extPoint),
            layer,
            mf,
            _currentBlock
    );

    _entityBuilder->appendEntity(lcLWPolyline);
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
    auto lw = getLcLineWidth<lc::MetaLineWidth>(data.lWeight);
    if (lw != nullptr) {
        if (mf == nullptr) {
            mf = lc::MetaInfo::create();
        }

        mf->add(lw);
    }

    // Try to find a entities meta color
    lc::MetaColor_CSPtr col;
    if(data.color == BYBLOCK_COLOR) {
        col = std::make_shared<const lc::MetaColorByBlock>();
    }
    else {
        col = icol.intToColor(data.color);
    }

    if (col != nullptr) {
        if (mf == nullptr) {
            mf = lc::MetaInfo::create();
        }

        mf->add(col);
    }

    // Most likely a lot of entities within a drawing will be 'BYLAYER' and with the CONTINUOUS linetype.
    // These are the default's for LibreCAD
    lc::DxfLinePattern_CSPtr linePattern = nullptr;
    if(data.lineType == LTYPE_BYBLOCK) {
        linePattern = std::make_shared<lc::DxfLinePatternByBlock>();
    }
    else if (!(lc::StringHelper::cmpCaseInsensetive()(data.lineType, SKIP_BYLAYER) || lc::StringHelper::cmpCaseInsensetive()(data.lineType, SKIP_CONTINUOUS))) {
        linePattern = _document->linePatternByName(data.lineType);
    }

    if(linePattern != nullptr) {
        if (mf == nullptr) {
            mf = lc::MetaInfo::create();
        }

        mf->add(linePattern);
    }


    return mf;
}

lc::geo::Coordinate DXFimpl::coord(DRW_Coord const& coord) const {
    return { coord.x, coord.y, coord.z };
}

std::vector<lc::geo::Coordinate> DXFimpl::coords(std::vector<DRW_Coord *> coordList) const {
    std::vector<lc::geo::Coordinate> coords;
    coords.reserve(coordList.size());
    for (const DRW_Coord* ptr : coordList) {
        coords.emplace_back(ptr->x,ptr->y, ptr->z);
    }
    return coords;
}

void DXFimpl::addLType(const DRW_LType& data) {
    std::make_shared<lc::operation::AddLinePattern>(_document, std::make_shared<lc::DxfLinePatternByValue>(data.name, data.desc, data.path, data.length))->execute();
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
    for(auto image = imageMapCache.cbegin(); image != imageMapCache.cend() /* not hoisted */; /* no increment */ ) {
        if (image->ref == data->handle) {
            auto layer = _document->layerByName(image->layer);
            if (layer == nullptr) {
                return;
            }

            std::shared_ptr<lc::MetaInfo> mf = getMetaInfo(*image);
            const lc::geo::Coordinate base(coord(image->basePoint));
            const lc::geo::Coordinate uv(coord(image->secPoint));
            const lc::geo::Coordinate vv(coord(image->vVector));

            auto lcImage = std::make_shared<lc::entity::Image>(
                    data->name,
                    base, uv, vv,
                    image->sizeu, image->sizev,
                    image->brightness, image->contrast, image->fade,
                    layer,
                    mf,
                    _currentBlock
            );
            _entityBuilder->appendEntity(lcImage);

            image = imageMapCache.erase( image ) ; // advances iter
        } else {
            image++;
        }
    }
}

void DXFimpl::addInsert(const DRW_Insert& data) {
    lc::builder::InsertBuilder builder;
    builder.setMetaInfo(getMetaInfo(data));
    builder.setBlock(_currentBlock);
    builder.setLayer(_document->layerByName(data.layer));
    builder.setCoordinate(coord(data.basePoint));
    builder.setDisplayBlock(_blocks[data.name]);
    builder.setDocument(_document);

    _entityBuilder->appendEntity(builder.build());
}

/*********************************************
 * Write DXF Implementation BELOW
 *********************************************/

void DXFimpl::writeLayers() {
    auto layers = _document->allLayers();
    for(const auto& layer: layers) {
        writeLayer(layer.second);
    }
}

void DXFimpl::writeLayer(const std::shared_ptr<const lc::Layer>& layer) {
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

bool DXFimpl::writeDXF(const std::string& filename, lc::File::Type type) {
    dxfW = new dxfRW(filename.c_str());

    //Default setting
    DRW::Version exportVersion;

    switch(type) {
        case lc::File::LIBDXFRW_DXF_R12:
        case lc::File::LIBDXFRW_DXB_R12:
            exportVersion = DRW::AC1009;
            break;
        case lc::File::LIBDXFRW_DXF_R14:
        case lc::File::LIBDXFRW_DXB_R14:
            exportVersion = DRW::AC1014;
            break;
        case lc::File::LIBDXFRW_DXF_R2000:
        case lc::File::LIBDXFRW_DXB_R2000:
            exportVersion = DRW::AC1015;
            break;
        case lc::File::LIBDXFRW_DXF_R2004:
        case lc::File::LIBDXFRW_DXB_R2004:
            exportVersion = DRW::AC1018;
            break;
        case lc::File::LIBDXFRW_DXF_R2007:
        case lc::File::LIBDXFRW_DXB_R2007:
            exportVersion = DRW::AC1021;
            break;
        case lc::File::LIBDXFRW_DXF_R2010:
        case lc::File::LIBDXFRW_DXB_R2010:
            exportVersion = DRW::AC1024;
            break;
        case lc::File::LIBDXFRW_DXF_R2013:
        case lc::File::LIBDXFRW_DXB_R2013:
            exportVersion = DRW::AC1027;
            break;
        default:
            exportVersion = DRW::AC1024;
        break;
    }

    bool isBinary = type >= lc::File::LIBDXFRW_DXB_R12 && type < lc::File::LIBDXFRW_DXB_R2013;

    bool success = dxfW->write(this, exportVersion, isBinary);
    delete dxfW;

    return success;
}

void DXFimpl::writePoint(const lc::entity::Point_CSPtr& p) {
    DRW_Point point;
    getEntityAttributes(&point, p);
    point.basePoint.x = p->x();
    point.basePoint.y = p->y();
    dxfW->writePoint(&point);
}

void DXFimpl::writeLine(const lc::entity::Line_CSPtr& l) {
    DRW_Line line;
    getEntityAttributes(&line, l);
    line.basePoint.x = l->start().x();
    line.basePoint.y = l->start().y();
    line.secPoint.x = l->end().x();
    line.secPoint.y = l->end().y();
    dxfW->writeLine(&line);
}

void DXFimpl::writeCircle(const lc::entity::Circle_CSPtr& c) {
    DRW_Circle circle;
    getEntityAttributes(&circle, c);
    circle.basePoint.x = c->center().x();
    circle.basePoint.y = c->center().y();
    circle.radious = c->radius();
    dxfW->writeCircle(&circle);
}

void DXFimpl::writeArc(const lc::entity::Arc_CSPtr& a) {
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

void DXFimpl::writeEllipse(const lc::entity::Ellipse_CSPtr& s) {
    DRW_Ellipse el;
    getEntityAttributes(&el, s);
    el.basePoint.x = s->center().x();
    el.basePoint.y = s->center().y();
    el.secPoint.x = s->majorP().x();
    el.secPoint.y = s->majorP().y();
    el.ratio = 1/s->ratio();
    if (s->isReversed()) {
        el.staparam = s->endAngle();
        el.endparam = s->startAngle();
    } else {
        el.staparam = s->startAngle();
        el.endparam = s->endAngle();
    }
    dxfW->writeEllipse(&el);
}

void DXFimpl::writeSpline(const lc::entity::Spline_CSPtr& s) {
    DRW_Spline sp;

    getEntityAttributes(&sp, s);

    sp.knotslist = s->knotPoints();
    sp.normalVec = DRW_Coord(s->nX(), s->nY(), s->nZ());
    sp.tgEnd = DRW_Coord(s->endTanX(), s->endTanY(), s->endTanZ());
    sp.tgStart = DRW_Coord(s->startTanX(), s->startTanY(), s->startTanZ());
    sp.degree = s->degree();

    for(const auto& cp : s->controlPoints()) {
        sp.controllist.push_back(new DRW_Coord(cp.x(), cp.y(), cp.z()));
    }

    for(const auto& fp : s->fitPoints()) {
        sp.fitlist.push_back(new DRW_Coord(fp.x(), fp.y(), fp.z()));
    }

    sp.flags = s->flags();
    sp.nknots = sp.knotslist.size();
    sp.nfit = sp.fitlist.size();
    sp.ncontrol = sp.controllist.size();

    dxfW->writeSpline(&sp);
}

void DXFimpl::writeInsert(const lc::entity::Insert_CSPtr& i) {
    DRW_Insert insert;
    getEntityAttributes(&insert, i);

    insert.name = i->displayBlock()->name();
    insert.basePoint.x = i->position().x();
    insert.basePoint.y = i->position().y();
    insert.basePoint.z = i->position().z();

    dxfW->writeInsert(&insert);
}

void DXFimpl::getEntityAttributes(DRW_Entity* ent, const lc::entity::CADEntity_CSPtr& entity) {
    auto layer_  = entity->layer();

    auto lpByValue = entity->metaInfo<lc::DxfLinePatternByValue>(lc::DxfLinePattern::LCMETANAME());
    auto lpByBlock = entity->metaInfo<lc::DxfLinePatternByBlock>(lc::DxfLinePattern::LCMETANAME());

    auto metaWidthByValue = entity->metaInfo<lc::MetaLineWidthByValue>(lc::MetaLineWidth::LCMETANAME());
    auto metaWidthByBlock = entity->metaInfo<lc::MetaLineWidthByBlock>(lc::MetaLineWidth::LCMETANAME());

    auto metaColorByBlock = entity->metaInfo<lc::MetaColorByBlock>(lc::MetaColor::LCMETANAME());
    auto metaColorByValue = entity->metaInfo<lc::MetaColorByValue>(lc::MetaColor::LCMETANAME());

    ent->layer = layer_->name();

    if(metaColorByBlock != nullptr) {
        ent->color = BYBLOCK_COLOR;
    }
    else if(metaColorByValue != nullptr) {
        lc::iColor col;
        auto color_ = col.colorToInt(metaColorByValue->color());
        ent->color = color_;
    }

    if(lpByValue != nullptr) {
        ent->lineType = lpByValue->name();
    }
    else if(lpByBlock != nullptr) {
        ent->lineType = LTYPE_BYBLOCK;
    }

    if(metaWidthByValue != nullptr) {
        ent->lWeight = static_cast<DRW_LW_Conv::lineWidth>(widthToInt(metaWidthByValue->width()));
    }
    else if(metaWidthByBlock != nullptr) {
        ent->lWeight = DRW_LW_Conv::lineWidth::widthByBlock;
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

void DXFimpl::writeDimension(const lc::entity::Dimension_CSPtr& d) {

}

void DXFimpl::writeLWPolyline(const lc::entity::LWPolyline_CSPtr& p) {

}

void DXFimpl::writeImage(const lc::entity::Image_CSPtr& i) {

}

void DXFimpl::writeText(const lc::entity::Text_CSPtr& t) {

}

void DXFimpl::writeEntities(){
    for(const auto& e :_document->entityContainer().asVector()) {
        if(e->block() != nullptr) {
            continue;
        }

        writeEntity(e);
    }
}

void DXFimpl::writeEntity(const lc::entity::CADEntity_CSPtr& entity) {
    auto line = std::dynamic_pointer_cast<const lc::entity::Line>(entity);
    if (line != nullptr) {
        writeLine(line);
        return;
    }

    auto circle = std::dynamic_pointer_cast<const lc::entity::Circle>(entity);
    if (circle != nullptr) {
        writeCircle(circle);
        return;
    }

    auto arc = std::dynamic_pointer_cast<const lc::entity::Arc>(entity);
    if (arc != nullptr) {
        writeArc(arc);
        return;
    }

    auto image = std::dynamic_pointer_cast<const lc::entity::Image>(entity);
    if (image != nullptr) {
        writeImage(image);
        return;
    }

    auto lwPolyline = std::dynamic_pointer_cast<const lc::entity::LWPolyline>(entity);
    if (lwPolyline != nullptr) {
        writeLWPolyline(lwPolyline);
        return;
    }

    auto text = std::dynamic_pointer_cast<const lc::entity::Text>(entity);
    if (text != nullptr) {
        writeText(text);
        return;
    }

    auto ellipse = std::dynamic_pointer_cast<const lc::entity::Ellipse>(entity);
    if (ellipse != nullptr) {
        writeEllipse(ellipse);
        return;
    }

    auto insert = std::dynamic_pointer_cast<const lc::entity::Insert>(entity);
    if (insert != nullptr) {
         writeInsert(insert);
         return;
    }
}

void DXFimpl::writeBlockRecords() {
    for(const auto& block : _document->blocks()) {
        dxfW->writeBlockRecord(block->name());
    }
}

void DXFimpl::writeBlocks() {
    for(const auto& block : _document->blocks()) {
        writeBlock(block);
    }
}

void DXFimpl::writeBlock(const lc::Block_CSPtr& block) {
    DRW_Block drwBlock;

    drwBlock.name = block->name();
    drwBlock.basePoint.x = block->base().x();
    drwBlock.basePoint.y = block->base().y();
    drwBlock.basePoint.z = block->base().z();

    auto customEntity = std::dynamic_pointer_cast<const lc::CustomEntityStorage>(block);
    if(customEntity) {
        auto list = std::list<DRW_Variant>({
            DRW_Variant(APP_NAME_CODE, APP_NAME),
            DRW_Variant(PLUGIN_NAME_CODE, customEntity->pluginName()),
            DRW_Variant(ENTITY_NAME_CODE, customEntity->entityName()),
        });

        for(const auto& data : customEntity->params()) {
            list.emplace_back(DRW_Variant(470, data.first));
            list.emplace_back(DRW_Variant(471, data.second));
        }

        drwBlock.appData.push_back(list);
    }

    dxfW->writeBlock(&drwBlock);

    for(const auto& entity : _document->entitiesByBlock(block).asVector()) {
        writeEntity(entity);
    }
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

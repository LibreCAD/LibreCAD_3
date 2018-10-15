#include <cad/meta/color.h>
#include <vector>
#include <cad/interface/metatype.h>
#include <cad/meta/layer.h>
#include <cad/meta/metacolor.h>
#include <cad/base/metainfo.h>
#include <cad/geometry/geocoordinate.h>
#include <cad/geometry/geovector.h>
#include <cad/storage/undomanager.h>
#include <cad/dochelpers/documentimpl.h>
#include <cad/dochelpers/undomanagerimpl.h>
#include <cad/dochelpers/storagemanagerimpl.h>
#include <cad/operations/entityops.h>
#include <cad/operations/entitybuilder.h>
#include <cad/operations/layerops.h>
#include <cad/meta/customentitystorage.h>
#include <cad/operations/blockops.h>
#include <cad/operations/builder.h>
#include <cad/primitive/insert.h>
#include <cad/const.h>
#include <cad/storage/settings.h>
#include <primitive/customentity.h>
#include "lclua.h"

using namespace LuaIntf;
using namespace lc::lua;

void LCLua::importLCKernel() {
    //18-09-2018: Those declarations are used to fix the CLang bug
    //            "candidate template ignored: couldn't infer template argument 'FN'"
    //            GCC is not affected
    using CADEntityBuilder_setLayer = void (builder::CADEntityBuilder::*)(const Layer_CSPtr&);
    using CADEntityBuilder_setMetaInfo = void (builder::CADEntityBuilder::*)(const MetaInfo_CSPtr&);
    using CADEntityBuilder_setBlock = void (builder::CADEntityBuilder::*)(const Block_CSPtr&);

    LuaBinding(_L)
        .beginClass<Color>("Color")
            .addConstructor(LUA_ARGS(
                                double r,
                                double g,
                                double b,
                                double a
                            )
            )
        .endClass()

        .beginClass<MetaType>("MetaType")
        .endClass()

        .beginExtendClass<EntityMetaType, MetaType>("EntityMetaType")
        .endClass()

        .beginExtendClass<DocumentMetaType, MetaType>("DocumentMetaType")
        .endClass()

        .beginExtendClass<Layer, MetaType>("Layer")
            .addConstructor(LUA_SP(Layer_SPtr), LUA_ARGS(
                                std::string,
                                MetaLineWidthByValue,
                                Color))
        .endClass()

        .beginExtendClass<DxfLinePattern, EntityMetaType>("DxfLinePattern")
        .endClass()

        .beginExtendClass<DxfLinePatternByValue, DxfLinePattern>("DxfLinePatternByValue")
            .addConstructor(LUA_SP(DxfLinePatternByValue_SPtr), LUA_ARGS(const std::string&,
                                                                   const std::string&,
                                                                   const std::vector<double>&,
                                                                   const double)
            )
        .endClass()

        .beginExtendClass<DxfLinePatternByBlock, DxfLinePattern>("DxfLinePatternByBlock")
            .addConstructor(LUA_SP(DxfLinePatternByBlock_SPtr), LUA_ARGS())
        .endClass()

        .beginExtendClass<MetaColorByValue, DocumentMetaType>("DocumentMetaColor")
           .addConstructor(LUA_SP(MetaColorByValue_SPtr), LUA_ARGS(const Color))
        .endClass()

        .beginExtendClass<MetaColorByValue, EntityMetaType>("EntityMetaColor")
          .addConstructor(LUA_SP(MetaColorByValue_SPtr), LUA_ARGS(const Color))
        .endClass()

        .beginExtendClass<MetaLineWidthByBlock, MetaType>("MetaLineWidthByBlock")
         .addConstructor(LUA_SP(MetaLineWidthByBlock_SPtr), LUA_ARGS())
        .endClass()

        .beginExtendClass<MetaLineWidthByValue, DocumentMetaType>("DocumentLineWidthByValue")
         .addConstructor(LUA_SP(MetaLineWidthByValue_SPtr), LUA_ARGS(const double))
        .endClass()

        .beginExtendClass<MetaLineWidthByValue, EntityMetaType>("EntityLineWidthByValue")
          .addConstructor(LUA_SP(MetaLineWidthByValue_SPtr), LUA_ARGS(const double))
        .endClass()

        .beginClass<MetaInfo>("MetaInfo")
            .addConstructor(LUA_SP(MetaInfo_SPtr), LUA_ARGS())
            .addFunction("add", &MetaInfo::add)
            .addFunction("addDxfLinePattern", &MetaInfo::addDxfLinePattern)
        .endClass()

        .beginClass<geo::Coordinate>("Coord")
            .addConstructor(LUA_ARGS(
                                double x,
                                double y))
            .addFunction("x", &geo::Coordinate::x)
            .addFunction("y", &geo::Coordinate::y)
            .addFunction("z", &geo::Coordinate::z)
        .endClass()

        .beginClass<geo::Coordinate>("Coordinate")
            .addConstructor(LUA_ARGS(
                                _opt<double> x,
                                _opt<double> y,
                                _opt<double> z))
            .addStaticFunction("_fromAngle", [](const double angle) {
                return geo::Coordinate(angle);
            })

            .addFunction("x", &geo::Coordinate::x)
            .addFunction("y", &geo::Coordinate::y)
            .addFunction("z", &geo::Coordinate::z)
            .addFunction("angleTo", &geo::Coordinate::angleTo)
            .addFunction("distanceTo", &geo::Coordinate::distanceTo)

            .addFunction("add", [](const geo::Coordinate* c1, const geo::Coordinate c2) {
                return *c1 + c2;
            })

            .addFunction("sub", [](const geo::Coordinate* c1, const geo::Coordinate c2) {
                return *c1 - c2;
            })

            .addFunction("mulDouble", [](const geo::Coordinate* c1, const double s) {
                return *c1 * s;
            })
        .endClass()

        .beginClass<geo::Vector>("Vector")
            .addConstructor(LUA_SP(std::shared_ptr<const geo::Vector>), LUA_ARGS(
                                const geo::Coordinate & start,
                                const geo::Coordinate & end))
        .endClass()

        .beginClass<Document>("Document")
            .addFunction("layerByName", &Document::layerByName)
            .addFunction("entitiesByLayer", &Document::entitiesByLayer)
            .addFunction("waitingCustomEntities", &Document::waitingCustomEntities)
            .addFunction("entitiesByBlock", &Document::entitiesByBlock)
        .endClass()

        .beginExtendClass<DocumentImpl, Document>("DocumentImpl")
        .endClass()

        .beginClass<UndoManager>("UndoManager")
            .addFunction("canRedo", &UndoManager::canRedo)
            .addFunction("canUndo", &UndoManager::canUndo)
            .addFunction("redo", &UndoManager::redo)
            .addFunction("removeUndoables", &UndoManager::removeUndoables)
            .addFunction("undo", &UndoManager::undo)
        .endClass()

        .beginExtendClass<UndoManagerImpl, UndoManager>("UndoManagerImpl")
        .endClass()

        .beginClass<EntityContainer<lc::entity::CADEntity_CSPtr>>("EntityContainer")
            .addFunction("asVector", &EntityContainer<lc::entity::CADEntity_CSPtr>::asVector, LUA_ARGS(
                    LuaIntf::_opt<short>
            ))
        .endClass()

        .beginClass<StorageManager>("StorageManager")
        .endClass()

        .beginExtendClass<StorageManagerImpl, StorageManager>("StorageManagerImpl")
            .addConstructor(LUA_SP(std::shared_ptr<StorageManagerImpl>), LUA_ARGS())
        .endClass()

        .beginClass<Visitable>("Visitable")
        .endClass()

        .beginClass<Snapable>("Snapable")
            .addFunction("snapPoints", &Snapable::snapPoints)
            .addFunction("nearestPointOnPath", &Snapable::nearestPointOnPath)
            .addStaticFunction("remove_ifDistanceGreaterThen", &Snapable::remove_ifDistanceGreaterThen)
            .addStaticFunction("snapPointsCleanup", &Snapable::snapPointsCleanup)
        .endClass()

        .beginClass<ID>("ID")
            .addConstructor(LUA_ARGS(_opt<ID_DATATYPE>))
            .addFunction("id", &ID::id)
            .addFunction("setId", &ID::setID)
        .endClass()

        .beginExtendClass<entity::CADEntity, ID>("CADEntity")
            .addFunction("move", &entity::CADEntity::move)
            .addFunction("rotate", &entity::CADEntity::rotate)
            .addFunction("copy", &entity::CADEntity::copy)
            .addFunction("scale", &entity::CADEntity::scale)
            .addFunction("mirror", &entity::CADEntity::mirror)

            .addFunction("layer", &entity::CADEntity::layer)
            .addFunction("metaInfo", [](const entity::CADEntity* ce) {
                return ce->metaInfo();
            })

            .addProperty("entityType", [](entity::CADEntity*) {
                return "unknown";
            })
        .endClass()

        .beginExtendClass<entity::Line, entity::CADEntity>("Line")
            .addConstructor(LUA_SP(entity::Line_SPtr), LUA_ARGS(
                       const geo::Coordinate & start,
                       const geo::Coordinate & end,
                       const Layer_CSPtr,
                       LuaIntf::_opt<const MetaInfo_CSPtr>,
                       LuaIntf::_opt<const Block_CSPtr>
            ))
            .addProperty("entityType", [](entity::Line*) {
                return "line";
            })
            .addFunction("nearestPointOnEntity", &entity::Line::nearestPointOnEntity)
            .addFunction("nearestPointOnPath", &entity::Line::nearestPointOnPath)
            .addFunction("start", &geo::Vector::start)
            .addFunction("finish", &geo::Vector::end) //"end" will make Lua crash
        .endClass()

        .beginExtendClass<entity::Circle, entity::CADEntity>("Circle")
            .addConstructor(LUA_SP(entity::Circle_SPtr), LUA_ARGS(
                       const geo::Coordinate & center,
                       double radius,
                       const Layer_CSPtr,
                       LuaIntf::_opt<const MetaInfo_CSPtr>,
                       LuaIntf::_opt<const Block_CSPtr>
            ))
            .addProperty("entityType", [](entity::Circle*) {
                return "circle";
            })
            .addFunction("nearestPointOnEntity", &geo::Circle::nearestPointOnEntity)
            .addFunction("nearestPointOnPath", &geo::Circle::nearestPointOnPath)
            .addFunction("center", &entity::Circle::center)
            .addFunction("radius", &entity::Circle::radius)
        .endClass()

        .beginExtendClass<entity::Arc, entity::CADEntity>("Arc")
            .addConstructor(LUA_SP(entity::Arc_SPtr), LUA_ARGS(
                       const geo::Coordinate & center,
                       double radius,
                       const double startAngle,
                       const double endAngle,
                       bool CCW,
                       const Layer_CSPtr layer,
                       LuaIntf::_opt<const MetaInfo_CSPtr>,
                       LuaIntf::_opt<const Block_CSPtr>
            ))

            .addFunction("nearestPointOnEntity", &geo::Arc::nearestPointOnEntity)
            .addFunction("nearestPointOnPath", &geo::Arc::nearestPointOnPath)

            .addFunction("angle", &geo::Arc::angle)
            .addFunction("startAngle", &geo::Arc::startAngle)
            .addFunction("endAngle", &geo::Arc::endAngle)
            .addFunction("center", &entity::Arc::center)
            .addFunction("radius", &entity::Arc::radius)
            .addFunction("CCW", &geo::Arc::CCW)

            .addProperty("entityType", [](entity::CADEntity*) {
                return "arc";
            })
        .endClass()

        .beginExtendClass<entity::Ellipse, entity::CADEntity>("Ellipse")
            .addConstructor(LUA_SP(entity::Ellipse_SPtr), LUA_ARGS(
                       const geo::Coordinate & center,
                       const geo::Coordinate & majorP,
                       double minorRadius,
                       double startAngle,
                       double endAngle,
                       bool reversed,
                       const Layer_CSPtr layer,
                       LuaIntf::_opt<const MetaInfo_CSPtr>,
                       LuaIntf::_opt<const Block_CSPtr>
            ))

            .addFunction("getEllipseAngle", &entity::Ellipse::getEllipseAngle)

            .endClass()

        .beginExtendClass<entity::Point, entity::CADEntity>("Point_")
        .endClass()

        .beginExtendClass<entity::Text, entity::CADEntity>("Text_")
        .endClass()

        .beginExtendClass<entity::DimRadial, entity::CADEntity>("DimRadial")
            .addConstructor(LUA_SP(entity::DimRadial_SPtr), LUA_ARGS(
                const geo::Coordinate&,
                const geo::Coordinate&,
                const TextConst::AttachmentPoint&,
                double,
                const double,
                const TextConst::LineSpacingStyle&,
                const std::string&,
                const geo::Coordinate&,
                const double,
                const Layer_CSPtr,
                LuaIntf::_opt<const MetaInfo_CSPtr>,
                LuaIntf::_opt<const Block_CSPtr>
            ))
        .endClass()

        .beginExtendClass<entity::DimDiametric, entity::CADEntity>("DimDiametric")
            .addConstructor(LUA_SP(entity::DimDiametric_SPtr), LUA_ARGS(
                const geo::Coordinate&,
                const TextConst::AttachmentPoint&,
                const double,
                const TextConst::LineSpacingStyle&,
                const std::string&,
                const geo::Coordinate&,
                const double,
                const Layer_CSPtr,
                LuaIntf::_opt<const MetaInfo_CSPtr>,
                LuaIntf::_opt<const Block_CSPtr>
            ))
        .endClass()

        .beginExtendClass<entity::DimLinear, entity::CADEntity>("DimLinear")
            .addStaticFunction("dimAuto", &entity::DimLinear::dimAuto, LUA_ARGS(
                    geo::Coordinate const&,
                    geo::Coordinate const&,
                    geo::Coordinate const&,
                    std::string const&,
                    const Layer_CSPtr,
                    LuaIntf::_opt<const MetaInfo_CSPtr>,
                    LuaIntf::_opt<const Block_CSPtr>
            ))
        .endClass()

        .beginExtendClass<entity::DimAligned, entity::CADEntity>("DimAligned")
            .addStaticFunction("dimAuto", &entity::DimAligned::dimAuto, LUA_ARGS(
                geo::Coordinate const&,
                geo::Coordinate const&,
                geo::Coordinate const&,
                std::string const&,
                const Layer_CSPtr,
                LuaIntf::_opt<const MetaInfo_CSPtr>,
                LuaIntf::_opt<const Block_CSPtr>
            ))
        .endClass()

        .beginExtendClass<entity::DimAngular, entity::CADEntity>("DimAngular")
            .addStaticFunction("dimAuto", &entity::DimAngular::dimAuto, LUA_ARGS(
                geo::Coordinate const&,
                geo::Coordinate const&,
                geo::Coordinate const&,
                std::string const&,
                const Layer_CSPtr,
                LuaIntf::_opt<const MetaInfo_CSPtr>,
                LuaIntf::_opt<const Block_CSPtr>
            ))
        .endClass()

        .beginExtendClass<entity::Spline, entity::CADEntity>("Spline")
            .addConstructor(LUA_SP(entity::Spline_SPtr), LUA_ARGS(
                const std::vector<geo::Coordinate>,
                const std::vector<double>,
                const std::vector<geo::Coordinate>,
                int,
                bool,
                double,
                double,
                double,
                double,
                double,
                double,
                double,
                double,
                double,
                double,
                geo::Spline::splineflag,
                const Layer_CSPtr,
                LuaIntf::_opt<const MetaInfo_CSPtr>,
                LuaIntf::_opt<const Block_CSPtr>
            ))
        .endClass()

        .beginClass<entity::LWVertex2D>("LWVertex2D")
            .addConstructor(LUA_ARGS(
                                    const geo::Coordinate & pos, _opt<double>, _opt<double>, _opt<double>))
                                    .addFunction("location", &lc::entity::LWVertex2D::location)
                                    .addFunction("bulge", &lc::entity::LWVertex2D::bulge)
                                    .addFunction("startWidth", &lc::entity::LWVertex2D::startWidth)
                                    .addFunction("endWidth", &lc::entity::LWVertex2D::endWidth)
        .endClass()

        .beginExtendClass<entity::LWPolyline, entity::CADEntity>("LWPolyline")
            .addConstructor(LUA_SP(entity::LWPolyline_SPtr), LUA_ARGS(
                    const std::vector<entity::LWVertex2D>&,
                    double,
                    double,
                    double,
                    bool,
                    const geo::Coordinate,
                    const Layer_CSPtr,
                    LuaIntf::_opt<const MetaInfo_CSPtr>,
                    LuaIntf::_opt<const Block_CSPtr>
            ))
            .addFunction("width", &lc::entity::LWPolyline::width)
            .addFunction("elevation", &lc::entity::LWPolyline::elevation)
            .addFunction("tickness", &lc::entity::LWPolyline::tickness)
            .addFunction("extrusionDirection", &lc::entity::LWPolyline::extrusionDirection)
            .addFunction("closed", &lc::entity::LWPolyline::closed)
        .endClass()

        .beginClass<operation::DocumentOperation>("DocumentOperation")
            .addFunction("execute", &operation::DocumentOperation::execute)
        .endClass()

        .beginExtendClass<operation::Builder, operation::DocumentOperation>("Builder")
            .addConstructor(LUA_SP(operation::Builder_SPtr), LUA_ARGS(
                    std::shared_ptr<lc::Document>,
                    const std::string&
            ))
            .addFunction("append", &operation::Builder::append)
        .endClass()

        .beginExtendClass<operation::EntityBuilder, operation::DocumentOperation>("EntityBuilder")
            .addConstructor(LUA_SP(operation::EntityBuilder_SPtr), LUA_ARGS(
                    std::shared_ptr<lc::Document>
            ))
            .addFunction("appendEntity", &operation::EntityBuilder::appendEntity)
            .addFunction("appendOperation", &operation::EntityBuilder::appendOperation)
            .addFunction("processStack", &operation::EntityBuilder::processStack)
        .endClass()

        .beginClass<lc::IntersectMany>("IntersectMany")
            .addConstructor(LUA_ARGS(
                                    std::vector<lc::entity::CADEntity_CSPtr>,
                                    _opt<lc::Intersect::Method>,
                                    _opt<double>
                            )
            )
            .addFunction("result", &lc::IntersectMany::result)
        .endClass()

        .beginClass<operation::Base>("Base")
        .endClass()

        .beginExtendClass<operation::Move, operation::Base>("Move")
            .addConstructor(LUA_SP(std::shared_ptr<operation::Move>), LUA_ARGS(const geo::Coordinate&))
        .endClass()

        .beginExtendClass<operation::Begin, operation::Base>("Begin")
            .addConstructor(LUA_SP(std::shared_ptr<operation::Begin>), LUA_ARGS())
        .endClass()

        .beginExtendClass<operation::Loop, operation::Base>("Loop")
            .addConstructor(LUA_SP(std::shared_ptr<operation::Loop>), LUA_ARGS(const int))
        .endClass()

        .beginExtendClass<operation::Copy, operation::Base>("Copy")
            .addConstructor(LUA_SP(std::shared_ptr<operation::Copy>), LUA_ARGS(const geo::Coordinate&))
        .endClass()

        .beginExtendClass<operation::Scale, operation::Base>("Scale")
            .addConstructor(LUA_SP(std::shared_ptr<operation::Scale>), LUA_ARGS(
                    const geo::Coordinate&,
                    const geo::Coordinate&
            ))
        .endClass()

        .beginExtendClass<operation::Push, operation::Base>("Push")
            .addConstructor(LUA_SP(std::shared_ptr<operation::Push>), LUA_ARGS())
        .endClass()

        .beginExtendClass<operation::SelectByLayer, operation::Base>("SelectByLayer")
            .addConstructor(LUA_SP(std::shared_ptr<operation::SelectByLayer>), LUA_ARGS(const Layer_CSPtr))
        .endClass()

        .beginExtendClass<operation::Rotate, operation::Base>("Rotate")
            .addConstructor(LUA_SP(std::shared_ptr<operation::Rotate>), LUA_ARGS(
                    const geo::Coordinate&,
                    const double
            ))
        .endClass()

        .beginExtendClass<operation::Remove, operation::Base>("Remove")
            .addConstructor(LUA_SP(std::shared_ptr<operation::Remove>), LUA_ARGS())
        .endClass()

        .beginExtendClass<operation::AddLayer, operation::DocumentOperation>("AddLayer")
            .addConstructor(LUA_SP(std::shared_ptr<lc::operation::AddLayer>), LUA_ARGS(
                    std::shared_ptr<lc::Document> doc,
                    const Layer_CSPtr
            ))
        .endClass()

        .beginExtendClass<operation::RemoveLayer, operation::DocumentOperation>("RemoveLayer")
            .addConstructor(LUA_SP(std::shared_ptr<lc::operation::RemoveLayer>), LUA_ARGS(
                    std::shared_ptr<lc::Document> doc,
                    const Layer_CSPtr
            ))
        .endClass()

        .beginExtendClass<operation::ReplaceLayer, operation::DocumentOperation>("ReplaceLayer")
            .addConstructor(LUA_SP(std::shared_ptr<lc::operation::ReplaceLayer>), LUA_ARGS(
                    std::shared_ptr<lc::Document> doc,
                    const Layer_CSPtr,
                    const Layer_CSPtr
            ))
        .endClass()

        .beginExtendClass<Block, DocumentMetaType>("Block")
            .addConstructor(LUA_SP(Block_SPtr), LUA_ARGS(
                std::string,
                geo::Coordinate
            ))
            .addFunction("base", &Block::base)
        .endClass()

        .beginExtendClass<CustomEntityStorage, Block>("CustomEntityStorage")
            .addConstructor(LUA_SP(CustomEntityStorage_SPtr), LUA_ARGS(
                const std::string&,
                const std::string&,
                const geo::Coordinate&,
                LuaIntf::_opt<const std::map<std::string, std::string>&>
            ))

            .addFunction("pluginName", &CustomEntityStorage::pluginName)
            .addFunction("entityName", &CustomEntityStorage::entityName)
            .addFunction("param", &CustomEntityStorage::param)
            .addFunction("setParam", &CustomEntityStorage::setParam)
        .endClass()

        .beginExtendClass<operation::AddBlock, operation::DocumentOperation>("AddBlock")
            .addConstructor(LUA_SP(std::shared_ptr<lc::operation::AddBlock>), LUA_ARGS(
                    std::shared_ptr<lc::Document>,
                    const Block_CSPtr
            ))
        .endClass()

        .beginExtendClass<operation::RemoveBlock, operation::DocumentOperation>("RemoveBlock")
            .addConstructor(LUA_SP(std::shared_ptr<lc::operation::RemoveBlock>), LUA_ARGS(
                    std::shared_ptr<lc::Document>,
                    const Block_CSPtr
            ))
        .endClass()

        .beginExtendClass<operation::ReplaceBlock, operation::DocumentOperation>("ReplaceBlock")
            .addConstructor(LUA_SP(std::shared_ptr<lc::operation::ReplaceBlock>), LUA_ARGS(
                    std::shared_ptr<lc::Document>,
                    const Block_CSPtr,
                    const Block_CSPtr
            ))
        .endClass()

        .beginClass<builder::CADEntityBuilder>("CADEntityBuilder")
            .addFunction("layer", &builder::CADEntityBuilder::layer)
            .addFunction<CADEntityBuilder_setLayer>("setLayer", &builder::CADEntityBuilder::setLayer)
            .addFunction("metaInfo", &builder::CADEntityBuilder::metaInfo)
            .addFunction<CADEntityBuilder_setMetaInfo>("setMetaInfo", &builder::CADEntityBuilder::setMetaInfo)
            .addFunction("block", &builder::CADEntityBuilder::block)
            .addFunction<CADEntityBuilder_setBlock>("setBlock", &builder::CADEntityBuilder::setBlock)
            .addFunction("id", &builder::CADEntityBuilder::id)
            .addFunction("setID", &builder::CADEntityBuilder::setID)
            .addFunction("newID", &builder::CADEntityBuilder::newID)
            .addFunction("checkValues", &builder::CADEntityBuilder::checkValues)
        .endClass()

        .beginExtendClass<builder::ArcBuilder, builder::CADEntityBuilder>("ArcBuilder")
            .addConstructor(LUA_ARGS())
            .addFunction("center", &builder::ArcBuilder::center)
            .addFunction("setCenter", &builder::ArcBuilder::setCenter)
            .addFunction("radius", &builder::ArcBuilder::radius)
            .addFunction("setRadius", &builder::ArcBuilder::setRadius)
            .addFunction("startAngle", &builder::ArcBuilder::startAngle)
            .addFunction("setStartAngle", &builder::ArcBuilder::setStartAngle)
            .addFunction("endAngle", &builder::ArcBuilder::endAngle)
            .addFunction("setEndAngle", &builder::ArcBuilder::setEndAngle)
            .addFunction("isCCW", &builder::ArcBuilder::isCCW)
            .addFunction("setIsCCW", &builder::ArcBuilder::setIsCCW)
            .addFunction("build", &builder::ArcBuilder::build)
        .endClass()

        .beginExtendClass<builder::CircleBuilder, builder::CADEntityBuilder>("CircleBuilder")
            .addConstructor(LUA_ARGS())
            .addFunction("center", &builder::CircleBuilder::center)
            .addFunction("setCenter", &builder::CircleBuilder::setCenter)
            .addFunction("radius", &builder::CircleBuilder::radius)
            .addFunction("setRadius", &builder::CircleBuilder::setRadius)
            .addFunction("build", &builder::CircleBuilder::build)
        .endClass()

        .beginExtendClass<builder::InsertBuilder, builder::CADEntityBuilder>("InsertBuilder")
            .addConstructor(LUA_ARGS())
            .addFunction("displayBlock", &builder::InsertBuilder::displayBlock)
            .addFunction("setDisplayBlock", &builder::InsertBuilder::setDisplayBlock)
            .addFunction("coordinate", &builder::InsertBuilder::coordinate)
            .addFunction("setCoordinate", &builder::InsertBuilder::setCoordinate)
            .addFunction("document", &builder::InsertBuilder::document)
            .addFunction("setDocument", &builder::InsertBuilder::setDocument)
            .addFunction("copy", &builder::InsertBuilder::copy)
            .addFunction("build", &builder::InsertBuilder::build)
        .endClass()

        .beginClass<builder::LayerBuilder>("LayerBuilder")
            .addConstructor(LUA_ARGS())
            .addFunction("name", &builder::LayerBuilder::name)
            .addFunction("setName", &builder::LayerBuilder::setName)
            .addFunction("lineWidth", &builder::LayerBuilder::lineWidth)
            .addFunction("setLineWidth", &builder::LayerBuilder::setLineWidth)
            .addFunction("color", &builder::LayerBuilder::color)
            .addFunction("setColor", &builder::LayerBuilder::setColor)
            .addFunction("linePattern", &builder::LayerBuilder::linePattern)
            .addFunction("setLinePattern", &builder::LayerBuilder::setLinePattern)
            .addFunction("isFrozen", &builder::LayerBuilder::isFrozen)
            .addFunction("setIsFrozen", &builder::LayerBuilder::setIsFrozen)
            .addFunction("build", &builder::LayerBuilder::build)
        .endClass()

        .beginExtendClass<builder::LineBuilder, builder::CADEntityBuilder>("LineBuilder")
            .addConstructor(LUA_ARGS())
            .addFunction("start", &builder::LineBuilder::start)
            .addFunction("setStart", &builder::LineBuilder::setStart)
            .addFunction("end", &builder::LineBuilder::end)
            .addFunction("setEnd", &builder::LineBuilder::setEnd)
            .addFunction("build", &builder::LineBuilder::build)
        .endClass()

        .beginClass<builder::LinePatternBuilder>("LinePatternBuilder")
            .addConstructor(LUA_ARGS())
            .addFunction("name", &builder::LinePatternBuilder::name)
            .addFunction("setName", &builder::LinePatternBuilder::setName)
            .addFunction("description", &builder::LinePatternBuilder::description)
            .addFunction("setDescription", &builder::LinePatternBuilder::setDescription)
            .addFunction("path", &builder::LinePatternBuilder::path)
            .addFunction("setPath", &builder::LinePatternBuilder::setPath)
            .addFunction("addElement", &builder::LinePatternBuilder::addElement)
            .addFunction("checkValues", &builder::LinePatternBuilder::checkValues)
            .addFunction("build", &builder::LinePatternBuilder::build)
        .endClass()

        .beginExtendClass<builder::PointBuilder, builder::CADEntityBuilder>("PointBuilder")
            .addConstructor(LUA_ARGS())
            .addFunction("coordinate", &builder::PointBuilder::coordinate)
            .addFunction("setCoordinate", &builder::PointBuilder::setCoordinate)
            .addFunction("build", &builder::PointBuilder::build)
        .endClass()

        .beginExtendClass<entity::Insert, entity::CADEntity>("Insert")
            .addFunction("displayBlock", &entity::Insert::displayBlock)
            .addFunction("position", &entity::Insert::position)
            .addFunction("document", &entity::Insert::document)
        .endClass()

        .beginExtendClass<entity::CustomEntity, entity::Insert>("CustomEntity")
        .endClass()

        .beginExtendClass<entity::LuaCustomEntity, entity::CustomEntity>("LuaCustomEntity")
        .endClass()

        .beginExtendClass<builder::CustomEntityBuilder, builder::InsertBuilder>("CustomEntityBuilder")
            .addConstructor(LUA_ARGS())
            .addFunction("snapFunction", &builder::CustomEntityBuilder::snapFunction)
            .addFunction("setSnapFunction", &builder::CustomEntityBuilder::setSnapFunction)
            .addFunction("nearestPointFunction", &builder::CustomEntityBuilder::nearestPointFunction)
            .addFunction("setNearestPointFunction", &builder::CustomEntityBuilder::setNearestPointFunction)
            .addFunction("dragPointsFunction", &builder::CustomEntityBuilder::dragPointsFunction)
            .addFunction("setDragPointsFunction", &builder::CustomEntityBuilder::setDragPointsFunction)
            .addFunction("newDragPointFunction", &builder::CustomEntityBuilder::newDragPointFunction)
            .addFunction("setNewDragPointFunction", &builder::CustomEntityBuilder::setNewDragPointFunction)
            .addFunction("dragPointsClickedFunction", &builder::CustomEntityBuilder::dragPointsClickedFunction)
            .addFunction("setDragPointsClickedFunction", &builder::CustomEntityBuilder::setDragPointsClickedFunction)
            .addFunction("dragPointsReleasedFunction", &builder::CustomEntityBuilder::dragPointsReleasedFunction)
            .addFunction("setDragPointsReleasedFunction", &builder::CustomEntityBuilder::setDragPointsReleasedFunction)
            .addFunction("checkValues", &builder::CustomEntityBuilder::checkValues)
            .addFunction("build", &builder::CustomEntityBuilder::build)
        .endClass()

        .beginClass<lc::SimpleSnapConstrain>("SimpleSnapConstrain")
            .addConstant("NONE", SimpleSnapConstrain::NONE)
            .addConstant("ON_ENTITY", SimpleSnapConstrain::ON_ENTITY)
            .addConstant("ON_ENTITYPATH", SimpleSnapConstrain::ON_ENTITYPATH)
            .addConstant("ENTITY_CENTER", SimpleSnapConstrain::ENTITY_CENTER)
            .addConstant("LOGICAL", SimpleSnapConstrain::LOGICAL)
            .addConstant("DIVIDED", SimpleSnapConstrain::DIVIDED)

            .addFunction("constrain", &SimpleSnapConstrain::constrain)
            .addFunction("divisions", &SimpleSnapConstrain::divisions)
            .addFunction("angle", &SimpleSnapConstrain::angle)
            .addFunction("setDivisions", &SimpleSnapConstrain::setDivisions)
            .addFunction("setAngle", &SimpleSnapConstrain::setAngle)
            .addFunction("enableConstrain", &SimpleSnapConstrain::enableConstrain)
            .addFunction("disableConstrain", &SimpleSnapConstrain::disableConstrain)
            .addFunction("hasConstrain", &SimpleSnapConstrain::hasConstrain)
        .endClass()

        .beginClass<EntityCoordinate>("EntityCoordinate")
            .addConstructor(LUA_ARGS(const geo::Coordinate&, int))
            .addFunction("coordinate", &EntityCoordinate::coordinate)
            .addFunction("pointId", &EntityCoordinate::pointId)
        .endClass()       
        ;
}

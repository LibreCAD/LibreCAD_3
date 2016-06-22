extern "C"
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

#include "cad/lualibrecadbridge.h"

namespace LuaIntf {
    LUA_USING_SHARED_PTR_TYPE(std::shared_ptr)
	LUA_USING_LIST_TYPE(std::vector)
}

using namespace LuaIntf;
using namespace lc;

/*
    .addConstant("VAlign_VABaseline", 0)
    .addConstant("VAlign_VABottom", 1)
    .addConstant("VAlign_VAMiddle", 2)
    .addConstant("VAlign_VATop", 3)
    .addConstant("HAlign_HALeft", 0)
    .addConstant("HAlign_HACenter", 1)
    .addConstant("HAlign_HARight", 2)
    .addConstant("HAlign_HAAligned", 3)
    .addConstant("HAlign_HAMiddle", 4)
    .addConstant("HAlign_HAFit", 5)

    .addConstant("AttachmentPoint_Top_left", 1)
    .addConstant("AttachmentPoint_Top_center", 2)
    .addConstant("AttachmentPoint_Top_right", 4)
    .addConstant("AttachmentPoint_Middle_left", 8+1)
    .addConstant("AttachmentPoint_Middle_center", 8+2)
    .addConstant("AttachmentPoint_Middle_right", 8+4)
    .addConstant("AttachmentPoint_Bottom_left", 16+1)
    .addConstant("AttachmentPoint_Bottom_center", 16+2)
    .addConstant("AttachmentPoint_Bottom_right", 16+4)
    .addConstant("DrawingDirection_None", 0)
    .addConstant("DrawingDirection_Backward", 1)
    .addConstant("DrawingDirection_Backward", 2)
    .addConstant("LineSpacingStyle_AtLeast", 0)
    .addConstant("LineSpacingStyle_Exact", 1)
 */

void lua_openlckernel(lua_State* L) {

    LuaBinding(L)



    .beginClass<Color>("Color")
    .addConstructor(LUA_ARGS(
                        double r,
                        double g,
                        double b,
                        double a))
    .endClass()

    .beginClass<MetaType>("MetaType")
    .endClass()

    .beginExtendClass<EntityMetaType, MetaType>("EntityMetaType")
    .endClass()

    .beginExtendClass<DocumentMetaType, MetaType>("DocumentMetaType")
    .endClass()

    .beginExtendClass<Layer, MetaType>("Layer")
    .addConstructor(LUA_SP(Layer_SPtr), LUA_ARGS(
                        const std::string,
                        const Color))
    .endClass()

    .beginExtendClass<MetaColor, DocumentMetaType>("DocumentMetaColor")
    .addConstructor(LUA_SP(MetaColor_SPtr), LUA_ARGS(const Color))
    .endClass()
    .beginExtendClass<MetaColor, EntityMetaType>("EntityMetaColor")
    .addConstructor(LUA_SP(MetaColor_SPtr), LUA_ARGS(const Color))
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
    .addConstructor(LUA_SP(std::shared_ptr<lc::MetaInfo>), LUA_ARGS())
    .addFunction("add", &lc::MetaInfo::add)
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
    .endClass()
    .beginExtendClass<DocumentImpl, Document>("DocumentImpl")
    .addFunction("layerByName", &DocumentImpl::layerByName)
    .addFunction("entitiesByLayer", &DocumentImpl::entitiesByLayer)
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
                                                   .endClass()

                                                   .beginClass<StorageManager>("StorageManager")
                                                   .endClass()
                                                   .beginExtendClass<StorageManagerImpl, StorageManager>("StorageManagerImpl")
                                                   .addConstructor(LUA_SP(std::shared_ptr<StorageManagerImpl>), LUA_ARGS())
                                                   .endClass()

    .beginClass<Visitable>("Visitable")
    .endClass()
    .beginClass<Snapable>("Snapable")
    .endClass()
   .beginClass<ID>("ID")
			.addConstructor(LUA_ARGS(_opt<ID_DATATYPE>))
			.addFunction("id", &ID::id)
			.addFunction("setId", &ID::setID)
   .endClass()
    .beginExtendClass<entity::CADEntity, ID>("CADEntity")
	.endClass()
	.beginExtendClass<entity::Line, entity::CADEntity>("Line")
		.addConstructor(LUA_SP(entity::Line_SPtr), LUA_ARGS(
				   const geo::Coordinate & start,
				   const geo::Coordinate & end,
				   const Layer_CSPtr))
		.addFunction("nearestPointOnPath", &geo::Vector::nearestPointOnPath)
	.endClass()
	.beginExtendClass<entity::Circle, entity::CADEntity>("Circle")
		.addConstructor(LUA_SP(entity::Circle_SPtr), LUA_ARGS(
				   const geo::Coordinate & center,
				   double radius,
				   const Layer_CSPtr))
	.endClass()
	.beginExtendClass<entity::Arc, entity::CADEntity>("Arc")
		.addConstructor(LUA_SP(entity::Arc_SPtr), LUA_ARGS(
				   const geo::Coordinate & center,
				   double radius,
				   const double startAngle,
				   const double endAngle,
				   bool CCW,
				   const Layer_CSPtr layer))
	.endClass()
	.beginExtendClass<entity::Ellipse, entity::CADEntity>("Ellipse")
		.addConstructor(LUA_SP(entity::Ellipse_SPtr), LUA_ARGS(
				   const geo::Coordinate & center,
				   const geo::Coordinate & majorP,
				   double minorRadius,
				   double startAngle,
				   double endAngle,
				   bool reversed,
				   const Layer_CSPtr layer))
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
			const MetaInfo_CSPtr
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
			const MetaInfo_CSPtr))
	.endClass()
	.beginExtendClass<entity::DimLinear, entity::CADEntity>("DimLinear")
		.addStaticFunction("dimAuto", &entity::DimLinear::dimAuto)
	.endClass()
	.beginExtendClass<entity::DimAligned, entity::CADEntity>("DimAligned")
		.addStaticFunction("dimAuto", &entity::DimAligned::dimAuto)
	.endClass()
	.beginExtendClass<entity::DimAngular, entity::CADEntity>("DimAngular")
		.addStaticFunction("dimAuto", &entity::DimAngular::dimAuto)
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
			const MetaInfo_CSPtr
		))
	.endClass()
	.beginClass<entity::LWVertex2D>("LWVertex2D")
		.addConstructor(LUA_ARGS(
								const geo::Coordinate & pos, _opt<double>, _opt<double>, _opt<double>))
								.addFunction("bulge", &lc::entity::LWVertex2D::bulge)
								.addFunction("startWidth", &lc::entity::LWVertex2D::startWidth)
								.addFunction("endWidth", &lc::entity::LWVertex2D::endWidth)
	.endClass()
	.beginExtendClass<entity::LWPolyline, entity::CADEntity>("LWPolyline_")
		.addFunction("width", &lc::entity::LWPolyline::width)
		.addFunction("elevation", &lc::entity::LWPolyline::elevation)
		.addFunction("tickness", &lc::entity::LWPolyline::tickness)
		.addFunction("extrusionDirection", &lc::entity::LWPolyline::extrusionDirection)
		.addFunction("closed", &lc::entity::LWPolyline::closed)
	.endClass()
	.beginClass<operation::DocumentOperation>("DocumentOperation")
		.addFunction("execute", &operation::DocumentOperation::execute)
	.endClass()
	.beginExtendClass<lc::operation::Builder, operation::DocumentOperation>("Builder")
		.addConstructor(LUA_SP(std::shared_ptr<lc::operation::Builder>), LUA_ARGS(std::shared_ptr<lc::Document> doc))
		.addFunction("append", &lc::operation::Builder::append)
		.addFunction("move", &lc::operation::Builder::move)
		.addFunction("copy", &lc::operation::Builder::copy)
		.addFunction("scale", &lc::operation::Builder::scale)
		.addFunction("rotate", &lc::operation::Builder::rotate)
		.addFunction("push", &lc::operation::Builder::push)
		.addFunction("loop", &lc::operation::Builder::repeat)
		.addFunction("begin", &lc::operation::Builder::begin)
		.addFunction("selectByLayer", &lc::operation::Builder::selectByLayer)
		.addFunction("remove", &lc::operation::Builder::remove)
	.endClass()

	.beginClass<operation::Base>("Base")
	.endClass()
	.beginExtendClass<operation::Move, operation::Base>("Move")
		.addConstructor(LUA_SP(std::shared_ptr<operation::Move>), LUA_ARGS(const geo::Coordinate & offset))
	.endClass()
	.beginExtendClass<operation::Begin, operation::Base>("Begin")
		.addConstructor(LUA_SP(std::shared_ptr<operation::Begin>), LUA_ARGS())
	.endClass()
	.beginExtendClass<operation::Loop, operation::Base>("Loop")
		.addConstructor(LUA_SP(std::shared_ptr<operation::Loop>), LUA_ARGS(const int numTimes))
	.endClass()
	.beginExtendClass<operation::Copy, operation::Base>("Copy")
		.addConstructor(LUA_SP(std::shared_ptr<operation::Copy>), LUA_ARGS(const geo::Coordinate & offset))
	.endClass()
	.beginExtendClass<operation::Scale, operation::Base>("Scale")
		.addConstructor(LUA_SP(std::shared_ptr<operation::Scale>), LUA_ARGS(const geo::Coordinate & scale_center, const geo::Coordinate & scale_factor))
	.endClass()
	.beginExtendClass<operation::Push, operation::Base>("Push")
		.addConstructor(LUA_SP(std::shared_ptr<operation::Push>), LUA_ARGS())
	.endClass()
	.beginExtendClass<operation::SelectByLayer, operation::Base>("SelectByLayer")
		.addConstructor(LUA_SP(std::shared_ptr<operation::SelectByLayer>), LUA_ARGS(const Layer_CSPtr))
	.endClass()

	.beginExtendClass<operation::AddLayer, operation::DocumentOperation>("AddLayer")
		.addConstructor(LUA_SP(std::shared_ptr<lc::operation::AddLayer>), LUA_ARGS(std::shared_ptr<lc::Document> doc, const Layer_CSPtr))
	.endClass()
	.beginExtendClass<operation::RemoveLayer, operation::DocumentOperation>("RemoveLayer")
		.addConstructor(LUA_SP(std::shared_ptr<lc::operation::RemoveLayer>), LUA_ARGS(std::shared_ptr<lc::Document> doc, const Layer_CSPtr))
	.endClass()
	.beginExtendClass<operation::ReplaceLayer, operation::DocumentOperation>("ReplaceLayer")
	.addConstructor(LUA_SP(std::shared_ptr<lc::operation::ReplaceLayer>), LUA_ARGS(std::shared_ptr<lc::Document> doc, const Layer_CSPtr, const Layer_CSPtr))
	.endClass();


}


/*

            .beginExtendClass<Dimension, entity::CADEntity>("Dimension")
            .addConstructor(LUA_SP(Text_SPtr), LUA_ARGS(
                    const geo::Coordinate &definition_point,
                    const geo::Coordinate &middle_of_text,
                    const int halignvalue,
                    const int valignvalue,
                    const geo::MText::LineSpacingStyle lineSpacingStyle,
                    const double lineSpacingFactor,
                    const std::string &text_value,
                    const std::string &style,
                    const double angle,
                    const Layer_CSPtr layer))
            .endClass()
            .beginExtendClass<DimLinear, entity::CADEntity>("DimLinear")
            .addConstructor(LUA_SP(DimLinear_SPtr), LUA_ARGS(
                    const Dimension& dimension,
                    const geo::Coordinate& extension_point1,
                    const geo::Coordinate& extension_point2,
                    const double oblique,
                    const Layer_CSPtr layer))
            .endClass()
 */
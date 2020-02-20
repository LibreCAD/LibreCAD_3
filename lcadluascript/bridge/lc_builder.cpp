#include <cad/builders/layer.h>
#include <cad/builders/cadentity.h>
#include <cad/builders/arc.h>
#include <cad/builders/circle.h>
#include <cad/builders/dimension.h>
#include <cad/builders/dimaligned.h>
#include <cad/builders/point.h>
#include <cad/builders/dimangular.h>
#include <cad/builders/dimdiametric.h>
#include <cad/builders/dimradial.h>
#include <cad/builders/ellipse.h>
#include <cad/builders/line.h>
#include <cad/builders/spline.h>
#include <cad/builders/insert.h>
#include <cad/primitive/insert.h>
#include "lc_builder.h"

void import_lc_builder_namespace(kaguya::State& state) {
    state["lc"]["builder"] = kaguya::NewTable();

    state["lc"]["builder"]["LinePatternBuilder"].setClass(kaguya::UserdataMetatable<lc::builder::LinePatternBuilder>()
        .setConstructors<lc::builder::LinePatternBuilder()>()
        .addFunction("addElement", &lc::builder::LinePatternBuilder::addElement)
        .addFunction("build", &lc::builder::LinePatternBuilder::build)
        .addFunction("checkValues", &lc::builder::LinePatternBuilder::checkValues)
        .addFunction("description", &lc::builder::LinePatternBuilder::description)
        .addFunction("name", &lc::builder::LinePatternBuilder::name)
        .addFunction("path", &lc::builder::LinePatternBuilder::path)
        .addFunction("setDescription", &lc::builder::LinePatternBuilder::setDescription)
        .addFunction("setName", &lc::builder::LinePatternBuilder::setName)
        .addFunction("setPath", &lc::builder::LinePatternBuilder::setPath)
    );



    state["lc"]["builder"]["LayerBuilder"].setClass(kaguya::UserdataMetatable<lc::builder::LayerBuilder>()
        .setConstructors<lc::builder::LayerBuilder()>()
        .addFunction("build", &lc::builder::LayerBuilder::build)
        .addFunction("color", &lc::builder::LayerBuilder::color)
        .addFunction("isFrozen", &lc::builder::LayerBuilder::isFrozen)
        .addFunction("linePattern", &lc::builder::LayerBuilder::linePattern)
        .addFunction("lineWidth", &lc::builder::LayerBuilder::lineWidth)
        .addFunction("name", &lc::builder::LayerBuilder::name)
        .addFunction("setColor", &lc::builder::LayerBuilder::setColor)
        .addFunction("setIsFrozen", &lc::builder::LayerBuilder::setIsFrozen)
        .addFunction("setLinePattern", &lc::builder::LayerBuilder::setLinePattern)
        .addFunction("setLineWidth", &lc::builder::LayerBuilder::setLineWidth)
        .addFunction("setName", &lc::builder::LayerBuilder::setName)
    );



    state["lc"]["builder"]["CADEntityBuilder"].setClass(kaguya::UserdataMetatable<lc::builder::CADEntityBuilder>()
        .setConstructors<lc::builder::CADEntityBuilder()>()
        .addFunction("block", &lc::builder::CADEntityBuilder::block)
        .addFunction("checkValues", &lc::builder::CADEntityBuilder::checkValues)
        .addFunction("id", &lc::builder::CADEntityBuilder::id)
        .addFunction("layer", &lc::builder::CADEntityBuilder::layer)
        .addFunction("metaInfo", &lc::builder::CADEntityBuilder::metaInfo)
        .addFunction("newID", &lc::builder::CADEntityBuilder::newID)
        .addOverloadedFunctions("setBlock", static_cast<void(lc::builder::CADEntityBuilder::*)(const lc::meta::Block_CSPtr &)>(&lc::builder::CADEntityBuilder::setBlock))
        .addFunction("setID", &lc::builder::CADEntityBuilder::setID)
        .addOverloadedFunctions("setLayer", static_cast<void(lc::builder::CADEntityBuilder::*)(const lc::meta::Layer_CSPtr &)>(&lc::builder::CADEntityBuilder::setLayer))
        .addOverloadedFunctions("setMetaInfo", static_cast<void(lc::builder::CADEntityBuilder::*)(const lc::meta::MetaInfo_CSPtr &)>(&lc::builder::CADEntityBuilder::setMetaInfo))
    );

    state["lc"]["builder"]["ArcBuilder"].setClass(kaguya::UserdataMetatable<lc::builder::ArcBuilder, lc::builder::CADEntityBuilder>()
        .setConstructors<lc::builder::ArcBuilder()>()
        .addFunction("build", &lc::builder::ArcBuilder::build)
        .addFunction("center", &lc::builder::ArcBuilder::center)
        .addFunction("endAngle", &lc::builder::ArcBuilder::endAngle)
        .addFunction("isCCW", &lc::builder::ArcBuilder::isCCW)
        .addFunction("radius", &lc::builder::ArcBuilder::radius)
        .addFunction("setCenter", &lc::builder::ArcBuilder::setCenter)
        .addFunction("setEndAngle", &lc::builder::ArcBuilder::setEndAngle)
        .addFunction("setIsCCW", &lc::builder::ArcBuilder::setIsCCW)
        .addFunction("setRadius", &lc::builder::ArcBuilder::setRadius)
        .addFunction("setStartAngle", &lc::builder::ArcBuilder::setStartAngle)
        .addFunction("startAngle", &lc::builder::ArcBuilder::startAngle)
    );



    state["lc"]["builder"]["CircleBuilder"].setClass(kaguya::UserdataMetatable<lc::builder::CircleBuilder, lc::builder::CADEntityBuilder>()
        .setConstructors<lc::builder::CircleBuilder()>()
        .addFunction("build", &lc::builder::CircleBuilder::build)
        .addFunction("center", &lc::builder::CircleBuilder::center)
        .addFunction("radius", &lc::builder::CircleBuilder::radius)
        .addFunction("setCenter", &lc::builder::CircleBuilder::setCenter)
        .addFunction("setRadius", &lc::builder::CircleBuilder::setRadius)
        .addFunction("threeTanConstructor", &lc::builder::CircleBuilder::threeTanConstructor)
        .addFunction("twoTanConstructor", &lc::builder::CircleBuilder::twoTanConstructor)
        .addFunction("twoTanCircleCenters", &lc::builder::CircleBuilder::twoTanCircleCenters)
    );



    state["lc"]["builder"]["DimensionBuilder"].setClass(kaguya::UserdataMetatable<lc::builder::DimensionBuilder, lc::builder::CADEntityBuilder>()
        .addFunction("attachmentPoint", &lc::builder::DimensionBuilder::attachmentPoint)
        .addFunction("definitionPoint", &lc::builder::DimensionBuilder::definitionPoint)
        .addFunction("explicitValue", &lc::builder::DimensionBuilder::explicitValue)
        .addFunction("lineSpacingFactor", &lc::builder::DimensionBuilder::lineSpacingFactor)
        .addFunction("lineSpacingStyle", &lc::builder::DimensionBuilder::lineSpacingStyle)
        .addFunction("middleOfText", &lc::builder::DimensionBuilder::middleOfText)
        .addFunction("setAttachmentPoint", &lc::builder::DimensionBuilder::setAttachmentPoint)
        .addFunction("setDefinitionPoint", &lc::builder::DimensionBuilder::setDefinitionPoint)
        .addFunction("setExplicitValue", &lc::builder::DimensionBuilder::setExplicitValue)
        .addFunction("setLineSpacingFactor", &lc::builder::DimensionBuilder::setLineSpacingFactor)
        .addFunction("setLineSpacingStyle", &lc::builder::DimensionBuilder::setLineSpacingStyle)
        .addFunction("setMiddleOfText", &lc::builder::DimensionBuilder::setMiddleOfText)
        .addFunction("setTextAngle", &lc::builder::DimensionBuilder::setTextAngle)
        .addFunction("textAngle", &lc::builder::DimensionBuilder::textAngle)
    );



    state["lc"]["builder"]["PointBuilder"].setClass(kaguya::UserdataMetatable<lc::builder::PointBuilder, lc::builder::CADEntityBuilder>()
        .setConstructors<lc::builder::PointBuilder()>()
        .addFunction("build", &lc::builder::PointBuilder::build)
        .addFunction("coordinate", &lc::builder::PointBuilder::coordinate)
        .addFunction("setCoordinate", &lc::builder::PointBuilder::setCoordinate)
    );




    state["lc"]["builder"]["DimAlignedBuilder"].setClass(kaguya::UserdataMetatable<lc::builder::DimAlignedBuilder, lc::builder::DimensionBuilder>()
        .setConstructors<lc::builder::DimAlignedBuilder()>()
        .addFunction("build", &lc::builder::DimAlignedBuilder::build)
        .addFunction("definitionPoint2", &lc::builder::DimAlignedBuilder::definitionPoint2)
        .addFunction("definitionPoint3", &lc::builder::DimAlignedBuilder::definitionPoint3)
        .addFunction("dimAuto", &lc::builder::DimAlignedBuilder::dimAuto)
        .addFunction("setDefinitionPoint2", &lc::builder::DimAlignedBuilder::setDefinitionPoint2)
        .addFunction("setDefinitionPoint3", &lc::builder::DimAlignedBuilder::setDefinitionPoint3)
    );

    state["lc"]["builder"]["DimAngularBuilder"].setClass(kaguya::UserdataMetatable<lc::builder::DimAngularBuilder, lc::builder::DimensionBuilder>()
        .setConstructors<lc::builder::DimAngularBuilder()>()
        .addFunction("build", &lc::builder::DimAngularBuilder::build)
        .addFunction("defLine11", &lc::builder::DimAngularBuilder::defLine11)
        .addFunction("defLine12", &lc::builder::DimAngularBuilder::defLine12)
        .addFunction("defLine21", &lc::builder::DimAngularBuilder::defLine21)
        .addFunction("defLine22", &lc::builder::DimAngularBuilder::defLine22)
        .addFunction("dimAuto", &lc::builder::DimAngularBuilder::dimAuto)
        .addFunction("setDefLine11", &lc::builder::DimAngularBuilder::setDefLine11)
        .addFunction("setDefLine12", &lc::builder::DimAngularBuilder::setDefLine12)
        .addFunction("setDefLine21", &lc::builder::DimAngularBuilder::setDefLine21)
        .addFunction("setDefLine22", &lc::builder::DimAngularBuilder::setDefLine22)
    );



    state["lc"]["builder"]["DimDiametricBuilder"].setClass(kaguya::UserdataMetatable<lc::builder::DimDiametricBuilder, lc::builder::DimensionBuilder>()
        .setConstructors<lc::builder::DimDiametricBuilder()>()
        .addFunction("build", &lc::builder::DimDiametricBuilder::build)
        .addFunction("definitionPoint2", &lc::builder::DimDiametricBuilder::definitionPoint2)
        .addFunction("leader", &lc::builder::DimDiametricBuilder::leader)
        .addFunction("setDefinitionPoint2", &lc::builder::DimDiametricBuilder::setDefinitionPoint2)
        .addFunction("setLeader", &lc::builder::DimDiametricBuilder::setLeader)
    );



    state["lc"]["builder"]["DimLinearBuilder"].setClass(kaguya::UserdataMetatable<lc::builder::DimLinearBuilder, lc::builder::DimensionBuilder>()
        .setConstructors<lc::builder::DimLinearBuilder()>()
        .addFunction("angle", &lc::builder::DimLinearBuilder::angle)
        .addFunction("build", &lc::builder::DimLinearBuilder::build)
        .addFunction("definitionPoint2", &lc::builder::DimLinearBuilder::definitionPoint2)
        .addFunction("definitionPoint3", &lc::builder::DimLinearBuilder::definitionPoint3)
        .addFunction("oblique", &lc::builder::DimLinearBuilder::oblique)
        .addFunction("setAngle", &lc::builder::DimLinearBuilder::setAngle)
        .addFunction("setDefinitionPoint2", &lc::builder::DimLinearBuilder::setDefinitionPoint2)
        .addFunction("setDefinitionPoint3", &lc::builder::DimLinearBuilder::setDefinitionPoint3)
        .addFunction("setOblique", &lc::builder::DimLinearBuilder::setOblique)
    );




    state["lc"]["builder"]["DimRadialBuilder"].setClass(kaguya::UserdataMetatable<lc::builder::DimRadialBuilder, lc::builder::DimensionBuilder>()
        .setConstructors<lc::builder::DimRadialBuilder()>()
        .addFunction("build", &lc::builder::DimRadialBuilder::build)
        .addFunction("definitionPoint2", &lc::builder::DimRadialBuilder::definitionPoint2)
        .addFunction("dimAuto", &lc::builder::DimRadialBuilder::dimAuto)
        .addFunction("leader", &lc::builder::DimRadialBuilder::leader)
        .addFunction("setDefinitionPoint2", &lc::builder::DimRadialBuilder::setDefinitionPoint2)
        .addFunction("setLeader", &lc::builder::DimRadialBuilder::setLeader)
    );





    state["lc"]["builder"]["EllipseBuilder"].setClass(kaguya::UserdataMetatable<lc::builder::EllipseBuilder, lc::builder::CADEntityBuilder>()
        .setConstructors<lc::builder::EllipseBuilder()>()
        .addFunction("build", &lc::builder::EllipseBuilder::build)
        .addFunction("center", &lc::builder::EllipseBuilder::center)
        .addFunction("endAngle", &lc::builder::EllipseBuilder::endAngle)
        .addFunction("isReversed", &lc::builder::EllipseBuilder::isReversed)
        .addFunction("majorPoint", &lc::builder::EllipseBuilder::majorPoint)
        .addFunction("minorRadius", &lc::builder::EllipseBuilder::minorRadius)
        .addFunction("setCenter", &lc::builder::EllipseBuilder::setCenter)
        .addFunction("setEndAngle", &lc::builder::EllipseBuilder::setEndAngle)
        .addFunction("setIsReversed", &lc::builder::EllipseBuilder::setIsReversed)
        .addFunction("setMajorPoint", &lc::builder::EllipseBuilder::setMajorPoint)
        .addFunction("setMinorRadius", &lc::builder::EllipseBuilder::setMinorRadius)
        .addFunction("setStartAngle", &lc::builder::EllipseBuilder::setStartAngle)
        .addFunction("startAngle", &lc::builder::EllipseBuilder::startAngle)
    );



    state["lc"]["builder"]["LineBuilder"].setClass(kaguya::UserdataMetatable<lc::builder::LineBuilder, lc::builder::CADEntityBuilder>()
        .setConstructors<lc::builder::LineBuilder()>()
        .addFunction("build", &lc::builder::LineBuilder::build)
        .addFunction("endPoint", &lc::builder::LineBuilder::end)
        .addFunction("setEndPoint", &lc::builder::LineBuilder::setEnd)
        .addFunction("setStartPoint", &lc::builder::LineBuilder::setStart)
        .addFunction("startPoint", &lc::builder::LineBuilder::start)
    );



    state["lc"]["builder"]["SplineBuilder"].setClass(kaguya::UserdataMetatable<lc::builder::SplineBuilder, lc::builder::CADEntityBuilder>()
        .setConstructors<lc::builder::SplineBuilder()>()
        .addFunction("addControlPoint", &lc::builder::SplineBuilder::addControlPoint)
        .addFunction("addFitPoint", &lc::builder::SplineBuilder::addFitPoint)
        .addFunction("addKnotPoint", &lc::builder::SplineBuilder::addKnotPoint)
        .addFunction("build", &lc::builder::SplineBuilder::build)
        .addFunction("closed", &lc::builder::SplineBuilder::closed)
        .addFunction("controlPoints", &lc::builder::SplineBuilder::controlPoints)
        .addFunction("degree", &lc::builder::SplineBuilder::degree)
        .addFunction("endTangent", &lc::builder::SplineBuilder::endTangent)
        .addFunction("fitPoints", &lc::builder::SplineBuilder::fitPoints)
        .addFunction("fitTolerance", &lc::builder::SplineBuilder::fitTolerance)
        .addFunction("flags", &lc::builder::SplineBuilder::flags)
        .addFunction("knotPoints", &lc::builder::SplineBuilder::knotPoints)
        .addFunction("normalVector", &lc::builder::SplineBuilder::normalVector)
        .addFunction("removeControlPoint", &lc::builder::SplineBuilder::removeControlPoint)
        .addFunction("removeFitPoint", &lc::builder::SplineBuilder::removeFitPoint)
        .addFunction("removeKnotPoint", &lc::builder::SplineBuilder::removeKnotPoint)
        .addFunction("setClosed", &lc::builder::SplineBuilder::setClosed)
        .addFunction("setControlPoints", &lc::builder::SplineBuilder::setControlPoints)
        .addFunction("setDegree", &lc::builder::SplineBuilder::setDegree)
        .addFunction("setEndTangent", &lc::builder::SplineBuilder::setEndTangent)
        .addFunction("setFitPoints", &lc::builder::SplineBuilder::setFitPoints)
        .addFunction("setFitTolerance", &lc::builder::SplineBuilder::setFitTolerance)
        .addFunction("setFlags", &lc::builder::SplineBuilder::setFlags)
        .addFunction("setKnotPoints", &lc::builder::SplineBuilder::setKnotPoints)
        .addFunction("setNormalVector", &lc::builder::SplineBuilder::setNormalVector)
        .addFunction("setStartTangent", &lc::builder::SplineBuilder::setStartTangent)
        .addFunction("startTangent", &lc::builder::SplineBuilder::startTangent)
    );

	state["lc"]["builder"]["LWPolylineBuilder"].setClass(kaguya::UserdataMetatable<lc::builder::LWPolylineBuilder, lc::builder::CADEntityBuilder>()
		.setConstructors<lc::builder::LWPolylineBuilder()>()
		.addFunction("addLineVertex", &lc::builder::LWPolylineBuilder::addLineVertex)
		.addFunction("addArcVertex", &lc::builder::LWPolylineBuilder::addArcVertex)
		.addFunction("modifyLastVertex", &lc::builder::LWPolylineBuilder::modifyLastVertex)
		.addFunction("modifyLastVertexArc", &lc::builder::LWPolylineBuilder::modifyLastVertexArc)
		.addFunction("modifyLastVertexLine", &lc::builder::LWPolylineBuilder::modifyLastVertexLine)
		.addFunction("removeVertex", &lc::builder::LWPolylineBuilder::removeVertex)
		.addFunction("getVertices", &lc::builder::LWPolylineBuilder::getVertices)
		.addFunction("build", &lc::builder::LWPolylineBuilder::build)
	);

    state["lc"]["builder"]["InsertBuilder"].setClass(kaguya::UserdataMetatable<lc::builder::InsertBuilder, lc::builder::CADEntityBuilder>()
        .setConstructors<lc::builder::InsertBuilder()>()
        .addFunction("build", &lc::builder::InsertBuilder::build)
        .addFunction("checkValues", &lc::builder::InsertBuilder::checkValues)
        .addFunction("coordinate", &lc::builder::InsertBuilder::coordinate)
        .addFunction("displayBlock", &lc::builder::InsertBuilder::displayBlock)
        .addFunction("document", &lc::builder::InsertBuilder::document)
        .addFunction("setCoordinate", &lc::builder::InsertBuilder::setCoordinate)
        .addFunction("setDisplayBlock", &lc::builder::InsertBuilder::setDisplayBlock)
        .addFunction("setDocument", &lc::builder::InsertBuilder::setDocument)
    );
}

#include <cad/meta/color.h>
#include <cad/base/visitor.h>
#include <cad/interface/entitydispatch.h>
#include <cad/vo/entitycoordinate.h>
#include <cad/interface/snapconstrain.h>
#include <cad/vo/entitydistance.h>
#include "lc.h"

void import_lc_namespace(kaguya::State& state) {
    state["lc"] = kaguya::NewTable();

    state["lc"]["Visitable"].setClass(kaguya::UserdataMetatable<lc::Visitable>()
                                      .addFunction("accept", &lc::Visitable::accept)
                                     );

    state["lc"]["Color"].setClass(kaguya::UserdataMetatable<lc::Color>()
                                  .setConstructors<lc::Color(), lc::Color(int, int, int, int), lc::Color(double, double, double, double), lc::Color(const lc::Color &)>()
                                  .addFunction("alpha", &lc::Color::alpha)
                                  .addFunction("alphaI", &lc::Color::alphaI)
                                  .addFunction("blue", &lc::Color::blue)
                                  .addFunction("blueI", &lc::Color::blueI)
                                  .addFunction("green", &lc::Color::green)
                                  .addFunction("greenI", &lc::Color::greenI)
                                  .addFunction("red", &lc::Color::red)
                                  .addFunction("redI", &lc::Color::redI)
                                 );

    state["lc"]["EntityDispatch"].setClass(kaguya::UserdataMetatable<lc::EntityDispatch>()
                                           .addOverloadedFunctions("visit", static_cast<void(lc::EntityDispatch::*)(lc::entity::Line_CSPtr)>(&lc::EntityDispatch::visit), static_cast<void(lc::EntityDispatch::*)(lc::entity::Point_CSPtr)>(&lc::EntityDispatch::visit), static_cast<void(lc::EntityDispatch::*)(lc::entity::Circle_CSPtr)>(&lc::EntityDispatch::visit), static_cast<void(lc::EntityDispatch::*)(lc::entity::Arc_CSPtr)>(&lc::EntityDispatch::visit), static_cast<void(lc::EntityDispatch::*)(lc::entity::Ellipse_CSPtr)>(&lc::EntityDispatch::visit), static_cast<void(lc::EntityDispatch::*)(lc::entity::Text_CSPtr)>(&lc::EntityDispatch::visit), static_cast<void(lc::EntityDispatch::*)(lc::entity::Spline_CSPtr)>(&lc::EntityDispatch::visit), static_cast<void(lc::EntityDispatch::*)(lc::entity::DimAligned_CSPtr)>(&lc::EntityDispatch::visit), static_cast<void(lc::EntityDispatch::*)(lc::entity::DimAngular_CSPtr)>(&lc::EntityDispatch::visit), static_cast<void(lc::EntityDispatch::*)(lc::entity::DimDiametric_CSPtr)>(&lc::EntityDispatch::visit), static_cast<void(lc::EntityDispatch::*)(lc::entity::DimLinear_CSPtr)>(&lc::EntityDispatch::visit), static_cast<void(lc::EntityDispatch::*)(lc::entity::DimRadial_CSPtr)>(&lc::EntityDispatch::visit), static_cast<void(lc::EntityDispatch::*)(lc::entity::LWPolyline_CSPtr)>(&lc::EntityDispatch::visit), static_cast<void(lc::EntityDispatch::*)(lc::entity::Image_CSPtr)>(&lc::EntityDispatch::visit))
                                          );

    state["lc"]["EntityCoordinate"].setClass(kaguya::UserdataMetatable<lc::EntityCoordinate>()
            .setConstructors<lc::EntityCoordinate(const lc::geo::Coordinate &, int), lc::EntityCoordinate(const lc::EntityCoordinate &)>()
            .addFunction("coordinate", &lc::EntityCoordinate::coordinate)
            .addFunction("pointId", &lc::EntityCoordinate::pointId)
                                            );

    state["lc"]["SimpleSnapConstrain"].setClass(kaguya::UserdataMetatable<lc::SimpleSnapConstrain>()
            .setConstructors<lc::SimpleSnapConstrain(), lc::SimpleSnapConstrain(uint16_t, int, double)>()
            .addFunction("angle", &lc::SimpleSnapConstrain::angle)
            .addFunction("constrain", &lc::SimpleSnapConstrain::constrain)
            .addFunction("disableConstrain", &lc::SimpleSnapConstrain::disableConstrain)
            .addFunction("divisions", &lc::SimpleSnapConstrain::divisions)
            .addFunction("enableConstrain", &lc::SimpleSnapConstrain::enableConstrain)
            .addFunction("hasConstrain", &lc::SimpleSnapConstrain::hasConstrain)
            .addFunction("setAngle", &lc::SimpleSnapConstrain::setAngle)
            .addFunction("setDivisions", &lc::SimpleSnapConstrain::setDivisions)
                                               );

    state["lc"]["EntityDistance"].setClass(kaguya::UserdataMetatable<lc::EntityDistance>()
                                           .setConstructors<lc::EntityDistance(lc::entity::CADEntity_CSPtr, const lc::geo::Coordinate &)>()
                                           .addFunction("coordinate", &lc::EntityDistance::coordinate)
                                           .addFunction("entity", &lc::EntityDistance::entity)
                                          );
}

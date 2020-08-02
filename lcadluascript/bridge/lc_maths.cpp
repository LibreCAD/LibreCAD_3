#include <cad/math/equation.h>
#include <cad/math/intersect.h>
#include "lc_maths.h"

void import_lc_maths_namespace(kaguya::State& state) {
    state["lc"]["maths"] = kaguya::NewTable();

    state["lc"]["maths"]["Equation"].setClass(kaguya::UserdataMetatable<lc::maths::Equation>()
        .setConstructors<lc::maths::Equation(), lc::maths::Equation(double, double, double, double, double, double), lc::maths::Equation(const std::vector<double> &)>()
        .addFunction("Coefficients", &lc::maths::Equation::Coefficients)
        .addFunction("Matrix", &lc::maths::Equation::Matrix)
        .addFunction("flipXY", &lc::maths::Equation::flipXY)
        .addFunction("move", &lc::maths::Equation::move)
        .addOverloadedFunctions("rotate", static_cast<const lc::maths::Equation(lc::maths::Equation::*)(double) const>(&lc::maths::Equation::rotate), static_cast<const lc::maths::Equation(lc::maths::Equation::*)(const lc::geo::Coordinate &, double) const>(&lc::maths::Equation::rotate))
        .addStaticFunction("rotationMatrix", &lc::maths::Equation::rotationMatrix)
        .addStaticFunction("translateMatrix", &lc::maths::Equation::translateMatrix)
    );

    state["lc"]["maths"]["IntersectMany"].setClass(kaguya::UserdataMetatable<lc::maths::IntersectMany>()
        .setConstructors<
        	lc::maths::IntersectMany(std::vector<lc::entity::CADEntity_CSPtr>, lc::maths::Intersect::Method, double),
        	lc::maths::IntersectMany(std::vector<lc::entity::CADEntity_CSPtr>)
        	>()
        .addFunction("result", &lc::maths::IntersectMany::result)
    );

    state["lc"]["maths"]["IntersectAgainstOthers"].setClass(kaguya::UserdataMetatable<lc::maths::IntersectAgainstOthers>()
        .setConstructors<lc::maths::IntersectAgainstOthers(std::vector<lc::entity::CADEntity_CSPtr>, std::vector<lc::entity::CADEntity_CSPtr>, lc::maths::Intersect::Method, double)>()
        .addFunction("result", &lc::maths::IntersectAgainstOthers::result)
    );
        }

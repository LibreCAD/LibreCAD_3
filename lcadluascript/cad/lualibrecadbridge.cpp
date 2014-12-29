extern "C"
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

#include <cad/operations/layerops.h>
#include "lualibrecadbridge.h"

#include "lua-intf/LuaIntf/LuaIntf.h"

#include "cad/geometry/geocoordinate.h"
#include "cad/primitive/line.h"
#include "cad/primitive/circle.h"
#include "cad/primitive/arc.h"
#include "cad/primitive/ellipse.h"
#include "cad/primitive/text.h"
#include "cad/meta/color.h"
#include "cad/base/metainfo.h"
#include "cad/operations/builder.h"
#include "cad/operations/builderops.h"
#include "cad/dochelpers/documentimpl.h"
#include "cad/operations/documentoperation.h"
#include "cad/dochelpers/storagemanagerimpl.h"

namespace LuaIntf {
    LUA_USING_SHARED_PTR_TYPE(std::shared_ptr)
}


using namespace LuaIntf;
using namespace lc;

void lua_openlckernel(lua_State *L) {

    LuaBinding(L)

            .beginClass<Color>("Color")
            .addConstructor(LUA_ARGS(double r, double g, double b, double a))
            .endClass()

            .beginClass<MetaType>("MetaType")
            .addConstructor(LUA_SP(MetaType_SPtr), LUA_ARGS())
            .endClass()

            .beginExtendClass<Layer, MetaType>("Layer")
            .addConstructor(LUA_SP(Layer_SPtr), LUA_ARGS(const std::string, const Color))
            .endClass()

            .beginExtendClass<MetaColor, MetaType>("MetaColor")
            .addConstructor(LUA_SP(MetaColor_SPtr), LUA_ARGS(const Color))
            .endClass()

            .beginClass<MetaInfo>("MetaInfo")
            .addConstructor(LUA_SP(std::shared_ptr<lc::MetaInfo>), LUA_ARGS())
            .addFunction("add", &lc::MetaInfo::add)
            .endClass()


            .beginClass<geo::Coordinate>("Coord")
            .addConstructor(LUA_ARGS(double x, double y))
            .addFunction("x", &geo::Coordinate::x)
            .addFunction("y", &geo::Coordinate::y)
            .addFunction("z", &geo::Coordinate::z)
            .endClass()
            .beginClass<geo::Coordinate>("Coordinate")
            .addConstructor(LUA_ARGS(double x, double y))
            .addFunction("x", &geo::Coordinate::x)
            .addFunction("y", &geo::Coordinate::y)
            .addFunction("z", &geo::Coordinate::z)
            .endClass()
            .beginClass<geo::Vector>("Vector")
            .addConstructor(LUA_SP(std::shared_ptr<const geo::Vector>), LUA_ARGS(const geo::Coordinate &start, const geo::Coordinate &end))
            .endClass()



            .beginClass<Document>("Document")
            .endClass()
            .beginExtendClass<DocumentImpl, Document>("DocumentImpl")
            .addFunction("entitiesByLayer", &DocumentImpl::entitiesByLayer)
            .endClass()
            .beginClass<EntityContainer<lc::CADEntity_CSPtr>>("EntityContainer")
            .endClass()

            .beginClass<StorageManager>("StorageManager")
            .endClass()
            .beginExtendClass<StorageManagerImpl, StorageManager>("StorageManagerImpl")
            .addConstructor(LUA_SP(std::shared_ptr<StorageManagerImpl>), LUA_ARGS())
            .endClass()

            .beginClass<ID>("ID")
            .addFunction("id", &ID::id)
            .endClass()
            .beginExtendClass<CADEntity, ID>("CADEntity")
            .endClass()
            .beginExtendClass<Line, CADEntity>("Line")
            .addConstructor(LUA_SP(Line_SPtr), LUA_ARGS(const geo::Coordinate &start, const geo::Coordinate &end, const std::shared_ptr<Layer>))
            .endClass()
            .beginExtendClass<Circle, CADEntity>("Circle")
            .addConstructor(LUA_SP(Circle_SPtr), LUA_ARGS(const geo::Coordinate &center, double radius, const std::shared_ptr<Layer>))
            .endClass()
            .beginExtendClass<Arc, CADEntity>("Arc")
            .addConstructor(LUA_SP(Arc_SPtr), LUA_ARGS(const geo::Coordinate &center, double radius, const double startAngle, const double endAngle, const std::shared_ptr<Layer> layer))
            .endClass()
            .beginExtendClass<Ellipse, CADEntity>("Ellipse")
            .addConstructor(LUA_SP(Ellipse_SPtr), LUA_ARGS(const geo::Coordinate &center, const geo::Coordinate &majorP, double minorRadius, double startAngle, double endAngle, const Layer_CSPtr layer))
            .endClass()
            .beginExtendClass<Text, CADEntity>("Text")
            .addConstructor(LUA_SP(Text_SPtr), LUA_ARGS(const geo::Coordinate &insertion_point, const geo::Coordinate &second_point, const double height, const std::string text_value, const double width_rel,
                    const double angle, const std::string style, const int textgenvalue,
                    const int halignvalue, const int valignvalue, const Layer_CSPtr layer))
            .endClass()

            .beginClass<operation::DocumentOperation>("DocumentOperation")
            .addFunction("execute", &operation::DocumentOperation::execute)
            .endClass()
            .beginExtendClass<lc::operation::Builder, operation::DocumentOperation>("Builder")
            .addConstructor(LUA_SP(std::shared_ptr<lc::operation::Builder>), LUA_ARGS(Document *doc))
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
            .addConstructor(LUA_SP(std::shared_ptr<operation::Move>), LUA_ARGS(const geo::Coordinate &offset))
            .endClass()
            .beginExtendClass<operation::Begin, operation::Base>("Begin")
            .addConstructor(LUA_SP(std::shared_ptr<operation::Begin>), LUA_ARGS())
            .endClass()
            .beginExtendClass<operation::Loop, operation::Base>("Loop")
            .addConstructor(LUA_SP(std::shared_ptr<operation::Loop>), LUA_ARGS(const int numTimes))
            .endClass()
            .beginExtendClass<operation::Copy, operation::Base>("Copy")
            .addConstructor(LUA_SP(std::shared_ptr<operation::Copy>), LUA_ARGS(const geo::Coordinate &offset))
            .endClass()
            .beginExtendClass<operation::Scale, operation::Base>("Scale")
            .addConstructor(LUA_SP(std::shared_ptr<operation::Scale>), LUA_ARGS(const geo::Coordinate &scale_center, const geo::Coordinate &scale_factor))
            .endClass()
            .beginExtendClass<operation::Push, operation::Base>("Push")
            .addConstructor(LUA_SP(std::shared_ptr<operation::Push>), LUA_ARGS())
            .endClass()
            .beginExtendClass<operation::SelectByLayer, operation::Base>("SelectByLayer")
            .addConstructor(LUA_SP(std::shared_ptr<operation::SelectByLayer>), LUA_ARGS(const std::shared_ptr<Layer>))
            .endClass()

            .beginExtendClass<operation::AddLayer, operation::DocumentOperation>("AddLayer")
            .addConstructor(LUA_SP(std::shared_ptr<lc::operation::AddLayer>), LUA_ARGS(Document *doc, const std::shared_ptr<Layer>))
            .endClass()
            .beginExtendClass<operation::RemoveLayer, operation::DocumentOperation>("RemoveLayer")
            .addConstructor(LUA_SP(std::shared_ptr<lc::operation::RemoveLayer>), LUA_ARGS(Document *doc, const std::shared_ptr<Layer>))
            .endClass()
            .beginExtendClass<operation::ReplaceLayer, operation::DocumentOperation>("ReplaceLayer")
            .addConstructor(LUA_SP(std::shared_ptr<lc::operation::ReplaceLayer>), LUA_ARGS(Document *doc, const std::shared_ptr<Layer>, const std::shared_ptr<Layer>))
            .endClass();

/*
        .beginClass<StorageManager>("StorageManager")
                .endClass()
                .beginExtendClass<StorageManagerImpl, StorageManager>("StorageManagerImpl")
                .addConstructor(LUA_SP(std::shared_ptr<StorageManagerImpl>), LUA_ARGS())
                .endClass() */
}

// .addConstructor(LUA_SP(std::shared_ptr<const CreateEntities>), LUA_ARGS(Document * doc))



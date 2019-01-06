#include <cad/interface/metatype.h>
#include <cad/meta/viewport.h>
#include <cad/meta/block.h>
#include <cad/meta/metalinewidth.h>
#include <cad/base/metainfo.h>
#include <cad/meta/layer.h>
#include <cad/meta/customentitystorage.h>
#include "lc_meta.h"

void import_lc_meta_namespace(kaguya::State& state) {
    state["lc"]["meta"] = kaguya::NewTable();

    state["lc"]["meta"]["MetaType"].setClass(kaguya::UserdataMetatable<lc::meta::MetaType>()
        .addFunction("id", &lc::meta::MetaType::id)
    );

    state["lc"]["meta"]["DocumentMetaType"].setClass(kaguya::UserdataMetatable<lc::meta::DocumentMetaType, lc::meta::MetaType>()
        .addFunction("name", &lc::meta::DocumentMetaType::name)
    );

    state["lc"]["meta"]["Viewport"].setClass(kaguya::UserdataMetatable<lc::meta::Viewport, lc::meta::DocumentMetaType>()
        .addStaticFunction("new", [](const std::string& name) {
            return std::make_shared<const lc::meta::Viewport>(name);
        })
    );

    state["lc"]["meta"]["EntityMetaType"].setClass(kaguya::UserdataMetatable<lc::meta::EntityMetaType, lc::meta::MetaType>()
        .addFunction("metaTypeID", &lc::meta::EntityMetaType::metaTypeID)
    );

    state["lc"]["meta"]["DxfLinePattern"].setClass(kaguya::UserdataMetatable<lc::meta::DxfLinePattern, kaguya::MultipleBase<lc::meta::DocumentMetaType, lc::meta::EntityMetaType>>()
        .addStaticFunction("LCMETANAME", &lc::meta::DxfLinePattern::LCMETANAME)
        .addFunction("description", &lc::meta::DxfLinePattern::description)
        .addFunction("id", &lc::meta::DxfLinePattern::id)
        .addFunction("metaTypeID", &lc::meta::DxfLinePattern::metaTypeID)
    );

    state["lc"]["meta"]["DxfLinePatternByValue"].setClass(kaguya::UserdataMetatable<lc::meta::DxfLinePatternByValue, lc::meta::DxfLinePattern>()
        .setConstructors<lc::meta::DxfLinePatternByValue(std::string, std::string, std::vector<double>, double)>()
        .addStaticFunction("calculatePathLength", &lc::meta::DxfLinePatternByValue::calculatePathLength)
        .addFunction("description", &lc::meta::DxfLinePatternByValue::description)
        .addFunction("generatePattern", &lc::meta::DxfLinePatternByValue::generatePattern)
        .addFunction("lcPattern", &lc::meta::DxfLinePatternByValue::lcPattern)
        .addFunction("length", &lc::meta::DxfLinePatternByValue::length)
        .addFunction("name", &lc::meta::DxfLinePatternByValue::name)
        .addFunction("path", &lc::meta::DxfLinePatternByValue::path)
    );

    state["lc"]["meta"]["DxfLinePatternByBlock"].setClass(kaguya::UserdataMetatable<lc::meta::DxfLinePatternByBlock, lc::meta::DxfLinePattern>()
        .addFunction("name", &lc::meta::DxfLinePatternByBlock::name)
    );

    state["lc"]["meta"]["MetaInfo"].setClass(kaguya::UserdataMetatable<lc::meta::MetaInfo>()
        .addStaticFunction("new", []() {
            return std::make_shared<lc::meta::MetaInfo>();
        })
        .addFunction("add", &lc::meta::MetaInfo::add)
        .addStaticFunction("create", &lc::meta::MetaInfo::create)
    );

    state["lc"]["meta"]["MetaColor"].setClass(kaguya::UserdataMetatable<lc::meta::MetaColor, lc::meta::EntityMetaType>()
        .addStaticFunction("LCMETANAME", &lc::meta::MetaColor::LCMETANAME)
        .addFunction("metaTypeID", &lc::meta::MetaColor::metaTypeID)
    );

    state["lc"]["meta"]["MetaColorByValue"].setClass(kaguya::UserdataMetatable<lc::meta::MetaColorByValue, lc::meta::MetaColor>()
        .setConstructors<lc::meta::MetaColorByValue(), lc::meta::MetaColorByValue(const lc::meta::MetaColorByValue &), lc::meta::MetaColorByValue(const lc::Color &), lc::meta::MetaColorByValue(double, double, double, double)>()
        .addFunction("alpha", &lc::meta::MetaColorByValue::alpha)
        .addFunction("alphaI", &lc::meta::MetaColorByValue::alphaI)
        .addFunction("blue", &lc::meta::MetaColorByValue::blue)
        .addFunction("blueI", &lc::meta::MetaColorByValue::blueI)
        .addFunction("color", &lc::meta::MetaColorByValue::color)
        .addFunction("green", &lc::meta::MetaColorByValue::green)
        .addFunction("greenI", &lc::meta::MetaColorByValue::greenI)
        .addFunction("id", &lc::meta::MetaColorByValue::id)
        .addFunction("red", &lc::meta::MetaColorByValue::red)
        .addFunction("redI", &lc::meta::MetaColorByValue::redI)
    );

    state["lc"]["meta"]["MetaColorByBlock"].setClass(kaguya::UserdataMetatable<lc::meta::MetaColorByBlock, lc::meta::MetaColor>()
        .setConstructors<lc::meta::MetaColorByBlock()>()
        .addFunction("id", &lc::meta::MetaColorByBlock::id)
    );

    state["lc"]["meta"]["MetaLineWidth"].setClass(kaguya::UserdataMetatable<lc::meta::MetaLineWidth, lc::meta::EntityMetaType>()
        .addStaticFunction("LCMETANAME", &lc::meta::MetaLineWidth::LCMETANAME)
        .addFunction("id", &lc::meta::MetaLineWidth::id)
    );

    state["lc"]["meta"]["MetaLineWidthByBlock"].setClass(kaguya::UserdataMetatable<lc::meta::MetaLineWidthByBlock, lc::meta::MetaLineWidth>()
        .setConstructors<lc::meta::MetaLineWidthByBlock()>()
        .addFunction("metaTypeID", &lc::meta::MetaLineWidthByBlock::metaTypeID)
    );

    state["lc"]["meta"]["MetaLineWidthByValue"].setClass(kaguya::UserdataMetatable<lc::meta::MetaLineWidthByValue, kaguya::MultipleBase<lc::meta::MetaLineWidth, lc::meta::DocumentMetaType>>()
        .setConstructors<lc::meta::MetaLineWidthByValue(), lc::meta::MetaLineWidthByValue(const double)>()
        .addFunction("id", &lc::meta::MetaLineWidthByValue::id)
        .addFunction("metaTypeID", &lc::meta::MetaLineWidthByValue::metaTypeID)
        .addFunction("name", &lc::meta::MetaLineWidthByValue::name)
        .addFunction("width", &lc::meta::MetaLineWidthByValue::width)
    );

    state["lc"]["meta"]["Block"].setClass(kaguya::UserdataMetatable<lc::meta::Block, lc::meta::DocumentMetaType>()
        .setConstructors<lc::meta::Block(std::string, lc::geo::Coordinate)>()
        .addFunction("base", &lc::meta::Block::base)
        .addFunction("id", &lc::meta::Block::id)
        .addFunction("name", &lc::meta::Block::name)
    );

    state["lc"]["meta"]["Layer"].setClass(kaguya::UserdataMetatable<lc::meta::Layer, kaguya::MultipleBase<lc::meta::EntityMetaType, lc::meta::DocumentMetaType>>()
        .setConstructors<lc::meta::Layer(std::string, const lc::meta::MetaLineWidthByValue &, const lc::Color &, lc::meta::DxfLinePatternByValue_CSPtr, bool)>()
        .addStaticFunction("LCMETANAME", &lc::meta::Layer::LCMETANAME)
        .addFunction("color", &lc::meta::Layer::color)
        .addFunction("id", &lc::meta::Layer::id)
        .addFunction("isFrozen", &lc::meta::Layer::isFrozen)
        .addFunction("linePattern", &lc::meta::Layer::linePattern)
        .addFunction("lineWidth", &lc::meta::Layer::lineWidth)
        .addFunction("metaTypeID", &lc::meta::Layer::metaTypeID)
        .addFunction("name", &lc::meta::Layer::name)
    );

    state["lc"]["meta"]["CustomEntityStorage"].setClass(kaguya::UserdataMetatable<lc::meta::CustomEntityStorage, lc::meta::Block>()
        .setConstructors<lc::meta::CustomEntityStorage(std::string, std::string, lc::geo::Coordinate, std::map<std::string, std::string>)>()
        .addFunction("entityName", &lc::meta::CustomEntityStorage::entityName)
        .addFunction("param", &lc::meta::CustomEntityStorage::param)
        .addFunction("params", &lc::meta::CustomEntityStorage::params)
        .addFunction("pluginName", &lc::meta::CustomEntityStorage::pluginName)
        .addFunction("setParam", &lc::meta::CustomEntityStorage::setParam)
    );
        }

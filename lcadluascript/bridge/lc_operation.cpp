#include <cad/operations/undoable.h>
#include <cad/operations/builder.h>
#include <cad/operations/documentoperation.h>
#include <cad/operations/entityops.h>
#include <cad/operations/layerops.h>
#include <cad/operations/linepatternops.h>
#include <cad/operations/blockops.h>
#include <cad/operations/entitybuilder.h>
#include "lc_operation.h"

void import_lc_operation_namespace(kaguya::State& state) {
    state["lc"]["operation"] = kaguya::NewTable();

    state["lc"]["operation"]["Undoable"].setClass(kaguya::UserdataMetatable<lc::operation::Undoable>()
        .addFunction("redo", &lc::operation::Undoable::redo)
        .addFunction("text", &lc::operation::Undoable::text)
        .addFunction("undo", &lc::operation::Undoable::undo)
    );

    state["lc"]["operation"]["DocumentOperation"].setClass(kaguya::UserdataMetatable<lc::operation::DocumentOperation, lc::operation::Undoable>()
        .addFunction("document", &lc::operation::DocumentOperation::document)
        .addFunction("execute", &lc::operation::DocumentOperation::execute)
    );
    
    state["lc"]["operation"]["Builder"].setClass(kaguya::UserdataMetatable<lc::operation::Builder, lc::operation::DocumentOperation>()
        .setConstructors<lc::operation::Builder(lc::storage::Document_SPtr, const std::string &)>()
        .addFunction("append", &lc::operation::Builder::append)
        .addFunction("redo", &lc::operation::Builder::redo)
        .addFunction("undo", &lc::operation::Builder::undo)
    );

    state["lc"]["operation"]["Base"].setClass(kaguya::UserdataMetatable<lc::operation::Base>()
        .addFunction("process", &lc::operation::Base::process)
    );

    state["lc"]["operation"]["Loop"].setClass(kaguya::UserdataMetatable<lc::operation::Loop, lc::operation::Base>()
        .addStaticFunction("new", [](int n) {
            return std::make_shared<lc::operation::Loop>(n);
        })
        .addFunction("process", &lc::operation::Loop::process)
    );

    state["lc"]["operation"]["Begin"].setClass(kaguya::UserdataMetatable<lc::operation::Begin, lc::operation::Base>()
        .addStaticFunction("new", []() {
            return std::make_shared<lc::operation::Begin>();
        })
        .addFunction("getEntities", &lc::operation::Begin::getEntities)
        .addFunction("process", &lc::operation::Begin::process)
    );

    state["lc"]["operation"]["Move"].setClass(kaguya::UserdataMetatable<lc::operation::Move, lc::operation::Base>()
        .addStaticFunction("new", [](lc::geo::Coordinate offset) {
            return std::make_shared<lc::operation::Move>(offset);
        })
        .addFunction("process", &lc::operation::Move::process)
    );

    state["lc"]["operation"]["Copy"].setClass(kaguya::UserdataMetatable<lc::operation::Copy, lc::operation::Base>()
        .addStaticFunction("new", [](lc::geo::Coordinate offset) {
            return std::make_shared<lc::operation::Copy>(offset);
        })
        .addFunction("process", &lc::operation::Copy::process)
    );

    state["lc"]["operation"]["Rotate"].setClass(kaguya::UserdataMetatable<lc::operation::Rotate, lc::operation::Base>()
        .addStaticFunction("new", [](lc::geo::Coordinate center, double angle) {
            return std::make_shared<lc::operation::Rotate>(center, angle);
        })
        .addFunction("process", &lc::operation::Rotate::process)
    );

    state["lc"]["operation"]["Scale"].setClass(kaguya::UserdataMetatable<lc::operation::Scale, lc::operation::Base>()
        .addStaticFunction("new", [](lc::geo::Coordinate center, lc::geo::Coordinate factor) {
            return std::make_shared<lc::operation::Scale>(center, factor);
        })
        .addFunction("process", &lc::operation::Scale::process)
    );

    state["lc"]["operation"]["Push"].setClass(kaguya::UserdataMetatable<lc::operation::Push, lc::operation::Base>()
        .addStaticFunction("new", []() {
            return std::make_shared<lc::operation::Push>();
        })
        .addFunction("process", &lc::operation::Push::process)
    );

    state["lc"]["operation"]["SelectByLayer"].setClass(kaguya::UserdataMetatable<lc::operation::SelectByLayer, lc::operation::Base>()
        .addStaticFunction("new", [](lc::meta::Layer_CSPtr layer) {
            return std::make_shared<lc::operation::SelectByLayer>(layer);
        })
        .addFunction("process", &lc::operation::SelectByLayer::process)
    );

    state["lc"]["operation"]["Remove"].setClass(kaguya::UserdataMetatable<lc::operation::Remove, lc::operation::Base>()
        .addStaticFunction("new", []() {
            return std::make_shared<lc::operation::Remove>();
        })
        .addFunction("process", &lc::operation::Remove::process)
    );

    state["lc"]["operation"]["AddLayer"].setClass(kaguya::UserdataMetatable<lc::operation::AddLayer, lc::operation::DocumentOperation>()
        .addStaticFunction("new", [](lc::storage::Document_SPtr document, lc::meta::Layer_CSPtr layer) {
            return std::make_shared<lc::operation::AddLayer>(document, layer);
        })
        .addFunction("redo", &lc::operation::AddLayer::redo)
        .addFunction("undo", &lc::operation::AddLayer::undo)
    );

    state["lc"]["operation"]["RemoveLayer"].setClass(kaguya::UserdataMetatable<lc::operation::RemoveLayer, lc::operation::DocumentOperation>()
        .addStaticFunction("new", [](lc::storage::Document_SPtr document, lc::meta::Layer_CSPtr layer) {
            return std::make_shared<lc::operation::RemoveLayer>(document, layer);
        })
        .addFunction("redo", &lc::operation::RemoveLayer::redo)
        .addFunction("undo", &lc::operation::RemoveLayer::undo)
    );

    state["lc"]["operation"]["ReplaceLayer"].setClass(kaguya::UserdataMetatable<lc::operation::ReplaceLayer, lc::operation::DocumentOperation>()
        .addStaticFunction("new", [](lc::storage::Document_SPtr document, lc::meta::Layer_CSPtr layer, lc::meta::Layer_CSPtr newLayer) {
            return std::make_shared<lc::operation::ReplaceLayer>(document, layer, newLayer);
        })
        .addFunction("redo", &lc::operation::ReplaceLayer::redo)
        .addFunction("undo", &lc::operation::ReplaceLayer::undo)
    );

    state["lc"]["operation"]["AddBlock"].setClass(kaguya::UserdataMetatable<lc::operation::AddBlock, lc::operation::DocumentOperation>()
        .addStaticFunction("new", [](lc::storage::Document_SPtr document, lc::meta::Block_CSPtr block) {
            return std::make_shared<lc::operation::AddBlock>(document, block);
        })
        .addFunction("redo", &lc::operation::AddBlock::redo)
        .addFunction("undo", &lc::operation::AddBlock::undo)
    );

    state["lc"]["operation"]["RemoveBlock"].setClass(kaguya::UserdataMetatable<lc::operation::RemoveBlock, lc::operation::DocumentOperation>()
        .addStaticFunction("new", [](lc::storage::Document_SPtr document, lc::meta::Block_CSPtr block) {
            return std::make_shared<lc::operation::RemoveBlock>(document, block);
        })
        .addFunction("redo", &lc::operation::RemoveBlock::redo)
        .addFunction("undo", &lc::operation::RemoveBlock::undo)
    );

    state["lc"]["operation"]["ReplaceBlock"].setClass(kaguya::UserdataMetatable<lc::operation::ReplaceBlock, lc::operation::DocumentOperation>()
        .addStaticFunction("new", [](lc::storage::Document_SPtr document, lc::meta::Block_CSPtr block, lc::meta::Block_CSPtr newBlock) {
            return std::make_shared<lc::operation::ReplaceBlock>(document, block, newBlock);
        })
        .addFunction("redo", &lc::operation::ReplaceBlock::redo)
        .addFunction("undo", &lc::operation::ReplaceBlock::undo)
    );

    state["lc"]["operation"]["AddLinePattern"].setClass(kaguya::UserdataMetatable<lc::operation::AddLinePattern, lc::operation::DocumentOperation>()
        .addStaticFunction("new", [](lc::storage::Document_SPtr document, lc::meta::DxfLinePattern_CSPtr lp) {
            return std::make_shared<lc::operation::AddLinePattern>(document, lp);
        })
        .addFunction("redo", &lc::operation::AddLinePattern::redo)
        .addFunction("undo", &lc::operation::AddLinePattern::undo)
    );

    state["lc"]["operation"]["RemoveLinePattern"].setClass(kaguya::UserdataMetatable<lc::operation::RemoveLinePattern, lc::operation::DocumentOperation>()
        .addStaticFunction("new", [](lc::storage::Document_SPtr document, lc::meta::DxfLinePattern_CSPtr lp) {
            return std::make_shared<lc::operation::RemoveLinePattern>(document, lp);
        })
        .addFunction("redo", &lc::operation::RemoveLinePattern::redo)
        .addFunction("undo", &lc::operation::RemoveLinePattern::undo)
    );

    state["lc"]["operation"]["ReplaceLinePattern"].setClass(kaguya::UserdataMetatable<lc::operation::ReplaceLinePattern, lc::operation::DocumentOperation>()
        .addStaticFunction("new", [](lc::storage::Document_SPtr document, lc::meta::DxfLinePattern_CSPtr lp, lc::meta::DxfLinePattern_CSPtr newLP) {
            return std::make_shared<lc::operation::ReplaceLinePattern>(document, lp, newLP);
        })
        .addFunction("redo", &lc::operation::ReplaceLinePattern::redo)
        .addFunction("undo", &lc::operation::ReplaceLinePattern::undo)
    );

    state["lc"]["operation"]["EntityBuilder"].setClass(kaguya::UserdataMetatable<lc::operation::EntityBuilder, lc::operation::DocumentOperation>()
        .addStaticFunction("new", [](const lc::storage::Document_SPtr& document) {
            return std::make_shared<lc::operation::EntityBuilder>(document);
        })
        .addFunction("appendEntity", &lc::operation::EntityBuilder::appendEntity)
        .addFunction("appendOperation", &lc::operation::EntityBuilder::appendOperation)
        .addFunction("processStack", &lc::operation::EntityBuilder::processStack)
        .addFunction("redo", &lc::operation::EntityBuilder::redo)
        .addFunction("undo", &lc::operation::EntityBuilder::undo)
    );
}

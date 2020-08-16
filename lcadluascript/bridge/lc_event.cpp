#include <cad/events/addlayerevent.h>
#include <cad/events/newwaitingcustomentityevent.h>
#include <cad/events/commitprocessevent.h>
#include <cad/events/addentityevent.h>
#include <cad/events/replacelayerevent.h>
#include <cad/events/removeentityevent.h>
#include <cad/events/replaceentityevent.h>
#include <cad/events/addlinepatternevent.h>
#include <cad/events/removelinepatternevent.h>
#include <cad/events/replacelinepatternevent.h>
#include <cad/events/removelayerevent.h>
#include <cad/primitive/insert.h>
#include "lc_event.h"

void import_lc_event_namespace(kaguya::State& state) {
    state["lc"]["event"] = kaguya::NewTable();

    state["lc"]["event"]["AddLayerEvent"].setClass(kaguya::UserdataMetatable<lc::event::AddLayerEvent>()
            .setConstructors<lc::event::AddLayerEvent(const lc::meta::Layer_CSPtr)>()
            .addFunction("layer", &lc::event::AddLayerEvent::layer)
                                                  );

    state["lc"]["event"]["RemoveLayerEvent"].setClass(kaguya::UserdataMetatable<lc::event::RemoveLayerEvent>()
            .setConstructors<lc::event::RemoveLayerEvent(const lc::meta::Layer_CSPtr)>()
            .addFunction("layer", &lc::event::RemoveLayerEvent::layer)
                                                     );

    state["lc"]["event"]["ReplaceLayerEvent"].setClass(kaguya::UserdataMetatable<lc::event::ReplaceLayerEvent>()
            .setConstructors<lc::event::ReplaceLayerEvent(const lc::meta::Layer_CSPtr, const lc::meta::Layer_CSPtr)>()
            .addFunction("newLayer", &lc::event::ReplaceLayerEvent::newLayer)
            .addFunction("oldLayer", &lc::event::ReplaceLayerEvent::oldLayer)
                                                      );

    state["lc"]["event"]["NewWaitingCustomEntityEvent"].setClass(kaguya::UserdataMetatable<lc::event::NewWaitingCustomEntityEvent>()
            .setConstructors<lc::event::NewWaitingCustomEntityEvent(const lc::entity::Insert_CSPtr &)>()
            .addFunction("insert", &lc::event::NewWaitingCustomEntityEvent::insert)
                                                                );

    state["lc"]["event"]["CommitProcessEvent"].setClass(kaguya::UserdataMetatable<lc::event::CommitProcessEvent>()
            .setConstructors<lc::event::CommitProcessEvent(lc::operation::DocumentOperation_SPtr)>()
            .addFunction("operation", &lc::event::CommitProcessEvent::operation)
                                                       );

    state["lc"]["event"]["AddEntityEvent"].setClass(kaguya::UserdataMetatable<lc::event::AddEntityEvent>()
            .setConstructors<lc::event::AddEntityEvent(lc::entity::CADEntity_CSPtr)>()
            .addFunction("entity", &lc::event::AddEntityEvent::entity)
                                                   );

    state["lc"]["event"]["RemoveEntityEvent"].setClass(kaguya::UserdataMetatable<lc::event::RemoveEntityEvent>()
            .setConstructors<lc::event::RemoveEntityEvent(const lc::entity::CADEntity_CSPtr)>()
            .addFunction("entity", &lc::event::RemoveEntityEvent::entity)
                                                      );

    state["lc"]["event"]["ReplaceEntityEvent"].setClass(kaguya::UserdataMetatable<lc::event::ReplaceEntityEvent>()
            .setConstructors<lc::event::ReplaceEntityEvent(const lc::entity::CADEntity_CSPtr)>()
            .addFunction("entity", &lc::event::ReplaceEntityEvent::entity)
                                                       );

    state["lc"]["event"]["AddLinePatternEvent"].setClass(kaguya::UserdataMetatable<lc::event::AddLinePatternEvent>()
            .setConstructors<lc::event::AddLinePatternEvent(const lc::meta::DxfLinePatternByValue_CSPtr)>()
            .addFunction("linePattern", &lc::event::AddLinePatternEvent::linePattern)
                                                        );

    state["lc"]["event"]["RemoveLinePatternEvent"].setClass(kaguya::UserdataMetatable<lc::event::RemoveLinePatternEvent>()
            .setConstructors<lc::event::RemoveLinePatternEvent(const lc::meta::DxfLinePatternByValue_CSPtr)>()
            .addFunction("linePattern", &lc::event::RemoveLinePatternEvent::linePattern)
                                                           );

    state["lc"]["event"]["ReplaceLinePatternEvent"].setClass(kaguya::UserdataMetatable<lc::event::ReplaceLinePatternEvent>()
            .setConstructors<lc::event::ReplaceLinePatternEvent(const lc::meta::DxfLinePatternByValue_CSPtr, const lc::meta::DxfLinePatternByValue_CSPtr)>()
            .addFunction("newLinePattern", &lc::event::ReplaceLinePatternEvent::newLinePattern)
            .addFunction("oldLinePattern", &lc::event::ReplaceLinePatternEvent::oldLinePattern)
                                                            );
}

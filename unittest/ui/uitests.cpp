#include "uitests.h"

LuaIntf::LuaState startLC() {
    LuaInterface luaInterface;

    luaInterface.hideUI(true);
    luaInterface.initLua();

    return luaInterface.luaState();
}

lc::Document_SPtr createDocument() {
    auto storageManager = std::make_shared<lc::StorageManagerImpl>();
    return std::make_shared<lc::DocumentImpl>(storageManager);
};
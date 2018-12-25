#include "uitests.h"

LuaInterface* startLC() {
    auto luaInterface = new LuaInterface;

    luaInterface->hideUI(true);
    luaInterface->initLua();

    return luaInterface;
}

storage::Document_SPtr createDocument() {
    auto storageManager = std::make_shared<storage::StorageManagerImpl>();
    return std::make_shared<storage::DocumentImpl>(storageManager);
};

CadMdiChild* getMdiChild(lua_State* L) {
    kaguya::State state(L);
    auto windows = state["windows"];

    if(windows.values().empty()) {
        state["new_file"]();
    }

    CadMdiChild* mdiChild = windows[0];

    return mdiChild;
}

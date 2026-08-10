#pragma once

// Phase 4 PR-8 — OpaquePtr type tags shared by lcUI's script-adapter
// extensions.  The tag is a plain C string literal; the OpaquePtr
// encoder registry (lc::lua::registerOpaqueEncoder) compares tags via
// strcmp so multi-TU literals with the same content match.
//
// Add new tags here as more UI types get exposed to script callbacks.

namespace lc {
namespace ui {
namespace opaquetag {

// api::Menu* — used by ContextMenuManager::operationContextCommands
// when it fires the current operation's `contextMenuOptions(op, menu)`
// method.  Encoder registered in guibridge.cpp's luaOpenGUIBridge.
inline constexpr const char* Menu = "lc::ui::api::Menu*";

} // namespace opaquetag
} // namespace ui
} // namespace lc

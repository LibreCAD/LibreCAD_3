#pragma once

// Bridge — lc.persistence.* for Lua (phase 2 slice 2.6).
// Mirror of lcadpythonscript/bridge/py_lc_persistence.h: exposes
// lc::persistence::File::open/save so headless Lua scripts can do
// non-interactive DXF/DWG I/O — no gap vs Python parity.

#include <kaguya/include/kaguya/state.hpp>

void import_lc_persistence_namespace(kaguya::State& state);

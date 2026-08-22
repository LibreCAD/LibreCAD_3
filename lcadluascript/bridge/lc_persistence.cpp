// lc_persistence.cpp — Lua bindings for lc::persistence::File.
// Phase 2 slice 2.6.  Mirrors py_lc_persistence.cpp 1:1 (names, args, enums).

#include <persistence/file.h>
#include "lc_persistence.h"

using lc::persistence::File;

void import_lc_persistence_namespace(kaguya::State& state) {
    state["lc"]["persistence"] = kaguya::NewTable();

    state["lc"]["persistence"]["File"].setClass(
        kaguya::UserdataMetatable<File>()
            // Static gateway methods.
            .addStaticFunction("open", &File::open)
            .addStaticFunction("save", &File::save)
            .addStaticFunction("getAvailableFileTypes",
                               &File::getAvailableFileTypes)
            .addStaticFunction("getAvailableLibrariesForFormat",
                               &File::getAvailableLibrariesForFormat)
            .addStaticFunction("getExtensionForFileType",
                               &File::getExtensionForFileType)
            .addStaticFunction("getSupportedFileExtensions",
                               &File::getSupportedFileExtensions)
    );

    // Type enum — flat integer constants at lc.persistence.File.LIBDXFRW_DXF_R12
    // etc.  Kaguya doesn't provide an addEnum helper here, so we set each name
    // explicitly on the class table for Lua-parity with the Python-side
    // export_values() shape (`lc.persistence.File.LIBDXFRW_DXF_R2000`).
    state["lc"]["persistence"]["File"]["LIBDXFRW_DXF_R12"]   = static_cast<int>(File::LIBDXFRW_DXF_R12);
    state["lc"]["persistence"]["File"]["LIBDXFRW_DXF_R14"]   = static_cast<int>(File::LIBDXFRW_DXF_R14);
    state["lc"]["persistence"]["File"]["LIBDXFRW_DXF_R2000"] = static_cast<int>(File::LIBDXFRW_DXF_R2000);
    state["lc"]["persistence"]["File"]["LIBDXFRW_DXF_R2004"] = static_cast<int>(File::LIBDXFRW_DXF_R2004);
    state["lc"]["persistence"]["File"]["LIBDXFRW_DXF_R2007"] = static_cast<int>(File::LIBDXFRW_DXF_R2007);
    state["lc"]["persistence"]["File"]["LIBDXFRW_DXF_R2010"] = static_cast<int>(File::LIBDXFRW_DXF_R2010);
    state["lc"]["persistence"]["File"]["LIBDXFRW_DXF_R2013"] = static_cast<int>(File::LIBDXFRW_DXF_R2013);
    state["lc"]["persistence"]["File"]["LIBDXFRW_DXB_R12"]   = static_cast<int>(File::LIBDXFRW_DXB_R12);
    state["lc"]["persistence"]["File"]["LIBDXFRW_DXB_R14"]   = static_cast<int>(File::LIBDXFRW_DXB_R14);
    state["lc"]["persistence"]["File"]["LIBDXFRW_DXB_R2000"] = static_cast<int>(File::LIBDXFRW_DXB_R2000);
    state["lc"]["persistence"]["File"]["LIBDXFRW_DXB_R2004"] = static_cast<int>(File::LIBDXFRW_DXB_R2004);
    state["lc"]["persistence"]["File"]["LIBDXFRW_DXB_R2007"] = static_cast<int>(File::LIBDXFRW_DXB_R2007);
    state["lc"]["persistence"]["File"]["LIBDXFRW_DXB_R2010"] = static_cast<int>(File::LIBDXFRW_DXB_R2010);
    state["lc"]["persistence"]["File"]["LIBDXFRW_DXB_R2013"] = static_cast<int>(File::LIBDXFRW_DXB_R2013);
    state["lc"]["persistence"]["File"]["LIBOPENCAD_DWG"]     = static_cast<int>(File::LIBOPENCAD_DWG);

    state["lc"]["persistence"]["File"]["LIBDXFRW"]   = static_cast<int>(File::LIBDXFRW);
    state["lc"]["persistence"]["File"]["LIBOPENCAD"] = static_cast<int>(File::LIBOPENCAD);
}

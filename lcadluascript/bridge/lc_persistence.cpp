// lc_persistence.cpp — Lua bindings for lc::persistence::File.
// Phase 2 slice 2.6.  Mirrors py_lc_persistence.cpp 1:1 (names, args, enums).

#include <persistence/file.h>
#include <persistence/format.h>
#include "lc_persistence.h"

using lc::persistence::File;

void import_lc_persistence_namespace(kaguya::State& state) {
    state["lc"]["persistence"] = kaguya::NewTable();

    // The result types, in parity with the Python side. A script that reads a
    // directory of drawings cannot say anything true about what it found from
    // File.open's revision number alone: it cannot tell a complete drawing from
    // the readable half of a broken one.
    state["lc"]["persistence"]["Diagnostic"].setClass(
        kaguya::UserdataMetatable<lc::persistence::Diagnostic>()
            .addProperty("code", &lc::persistence::Diagnostic::code)
            .addProperty("message", &lc::persistence::Diagnostic::message)
            .addStaticFunction("severity", [](const lc::persistence::Diagnostic& d) {
                return static_cast<int>(d.severity);
            })
    );

    state["lc"]["persistence"]["LossSummary"].setClass(
        kaguya::UserdataMetatable<lc::persistence::LossSummary>()
            .addProperty("droppedByType", &lc::persistence::LossSummary::droppedByType)
            .addProperty("notes", &lc::persistence::LossSummary::notes)
            .addFunction("empty", &lc::persistence::LossSummary::empty)
            .addFunction("total", &lc::persistence::LossSummary::total)
    );

    state["lc"]["persistence"]["ImportResult"].setClass(
        kaguya::UserdataMetatable<lc::persistence::ImportResult>()
            .addProperty("ok", &lc::persistence::ImportResult::ok)
            .addProperty("partial", &lc::persistence::ImportResult::partial)
            .addProperty("variantId", &lc::persistence::ImportResult::variantId)
            .addProperty("sourceVersionTag", &lc::persistence::ImportResult::sourceVersionTag)
            .addProperty("entitiesDelivered", &lc::persistence::ImportResult::entitiesDelivered)
            .addProperty("diagnostics", &lc::persistence::ImportResult::diagnostics)
            .addProperty("loss", &lc::persistence::ImportResult::loss)
    );

    state["lc"]["persistence"]["ExportResult"].setClass(
        kaguya::UserdataMetatable<lc::persistence::ExportResult>()
            .addProperty("ok", &lc::persistence::ExportResult::ok)
            .addProperty("variantId", &lc::persistence::ExportResult::variantId)
            .addProperty("diagnostics", &lc::persistence::ExportResult::diagnostics)
            .addProperty("loss", &lc::persistence::ExportResult::loss)
    );

    state["lc"]["persistence"]["FormatVariant"].setClass(
        kaguya::UserdataMetatable<lc::persistence::FormatVariant>()
            .addProperty("id", &lc::persistence::FormatVariant::id)
            .addProperty("formatId", &lc::persistence::FormatVariant::formatId)
            .addProperty("label", &lc::persistence::FormatVariant::label)
            .addProperty("versionTag", &lc::persistence::FormatVariant::versionTag)
            .addProperty("extension", &lc::persistence::FormatVariant::extension)
            .addProperty("libraryId", &lc::persistence::FormatVariant::libraryId)
            .addProperty("binary", &lc::persistence::FormatVariant::binary)
            .addProperty("readable", &lc::persistence::FormatVariant::readable)
            .addProperty("writable", &lc::persistence::FormatVariant::writable)
            .addProperty("experimental", &lc::persistence::FormatVariant::experimental)
    );

    state["lc"]["persistence"]["formatVariants"] = kaguya::function(
        []() { return lc::persistence::formatVariants(); });
    state["lc"]["persistence"]["formatVariantById"] = kaguya::function(
        [](const std::string& id) {
            const auto* variant = lc::persistence::formatVariantById(id);
            return variant == nullptr ? lc::persistence::FormatVariant()
                                      : *variant;
        });

    // Severity as flat constants, the same shape the Type enum uses below.
    state["lc"]["persistence"]["Severity"] = kaguya::NewTable();
    state["lc"]["persistence"]["Severity"]["Info"] =
        static_cast<int>(lc::persistence::Severity::Info);
    state["lc"]["persistence"]["Severity"]["Warning"] =
        static_cast<int>(lc::persistence::Severity::Warning);
    state["lc"]["persistence"]["Severity"]["Error"] =
        static_cast<int>(lc::persistence::Severity::Error);

    state["lc"]["persistence"]["File"].setClass(
        kaguya::UserdataMetatable<File>()
            // Static gateway methods.
            .addStaticFunction("importFile", &File::importFile)
            .addStaticFunction("exportFile", &File::exportFile)
            .addStaticFunction("variantIdForType", &File::variantIdForType)
            .addStaticFunction("typeForVariantId", [](const std::string& id) {
                File::Type type = File::LIBDXFRW_DXF_R2000;
                return File::typeForVariantId(id, type)
                    ? static_cast<int>(type) : -1;
            })
            // open and save stay: they are what existing scripts call.
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

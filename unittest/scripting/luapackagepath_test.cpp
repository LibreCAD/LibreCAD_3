#include <gtest/gtest.h>
#include <lclua.h>
#include <kaguya/kaguya.hpp>

#include <boost/filesystem.hpp>
#include <fstream>

TEST(LuaRuntimeTest, ModuleSearchPathPreservesExistingEntriesAndIsIdempotent) {
    kaguya::State state;
    lc::lua::LCLua runtime(state.state());
    runtime.addLuaLibs();
    const std::string original = "preserved/?.lua;preserved/?/init.lua";
    state["package"]["path"] = original;

    runtime.addModuleSearchPath("installed path 'quoted'/lcUILua");
    runtime.addModuleSearchPath("installed path 'quoted'/lcUILua/");
    const std::string windowsRoot = "C:\\Users\\O'Brien\\LibreCAD Lua\\";
    runtime.addModuleSearchPath(windowsRoot);
    runtime.addModuleSearchPath(windowsRoot);
    runtime.addModuleSearchPath("");

    EXPECT_EQ(state["package"]["path"].get<std::string>(),
              original + ";installed path 'quoted'/lcUILua/?.lua;" +
                  windowsRoot + "?.lua");
}

TEST(LuaRuntimeTest, InstalledModulesLoadAfterPrefixRelocation) {
    namespace fs = boost::filesystem;
    struct TemporaryPrefix {
        fs::path path = fs::temp_directory_path() /
            fs::unique_path("LibreCAD Lua 'quoted' %%%%%%%%");
        ~TemporaryPrefix() {
            boost::system::error_code error;
            fs::remove_all(path, error);
        }
    } temporary;
    const fs::path installed = temporary.path / "installed prefix";
    const fs::path relocated = temporary.path / "relocated prefix";
    ASSERT_TRUE(fs::create_directories(installed / "lcUILua"));
    {
        std::ofstream module((installed / "lcUILua/librecad_package_path_probe.lua").string());
        ASSERT_TRUE(module.good());
        module << "return 'installed-module-reached'\n";
        ASSERT_TRUE(module.good());
    }

    for (const auto& prefix : {installed, relocated}) {
        if (prefix == relocated) {
            fs::rename(installed, relocated);
            ASSERT_FALSE(fs::exists(installed));
        }
        kaguya::State state;
        lc::lua::LCLua runtime(state.state());
        runtime.addLuaLibs();
        // Installed packages have no build-tree path.lua. Start with no
        // ambient module paths so cwd or a developer checkout cannot pass.
        state["package"]["path"] = "";
        const std::string root = (prefix / "lcUILua").generic_string();
        runtime.addModuleSearchPath(root);
        runtime.addModuleSearchPath(root);
        EXPECT_EQ(state["package"]["path"].get<std::string>(), root + "/?.lua");
        ASSERT_TRUE(runtime.runString(
            "module_value = require('librecad_package_path_probe')").empty());
        EXPECT_EQ(state["module_value"].get<std::string>(), "installed-module-reached");
    }
}

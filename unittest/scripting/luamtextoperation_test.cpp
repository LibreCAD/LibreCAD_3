// The MText creation operation, from Lua.
//
// Two things here that nothing else checks.
//
// The first is decodeTypedText, the only reason MTextOperations is no longer a
// verbatim copy of TextOperations: the command line is one line, so `\P` is
// how a typed MText gets more than one.
//
// The second is the wiring.  An operation's `icon` and `menu_actions` are
// strings resolved at startup against two files nothing compiles together with
// the Lua: a menu action the .ui does not define makes
// MainWindow::connectMenuItem dereference a null MenuItem, and an icon the
// .qrc does not alias is a blank toolbar button.  Both are silent until the
// application runs, so they are checked for every operation, not just MText.

#include <algorithm>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include <boost/filesystem.hpp>

#include <lua.hpp>

#include <lclua.h>

namespace {

std::string readWholeFile(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    std::ostringstream buffer;
    buffer << in.rdbuf();
    return buffer.str();
}

/// Every operation .lua, sorted so a failure names the same file every run.
std::vector<std::string> operationFiles() {
    std::vector<std::string> files;

    for (const char* folder : {"/createActions", "/actions"}) {
        const boost::filesystem::path dir(std::string(LCUILUA_SOURCE_DIR) + folder);
        if (!boost::filesystem::is_directory(dir)) {
            continue;
        }
        for (const auto& entry : boost::filesystem::directory_iterator(dir)) {
            if (entry.path().extension() == ".lua") {
                files.push_back(entry.path().generic_string());
            }
        }
    }

    std::sort(files.begin(), files.end());
    return files;
}

struct LuaMTextOperationFixture : public ::testing::Test {
    void SetUp() override {
        L = luaL_newstate();
        lcLua = std::make_shared<lc::lua::LCLua>(L);
        lcLua->addLuaLibs();
    }

    void TearDown() override {
        lcLua.reset();
        lua_close(L);
    }

    /// Load one operation file into the state.  These are class definitions:
    /// the UI globals they name (mainWindow, luaInterface, CreateOperations)
    /// are only reached from inside their methods, so loading needs no
    /// application.
    void loadOperation(const std::string& path) {
        ASSERT_EQ(lcLua->runString(("dofile('" + path + "')").c_str()), "")
            << "could not load " << path;
    }

    /// Hand a file's text to Lua under a global name, so the assertions can
    /// be written in Lua and report themselves through runString.
    void setGlobalString(const char* name, const std::string& value) {
        lua_pushlstring(L, value.data(), value.size());
        lua_setglobal(L, name);
    }

    lua_State* L{nullptr};
    std::shared_ptr<lc::lua::LCLua> lcLua;
};

}  // namespace

// NOLINTNEXTLINE(readability-identifier-naming)
TEST_F(LuaMTextOperationFixture, TypedTextBreaksIntoLines) {
    loadOperation(std::string(LCUILUA_SOURCE_DIR) + "/createActions/mtextoperations.lua");

    EXPECT_EQ(lcLua->runString(R"LUA(
local decode = MTextOperations.decodeTypedText
assert(decode('one\\Ptwo\\Pthree') == 'one\ntwo\nthree', 'paragraph breaks')
assert(decode('\\P') == '\n', 'a break on its own')
assert(decode('plain text') == 'plain text', 'text without escapes is untouched')
)LUA"), "");
}

// A user typing a Windows path has to be able to say so, and `\\` is how the
// file format spells one literal backslash.
// NOLINTNEXTLINE(readability-identifier-naming)
TEST_F(LuaMTextOperationFixture, AnEscapedBackslashStaysABackslash) {
    loadOperation(std::string(LCUILUA_SOURCE_DIR) + "/createActions/mtextoperations.lua");

    EXPECT_EQ(lcLua->runString(R"LUA(
local decode = MTextOperations.decodeTypedText
assert(decode('C:\\\\Path') == 'C:\\Path', 'an escaped backslash')
assert(decode('\\{braced\\}') == '{braced}', 'escaped braces')
assert(decode('a\\~b') == 'a\194\160b', 'a no-break space')
)LUA"), "");
}

// Deleting characters the user can see is a worse failure than leaving a rare
// escape alone, so an introducer this does not know is kept as typed.
// NOLINTNEXTLINE(readability-identifier-naming)
TEST_F(LuaMTextOperationFixture, AnUnknownEscapeIsKeptAsTyped) {
    loadOperation(std::string(LCUILUA_SOURCE_DIR) + "/createActions/mtextoperations.lua");

    EXPECT_EQ(lcLua->runString(R"LUA(
local decode = MTextOperations.decodeTypedText
assert(decode('\\H2;keep') == '\\H2;keep', 'an unhandled formatting code')
assert(decode('trailing\\') == 'trailing\\', 'a backslash at the end')
)LUA"), "");
}

// MText had no toolbar button and no menu entry: the command line was the only
// route to one that did not open the modal dialog.
// NOLINTNEXTLINE(readability-identifier-naming)
TEST_F(LuaMTextOperationFixture, MTextIsReachableFromTheToolbarAndTheMenu) {
    loadOperation(std::string(LCUILUA_SOURCE_DIR) + "/createActions/mtextoperations.lua");

    EXPECT_EQ(lcLua->runString(R"LUA(
assert(MTextOperations.command_line == 'MTEXT', 'command line')
assert(MTextOperations.icon == 'mtext.svg', 'toolbar icon')
assert(type(MTextOperations.menu_actions) == 'table', 'menu actions')
assert(MTextOperations.menu_actions.default == 'actionMText_Operation', 'menu action')
)LUA"), "");
}

// NOLINTNEXTLINE(readability-identifier-naming)
TEST_F(LuaMTextOperationFixture, EveryMenuActionNamesAnActionTheUiDefines) {
    const std::string ui = readWholeFile(std::string(LCUI_SOURCE_DIR) + "/mainwindow.ui");
    ASSERT_FALSE(ui.empty()) << "could not read mainwindow.ui";

    const std::vector<std::string> files = operationFiles();
    ASSERT_FALSE(files.empty()) << "no operation .lua files under " << LCUILUA_SOURCE_DIR;
    for (const std::string& file : files) {
        loadOperation(file);
    }

    setGlobalString("UI_XML", ui);

    EXPECT_EQ(lcLua->runString(R"LUA(
local checked = 0
for name, class in pairs(_G) do
    if type(name) == 'string' and name:find('Operation')
       and type(class) == 'table' and type(rawget(class, 'menu_actions')) == 'table' then
        for role, action in pairs(class.menu_actions) do
            assert(UI_XML:find('name="' .. action .. '"', 1, true) ~= nil,
                   name .. '.' .. role .. ' names ' .. action
                   .. ', which mainwindow.ui does not define')
            checked = checked + 1
        end
    end
end
assert(checked > 0, 'no menu actions were checked')
)LUA"), "");
}

// NOLINTNEXTLINE(readability-identifier-naming)
TEST_F(LuaMTextOperationFixture, EveryOperationIconIsInTheResourceFile) {
    const std::string qrc = readWholeFile(std::string(LCUI_SOURCE_DIR) + "/ui/resource.qrc");
    ASSERT_FALSE(qrc.empty()) << "could not read resource.qrc";

    const std::vector<std::string> files = operationFiles();
    ASSERT_FALSE(files.empty()) << "no operation .lua files under " << LCUILUA_SOURCE_DIR;
    for (const std::string& file : files) {
        loadOperation(file);
    }

    setGlobalString("QRC_XML", qrc);

    EXPECT_EQ(lcLua->runString(R"LUA(
local checked = 0
for name, class in pairs(_G) do
    if type(name) == 'string' and name:find('Operation') and type(class) == 'table' then
        local icon = rawget(class, 'icon')
        if type(icon) == 'string' then
            assert(QRC_XML:find('alias="' .. icon .. '"', 1, true) ~= nil,
                   name .. ' asks for ' .. icon
                   .. ', which resource.qrc does not alias')
            checked = checked + 1
        end
    end
end
assert(checked > 0, 'no icons were checked')
)LUA"), "");
}

// The persistence bindings, from Lua.
//
// Parity with unittest/python/persistence_test.cpp: whatever a Python script
// can learn about a read, a Lua script must be able to learn too, or the two
// languages disagree about what LibreCAD just did.
//
// Needs lcluascript, kaguya and a real Lua runtime, so it sits with the other
// kaguya-backed tests rather than in unittest/persistence/.

#include <memory>
#include <string>

#include <gtest/gtest.h>

#include <lua.hpp>

#include <lclua.h>

#include <cad/storage/documentimpl.h>
#include <cad/storage/storagemanagerimpl.h>

#include <boost/filesystem.hpp>

namespace {

struct LuaFixture : public ::testing::Test {
    void SetUp() override {
        L = luaL_newstate();
        lcLua = std::make_shared<lc::lua::LCLua>(L);
        lcLua->addLuaLibs();
        lcLua->importLCKernel();

        document = std::make_shared<lc::storage::DocumentImpl>(
            std::make_shared<lc::storage::StorageManagerImpl>());
        lcLua->setDocument(document);
    }

    void TearDown() override {
        lcLua.reset();
        lua_close(L);
    }

    lua_State* L{nullptr};
    std::shared_ptr<lc::lua::LCLua> lcLua;
    std::shared_ptr<lc::storage::DocumentImpl> document;
};

}  // namespace

// NOLINTNEXTLINE(readability-identifier-naming)
TEST_F(LuaFixture, PersistenceNamespaceIsRegistered) {
    // lc.persistence was registered behind `#if USE_PERSISTENCE`, which was
    // defined after this target had already been configured -- so the
    // namespace did not exist at all, while the bridge still compiled and the
    // library still linked.
    EXPECT_EQ(lcLua->runString(R"LUA(
assert(lc.persistence ~= nil, 'lc.persistence is missing')
assert(lc.persistence.File ~= nil, 'lc.persistence.File is missing')
assert(lc.persistence.File.open ~= nil, 'open must stay for existing scripts')
assert(lc.persistence.File.save ~= nil, 'save must stay for existing scripts')
assert(lc.persistence.File.importFile ~= nil)
assert(lc.persistence.File.exportFile ~= nil)
)LUA"), "");
}

// NOLINTNEXTLINE(readability-identifier-naming)
TEST_F(LuaFixture, VariantTableIsReadable) {
    EXPECT_EQ(lcLua->runString(R"LUA(
local id = lc.persistence.File.variantIdForType(lc.persistence.File.LIBDXFRW_DXF_R2000)
assert(id == 'dxf.ac1015.ascii', 'variantIdForType gave ' .. tostring(id))
assert(lc.persistence.File.typeForVariantId('dxf.ac1015.ascii')
       == lc.persistence.File.LIBDXFRW_DXF_R2000)
assert(lc.persistence.File.typeForVariantId('dxf.nope') == -1,
       'an unknown id must be reported, not guessed at')

local binary = lc.persistence.formatVariantById('dxf.ac1009.binary')
assert(binary.binary == true)
assert(binary.writable == true)
assert(binary.versionTag == 'AC1009')
assert(binary.formatId == 'dxf')
)LUA"), "");
}

// NOLINTNEXTLINE(readability-identifier-naming)
TEST_F(LuaFixture, ImportResultIsReadable) {
    const std::string missing =
        (boost::filesystem::temp_directory_path() / "lc3-lua-not-here.dxf").string();
    boost::filesystem::remove(missing);

    lcLua->runString(("missing_path = '" + missing + "'").c_str());

    EXPECT_EQ(lcLua->runString(R"LUA(
local result = lc.persistence.File.importFile(document, missing_path,
                                              lc.persistence.File.LIBDXFRW)
assert(result.ok == false, 'a file that is not there did not fail')
assert(result.partial == false, 'nothing arrived, so nothing is partial')
assert(result.entitiesDelivered == 0)
assert(result.sourceVersionTag == '')
assert(#result.diagnostics >= 1, 'a failure must be explained')
assert(result.diagnostics[1].code ~= '', 'a diagnostic needs a code to match on')
assert(result.loss:empty() == true)
assert(result.loss:total() == 0)
assert(result.variantId ~= '', 'the recorded variant must never be blank')
)LUA"), "");
}

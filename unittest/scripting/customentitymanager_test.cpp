// Phase 6 PR-6.1 sub-piece 1 fixup regression suite for
// LuaCustomEntityManager's concrete-Insert dispatch path.
//
// Coordinator's review of sub-piece 1 caught a subtle type-erasure bug:
// the manager's pre-fixup dispatch did
// `CADEntity_CSPtr insertAsEntity = event.insert();`
// which upcast the concrete Insert_CSPtr at compile time.  When the Lua
// adapter's ScriptValue-round-trip handed that to kaguya's LuaRef
// constructor, `lua_type_traits<shared_ptr<T>>::push()` deduced
// T = CADEntity from the STATIC template argument (verified against
// third_party/kaguya/include/kaguya/type.hpp).  The pushed Lua userdata
// got CADEntity's metatable — missing Insert-specific methods like
// `document()` / `displayBlock()` / `position()`.  Every existing Lua
// custom-entity plugin (rectangle, fractal, spiral, gear, star) calls
// `insert:document()` from its `onNewWaitingCustomEntity` handler and
// would fail post-refactor.
//
// The fix: `unwrapLuaCallback()` returns the underlying `kaguya::LuaRef`
// when the callback was produced by `makeLuaCallback` (runtime tag
// "lua").  Calling the LuaRef DIRECTLY with the concrete Insert_CSPtr
// lets kaguya deduce T = Insert; the pushed userdata gets Insert's
// metatable and Lua-side `insert:document()` works.
//
// This test file exercises BOTH paths end-to-end:
//   1. Native ScriptCallback (nativeCallback) — verifies the neutral
//      ScriptValue path correctly boxes a real Insert and the receiver
//      recovers the concrete type via dynamic_pointer_cast<Insert>.
//   2. Real Lua state (kaguya::State) — verifies the Lua fast path
//      pushes Insert with kaguya's Insert-typed metatable, so a
//      Lua-side `insert:position()` call returns real coordinates.

#include <gtest/gtest.h>

#include <cad/builders/insert.h>
#include <cad/events/newwaitingcustomentityevent.h>
#include <cad/meta/customentitystorage.h>
#include <cad/meta/layer.h>
#include <cad/operations/blockops.h>
#include <cad/primitive/insert.h>
#include <cad/storage/documentimpl.h>
#include <cad/storage/documentlist.h>
#include <cad/storage/storagemanagerimpl.h>

#include <lcscripting/scriptcallback.h>
#include <lcscripting/scriptvalue.h>

// Phase 6 PR-6.1 sub-piece 2a — ScriptCustomEntity + CustomEntityBuilder
// direct-dispatch tests.  Exercises the 6 script-defined behavior slots
// after the LuaRef→ScriptCallback neutralization.
#include <builders/customentity.h>
#include <primitive/customentity.h>

#include <managers/luacustomentitymanager.h>
#include <scriptadapter/luacallback.h>

#include <kaguya/kaguya.hpp>

#include <memory>
#include <string>

namespace {

// Helper: construct a real document + Insert whose displayBlock is a
// CustomEntityStorage with `pluginName`.  Mirrors the pattern from
// unittest/lckernel/dochelpers/documentlist.cpp so the test reuses the
// verified kernel path.
struct InsertFixture {
    std::shared_ptr<lc::storage::Document> doc;
    lc::meta::CustomEntityStorage_CSPtr ces;
    lc::entity::Insert_CSPtr insert;

    explicit InsertFixture(const std::string& pluginName,
                           const std::string& entityName = "TestEntity") {
        doc = std::make_shared<lc::storage::DocumentImpl>(
            std::make_shared<lc::storage::StorageManagerImpl>());
        ces = std::make_shared<lc::meta::CustomEntityStorage>(
            pluginName, entityName,
            lc::geo::Coordinate(11.0, 22.0, 33.0));
        // AddBlock so the storage lives in the document tree.
        std::make_shared<lc::operation::AddBlock>(doc, ces)->execute();

        lc::builder::InsertBuilder ib;
        ib.setDisplayBlock(ces);
        ib.setDocument(doc);
        ib.setLayer(std::make_shared<lc::meta::Layer>());
        insert = ib.build();
    }
};

// -----------------------------------------------------------------------------
// Test 1 — native ScriptCallback path.  Registers a nativeCallback,
// fires the event through the manager's dispatch, verifies the received
// ScriptValue is Kind::Entity AND the stored CADEntity_CSPtr
// dynamic_pointer_casts back to a concrete Insert_CSPtr with the
// expected position.
// -----------------------------------------------------------------------------
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(LuaCustomEntityManager, NativeCallbackReceivesConcreteInsert) {
    auto& mgr = lc::lua::LuaCustomEntityManager::getInstance();
    mgr.removePlugins();  // reset state

    // Capture buffer for the callback.
    lc::entity::CADEntity_CSPtr captured_entity;

    auto native_cb = lc::scripting::nativeCallback(
        [&captured_entity](const std::vector<lc::scripting::ScriptValue>& args)
            -> lc::scripting::ScriptValue {
            EXPECT_EQ(args.size(), 1u)
                << "manager must invoke with exactly ONE argument "
                   "(the insert)";
            if (!args.empty()) {
                EXPECT_EQ(args[0].kind(),
                          lc::scripting::ScriptValue::Kind::Entity)
                    << "the arg MUST be Kind::Entity";
                captured_entity = args[0].asEntity();
            }
            return lc::scripting::ScriptValue{};
        });

    mgr.registerPlugin("NativeTestPlugin", native_cb);

    // Trigger the event via a real document build (matches
    // dochelpers/documentlist.cpp).  The manager subscribed at ctor
    // time, so firing the AddEntity path triggers
    // NewWaitingCustomEntityEvent → DocumentList → the manager.
    InsertFixture fix("NativeTestPlugin");

    // Fire the waiting-entity event manually.  The manager subscribes
    // to DocumentList's signal at construction; direct fire replays
    // the exact path onNewWaitingEntity uses.
    lc::event::NewWaitingCustomEntityEvent evt(fix.insert);
    lc::storage::DocumentList::getInstance().newWaitingCustomEntityEvent()(evt);

    ASSERT_TRUE(static_cast<bool>(captured_entity))
        << "callback must have been invoked with a non-null entity";

    // The critical assertion: dynamic_pointer_cast<Insert> succeeds.
    // Even though ScriptValue::Entity stores CADEntity_CSPtr, the
    // underlying shared_ptr still points at the concrete Insert
    // object; RTTI-based downcasting recovers it.  This proves the
    // native path preserves concrete type at the entity level (the
    // Lua-metatable issue is Lua-specific, addressed by the fast
    // path — see test 2).
    auto as_insert =
        std::dynamic_pointer_cast<const lc::entity::Insert>(captured_entity);
    ASSERT_TRUE(static_cast<bool>(as_insert))
        << "dynamic_pointer_cast<Insert> must succeed on the "
           "received CADEntity_CSPtr — proves the shared_ptr's actual "
           "type is preserved through the ScriptValue::Entity path";

    // Sanity check on the concrete-type accessor: Insert::position()
    // must return the same coord we put on the CustomEntityStorage.
    EXPECT_DOUBLE_EQ(as_insert->position().x(), 11.0);
    EXPECT_DOUBLE_EQ(as_insert->position().y(), 22.0);
    EXPECT_DOUBLE_EQ(as_insert->position().z(), 33.0);

    // Cleanup.
    mgr.removePlugins();
}

// -----------------------------------------------------------------------------
// Test 2 — Lua fast path.  Registers a real Lua callback that reaches
// for Insert-specific methods and asserts on the results.  This is the
// coordinator-requested test: a Lua-side call to a type-specific method
// must succeed post-fix.  Pre-fix, `insert:position()` would return nil
// or error because the CADEntity metatable doesn't have `position`.
// -----------------------------------------------------------------------------
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(LuaCustomEntityManager, LuaCallbackReceivesInsertNotBaseCADEntity) {
    auto& mgr = lc::lua::LuaCustomEntityManager::getInstance();
    mgr.removePlugins();  // reset state

    // Build a fresh Lua state and register Insert's metatable so
    // `insert:position()` works from Lua.  The metatable exposes
    // position() → geo::Coordinate; we ask Lua to store the returned
    // x/y/z into three globals so C++ can assert on them.
    kaguya::State state;
    state["_captured_x"] = 0.0;
    state["_captured_y"] = 0.0;
    state["_captured_z"] = 0.0;
    state["_hits"] = 0;

    // Register lc.entity.Insert with the specific methods our Lua
    // callback needs.  This is the same shape as
    // lcadluascript/bridge/lc_entity.cpp's real binding — we register
    // ONLY what the test callback touches.
    state["lc"] = kaguya::NewTable();
    state["lc"]["entity"] = kaguya::NewTable();
    state["lc"]["entity"]["Insert"].setClass(
        kaguya::UserdataMetatable<lc::entity::Insert>()
            .addFunction("position", &lc::entity::Insert::position));

    // Register lc::geo::Coordinate so position() returns a Lua-visible
    // userdata with x()/y()/z() accessors.
    state["lc"]["geo"] = kaguya::NewTable();
    state["lc"]["geo"]["Coordinate"].setClass(
        kaguya::UserdataMetatable<lc::geo::Coordinate>()
            .addFunction("x", &lc::geo::Coordinate::x)
            .addFunction("y", &lc::geo::Coordinate::y)
            .addFunction("z", &lc::geo::Coordinate::z));

    // Define the Lua callback.  The critical call:
    // `insert:position()` reaches for Insert's `position` method,
    // which requires the userdata to have Insert's metatable, not
    // CADEntity's.  Pre-fix, this call returned nil (base metatable
    // lacks position); the callback would set captured_x = 0 and the
    // test would fail on the coordinate assertion.
    state.dostring(R"lua(
function my_plugin(insert)
    _hits = _hits + 1
    local pos = insert:position()
    _captured_x = pos:x()
    _captured_y = pos:y()
    _captured_z = pos:z()
end
    )lua");

    kaguya::LuaRef fn = state["my_plugin"];
    ASSERT_EQ(fn.type(), LUA_TFUNCTION)
        << "Lua-side callback must be a function — test setup error";

    // Register via the LuaRef overload; the manager wraps it via
    // makeLuaCallback internally, but dispatch uses unwrapLuaCallback
    // to get the LuaRef back for the fast path.
    mgr.registerPlugin("LuaTestPlugin", fn);

    // Fire an event with a real Insert on the same plugin name.
    InsertFixture fix("LuaTestPlugin");
    lc::event::NewWaitingCustomEntityEvent evt(fix.insert);
    lc::storage::DocumentList::getInstance().newWaitingCustomEntityEvent()(evt);

    // Assertions:
    EXPECT_EQ(state["_hits"].get<int>(), 1)
        << "the Lua callback must have been invoked exactly once — "
           "if hits == 0, the dispatch never fired the LuaRef; if > 1, "
           "the manager is over-firing";

    // The critical type-erasure check.  If the fix regressed and
    // the userdata got CADEntity's metatable instead of Insert's,
    // `insert:position()` would return nil and pos:x()/y()/z() would
    // error or return 0.  These assertions catch the regression.
    EXPECT_DOUBLE_EQ(state["_captured_x"].get<double>(), 11.0)
        << "insert:position():x() must return 11.0 — if this is 0, the "
           "Lua userdata got CADEntity's metatable instead of Insert's "
           "and the position() method is not available.  The fix in "
           "luacustomentitymanager.cpp::dispatchToPlugin (using "
           "unwrapLuaCallback for the Lua fast path) is not working.";
    EXPECT_DOUBLE_EQ(state["_captured_y"].get<double>(), 22.0);
    EXPECT_DOUBLE_EQ(state["_captured_z"].get<double>(), 33.0);

    // Cleanup.
    mgr.removePlugins();
}

// -----------------------------------------------------------------------------
// Test 3 — replay path.  When registerPlugin is called AFTER some
// waiting entities already exist, the manager fires the newly-
// registered callback for each of them.  Verifies the replay uses the
// SAME concrete-type dispatch (bug symmetry check: if the primary
// dispatch preserves type but replay doesn't, plugins registering
// after a DXF was opened would get CADEntity-typed userdata for the
// replayed inserts).
// -----------------------------------------------------------------------------
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(LuaCustomEntityManager, ReplayPathPreservesConcreteType) {
    auto& mgr = lc::lua::LuaCustomEntityManager::getInstance();
    mgr.removePlugins();

    // First, create a waiting Insert BEFORE registering the plugin.
    // The DocumentList tracks it as "waiting" because no plugin was
    // registered when it was created.
    InsertFixture fix("ReplayTestPlugin");

    // Verify the DocumentList sees the waiting entity.
    ASSERT_EQ(lc::storage::DocumentList::getInstance()
                  .waitingCustomEntities("ReplayTestPlugin").size(), 1u)
        << "test setup: the Insert must be in the waiting list "
           "before we register the plugin";

    // Now register the plugin — the manager's registerPlugin should
    // replay for the already-waiting entity, using the SAME
    // dispatchToPlugin helper as the primary onNewWaitingEntity path.
    lc::entity::CADEntity_CSPtr replay_captured;
    auto native_cb = lc::scripting::nativeCallback(
        [&replay_captured](
            const std::vector<lc::scripting::ScriptValue>& args)
            -> lc::scripting::ScriptValue {
            if (!args.empty() && args[0].kind() ==
                    lc::scripting::ScriptValue::Kind::Entity) {
                replay_captured = args[0].asEntity();
            }
            return lc::scripting::ScriptValue{};
        });
    mgr.registerPlugin("ReplayTestPlugin", native_cb);

    ASSERT_TRUE(static_cast<bool>(replay_captured))
        << "the replay path must invoke the callback with the "
           "waiting Insert";

    auto as_insert = std::dynamic_pointer_cast<const lc::entity::Insert>(
        replay_captured);
    ASSERT_TRUE(static_cast<bool>(as_insert))
        << "the replay path must preserve concrete Insert type — "
           "if this fails, dispatchToPlugin() is being bypassed on "
           "the replay path";

    EXPECT_DOUBLE_EQ(as_insert->position().x(), 11.0);

    // Cleanup.
    mgr.removePlugins();
}

// -----------------------------------------------------------------------------
// Test 4 — Phase 6 PR-6.1 sub-piece 2a: ScriptCustomEntity's OWN
// dispatch preserves concrete type.  Constructs a ScriptCustomEntity
// via CustomEntityBuilder with a Lua-side snap callback, invokes
// snapPoints(...) directly on the entity, and asserts:
//   (a) the Lua callback fired (via a hit-counter global)
//   (b) inside the callback, `insert:position()` returned real
//       coordinates (proves the userdata got Insert's metatable, not
//       CADEntity's — same lesson as sub-piece 1's fixup applied to
//       every one of the 6 script-defined behavior slots)
//
// Bug scenario this test catches: if the sub-piece 2a refactor lost
// the `unwrapLuaCallback` fast path, the Lua-side `insert:position()`
// would return nil and the coordinate assertions would fail.
// -----------------------------------------------------------------------------
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(ScriptCustomEntity, SnapDispatchPreservesConcreteInsertType) {
    // Register a native "no-op" plugin so the manager doesn't complain
    // about a missing plugin name when we build our custom entity via
    // the builder (the builder itself doesn't require a registration,
    // but keeping state clean avoids cross-test contamination).
    auto& mgr = lc::lua::LuaCustomEntityManager::getInstance();
    mgr.removePlugins();

    // Build a real Lua state and register Insert::position + Coordinate
    // accessors so the Lua-side callback can call them.
    kaguya::State state;
    state["_snap_hits"] = 0;
    state["_snap_x"] = 0.0;
    state["_snap_y"] = 0.0;

    state["lc"] = kaguya::NewTable();
    state["lc"]["entity"] = kaguya::NewTable();
    state["lc"]["entity"]["Insert"].setClass(
        kaguya::UserdataMetatable<lc::entity::Insert>()
            .addFunction("position", &lc::entity::Insert::position));

    state["lc"]["geo"] = kaguya::NewTable();
    state["lc"]["geo"]["Coordinate"].setClass(
        kaguya::UserdataMetatable<lc::geo::Coordinate>()
            .addFunction("x", &lc::geo::Coordinate::x)
            .addFunction("y", &lc::geo::Coordinate::y)
            .addFunction("z", &lc::geo::Coordinate::z));

    // EntityCoordinate return type — the snap callback must return
    // a list of these.  Also bind SimpleSnapConstrain so the callback
    // signature matches.
    state["lc"]["EntityCoordinate"].setClass(
        kaguya::UserdataMetatable<lc::EntityCoordinate>()
            .setConstructors<lc::EntityCoordinate(const lc::geo::Coordinate&, int)>());
    state["lc"]["SimpleSnapConstrain"].setClass(
        kaguya::UserdataMetatable<lc::SimpleSnapConstrain>());

    // The Lua callback that snapPoints will dispatch to.  It calls
    // `insert:position()` — needs Insert's metatable — and returns a
    // single EntityCoordinate at the insert's position.
    state.dostring(R"lua(
function my_snap(insert, coord, constrain, min_dist, max_pts)
    _snap_hits = _snap_hits + 1
    local pos = insert:position()
    _snap_x = pos:x()
    _snap_y = pos:y()
    return { lc.EntityCoordinate(pos, 0) }
end
    )lua");
    kaguya::LuaRef snap_fn = state["my_snap"];
    ASSERT_EQ(snap_fn.type(), LUA_TFUNCTION)
        << "test setup: my_snap must be a Lua function";

    // We need a stub for the other 5 callbacks that builder->build()
    // requires (checkValues fails on any nil callback).  A simple
    // native no-op callback works.
    auto noop = lc::scripting::nativeCallback(
        []() { return lc::scripting::ScriptValue{}; });

    // Build a real document + CustomEntityStorage so the CustomEntity
    // (which inherits Insert) has a valid displayBlock + document.
    auto doc = std::make_shared<lc::storage::DocumentImpl>(
        std::make_shared<lc::storage::StorageManagerImpl>());
    auto ces = std::make_shared<lc::meta::CustomEntityStorage>(
        "DispatchTestPlugin", "TestEntity",
        lc::geo::Coordinate(7.0, 14.0, 21.0));
    std::make_shared<lc::operation::AddBlock>(doc, ces)->execute();

    // Construct the ScriptCustomEntity via CustomEntityBuilder.  Wire
    // the Lua snap fn via the LuaRef overload (which wraps into a
    // ScriptCallback internally); wire noops for the other 5 slots.
    lc::builder::CustomEntityBuilder ceb;
    ceb.setDisplayBlock(ces);
    ceb.setDocument(doc);
    ceb.setLayer(std::make_shared<lc::meta::Layer>());
    ceb.setCoordinate(lc::geo::Coordinate(7.0, 14.0, 21.0));
    ceb.setSnapFunction(snap_fn);
    ceb.setNearestPointFunction(noop);
    ceb.setDragPointsFunction(noop);
    ceb.setNewDragPointFunction(noop);
    ceb.setDragPointsClickedFunction(noop);
    ceb.setDragPointsReleasedFunction(noop);
    auto sce = ceb.build();

    ASSERT_TRUE(static_cast<bool>(sce))
        << "builder.build() must return a non-null ScriptCustomEntity";

    // Dispatch snapPoints directly.  Under the fix, the Lua fast path
    // fires and `insert:position()` returns 7/14/21.  If the refactor
    // broke the fast path, _snap_x/_snap_y would be 0.
    lc::SimpleSnapConstrain constrain;
    auto snapResult = sce->snapPoints(
        lc::geo::Coordinate(0, 0, 0),  // query point
        constrain,
        1e6,   // min distance
        16);   // max points

    EXPECT_EQ(state["_snap_hits"].get<int>(), 1)
        << "snap callback must have been invoked exactly once — if 0, "
           "dispatchToPlugin's fast path isn't being reached; if > 1, "
           "the manager is over-firing";

    // The critical assertion: `insert:position()` from Lua returned
    // the ACTUAL Insert's position (7/14).  Pre-fix or if the
    // refactor breaks the fast path, these would be 0 because the
    // userdata's metatable wouldn't have `position`.
    EXPECT_DOUBLE_EQ(state["_snap_x"].get<double>(), 7.0)
        << "Lua-side insert:position():x() must return 7.0 — regressions "
           "in ScriptCustomEntity's Lua fast-path dispatch land here";
    EXPECT_DOUBLE_EQ(state["_snap_y"].get<double>(), 14.0);

    // The returned snap points should have exactly 1 entry (from the
    // Lua callback's return).  After Snapable::snapPointsCleanup the
    // list is filtered by distance — with min_dist = 1e6 the single
    // point at distance ~15.6 from origin survives.
    EXPECT_EQ(snapResult.size(), 1u)
        << "snap callback returned 1 EntityCoordinate; after cleanup "
           "with a very-generous minDistance, the point should survive";

    // Cleanup.
    mgr.removePlugins();
}

// NOLINTNEXTLINE(readability-identifier-naming)
TEST(ScriptCustomEntity, LuaCustomEntityTypedefStillWorks) {
    // Regression guard: the LuaCustomEntity typedef in
    // primitive/customentity.h and builders/customentity.h must map to
    // ScriptCustomEntity so pre-existing callers spelling
    // `LuaCustomEntity_CSPtr` continue to compile.  This is a
    // compile-time check disguised as a runtime test — if the typedef
    // is missing or wrong, the file doesn't compile.
    lc::entity::LuaCustomEntity_CSPtr as_lua_typedef;
    lc::entity::ScriptCustomEntity_CSPtr as_new_name;
    EXPECT_EQ(static_cast<bool>(as_lua_typedef),
              static_cast<bool>(as_new_name))
        << "both spellings must produce interoperable smart pointers "
           "(default-constructed here — both should be null)";

    // Also verify the typedef is a REAL type alias (not a subclass) by
    // constructing one shape and assigning it to the other.
    lc::entity::LuaCustomEntity_CSPtr a;
    lc::entity::ScriptCustomEntity_CSPtr b = a;
    lc::entity::LuaCustomEntity_CSPtr c = b;
    EXPECT_FALSE(static_cast<bool>(c));  // still null, but the round-trip typechecked
}

} // namespace

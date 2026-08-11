#pragma once

// Phase 6 PR-6.1 sub-piece 2b — ScriptCustomEntity physically moved from
// lcadluascript to lcscripting.  See sub-piece 2a's original file
// (`lcadluascript/primitive/customentity.h` pre-move) for the class
// rename + slot neutralization history.
//
// Dispatch preserves concrete Insert type via the
// `CustomEntityDispatchHook` mechanism (see
// `lcscripting/customentitydispatchhook.h`).  lcadluascript installs a
// `LuaCustomEntityDispatchHook` at LCLua construction that uses
// `unwrapLuaCallback` + `luaRef.call<T>()` for the Lua fast path.
// Non-Lua callbacks fall through to `callback.invoke(...)` per the
// sub-piece 2a fixup.

#include "../builders/customentity.h"
#include "../scriptcallback.h"
#include <cad/primitive/customentity.h>
#include <cad/interface/snapable.h>

namespace lc {
namespace entity {

/**
 * @brief Script-defined custom entity (was `LuaCustomEntity` pre-PR-6.1
 * sub-piece 2a; physically moved to lcscripting in sub-piece 2b).
 *
 * The 6 script-defined behaviors — snap / nearestPoint / dragPoints /
 * newDragPoint / dragPointClick / dragPointRelease — are stored as
 * neutral `lc::scripting::ScriptCallback` slots.  Dispatch queries the
 * installed `CustomEntityDispatchHook` for the concrete-type-preserving
 * fast path; falls through to `callback.invoke(...)` for non-Lua
 * callbacks.
 */
class ScriptCustomEntity : public CustomEntity {
public:
    ScriptCustomEntity(const builder::CustomEntityBuilder& builder);

    /**
     * @brief Constructor used in operations which only affect the insert
     * @param customEntity Custom entity to copy functions from
     * @param insert Insert to copy data from, should be an instance of Insert
     */
    ScriptCustomEntity(const Insert_CSPtr& insert,
                       const std::shared_ptr<const ScriptCustomEntity>& customEntity,
                       bool sameID = false);

    std::vector<EntityCoordinate> snapPoints(const geo::Coordinate& coord,
            const SimpleSnapConstrain& simpleSnapConstrain,
            double minDistanceToSnap,
            int maxNumberOfSnapPoints) const override;

    geo::Coordinate nearestPointOnPath(const geo::Coordinate& coord) const override;

    std::map<unsigned int, geo::Coordinate> dragPoints() const override;
    void setDragPoint(lc::geo::Coordinate position) const override;
    void onDragPointClick(lc::operation::Builder_SPtr builder, unsigned int point) const override;
    void onDragPointRelease(lc::operation::Builder_SPtr builder) const override;

    CADEntity_CSPtr modifyInsert(const CADEntity_CSPtr& insert) const;
    CADEntity_CSPtr move(const geo::Coordinate& offset) const override;
    CADEntity_CSPtr copy(const geo::Coordinate& offset) const override;
    CADEntity_CSPtr rotate(const geo::Coordinate& rotation_center, double rotation_angle) const override;
    CADEntity_CSPtr scale(const geo::Coordinate& scale_center, const geo::Coordinate& scale_factor) const override;
    CADEntity_CSPtr mirror(const geo::Coordinate& axis1, const geo::Coordinate& axis2) const override;
    CADEntity_CSPtr modify(meta::Layer_CSPtr layer, meta::MetaInfo_CSPtr metaInfo, meta::Block_CSPtr block) const override;

private:
    lc::scripting::ScriptCallback _snapPoints;
    lc::scripting::ScriptCallback _nearestPoint;
    lc::scripting::ScriptCallback _dragPoints;
    lc::scripting::ScriptCallback _newDragPoint;
    lc::scripting::ScriptCallback _dragPointClick;
    lc::scripting::ScriptCallback _dragPointRelease;
};

// DECLARE_SHORT_SHARED_PTR + LuaCustomEntity typedefs live in the
// builder header (which the primitive header includes above); no
// re-declaration here to avoid the compile error of "redeclaring an
// alias".
}
}

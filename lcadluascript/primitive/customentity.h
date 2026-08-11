#pragma once

#include "../builders/customentity.h"
#include <cad/primitive/customentity.h>
#include <cad/interface/snapable.h>

// Phase 6 PR-6.1 sub-piece 2a — LuaCustomEntity renamed to
// ScriptCustomEntity with a compat typedef.  The 6 function slots
// (snap / nearestPoint / dragPoints / newDragPoint / dragPointClick /
// dragPointRelease) neutralized from kaguya::LuaRef to
// lc::scripting::ScriptCallback so both Lua and (future) Python
// custom-entity plugins share the same storage type.
//
// The Lua fast-path from PR-6.1 sub-piece 1's fixup (unwrapLuaCallback
// → direct kaguya::LuaRef call preserving concrete Insert type) is
// applied at every dispatch site here — same concrete-type-preservation
// lesson the coordinator emphasized after the LuaCustomEntityManager
// review.  Non-Lua callbacks (Python, native) fall through to the
// neutral ScriptValue path; Python custom-entity plugin bindings are
// scoped for sub-piece 3.
//
// The physical file-move to lcscripting/ (per sub-plan) is deferred to
// sub-piece 2b — requires adding a "Lua fast-path dispatch hook"
// mechanism because lcscripting can't include lcadluascript's
// scriptadapter/luacallback.h.
#include <lcscripting/scriptcallback.h>
#include <kaguya/kaguya.hpp>  // still needed for the constructor-time
                              // kaguya::LuaRef→ScriptCallback wrap in
                              // the builder path; will drop when the
                              // physical move happens in sub-piece 2b.

namespace lc {
namespace entity {

/**
 * @brief Script-defined custom entity (was `LuaCustomEntity` pre-PR-6.1
 * sub-piece 2a).
 *
 * The 6 script-defined behaviors — snap / nearestPoint / dragPoints /
 * newDragPoint / dragPointClick / dragPointRelease — are stored as
 * neutral `lc::scripting::ScriptCallback` slots.  Dispatch preserves
 * the concrete Insert type for Lua callbacks via `unwrapLuaCallback`
 * (see customentity.cpp for the fast-path implementation).
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
    // Phase 6 PR-6.1 sub-piece 2a — was `kaguya::LuaRef`; now the
    // neutral ScriptCallback so Python/native adapters can also
    // register these behaviors.  Dispatch in customentity.cpp uses
    // `unwrapLuaCallback` for the Lua fast path.
    lc::scripting::ScriptCallback _snapPoints;
    lc::scripting::ScriptCallback _nearestPoint;
    lc::scripting::ScriptCallback _dragPoints;
    lc::scripting::ScriptCallback _newDragPoint;
    lc::scripting::ScriptCallback _dragPointClick;
    lc::scripting::ScriptCallback _dragPointRelease;
};

DECLARE_SHORT_SHARED_PTR(ScriptCustomEntity)

// Phase 6 PR-6.1 sub-piece 2a — API-compatibility typedefs.  Callers
// that spell `LuaCustomEntity` / `LuaCustomEntity_CSPtr` /
// `LuaCustomEntity_SPtr` continue to compile without change.  Preserves
// backward compatibility with the pre-refactor internal + external API.
using LuaCustomEntity = ScriptCustomEntity;
using LuaCustomEntity_CSPtr = ScriptCustomEntity_CSPtr;
using LuaCustomEntity_SPtr = ScriptCustomEntity_SPtr;

}
}

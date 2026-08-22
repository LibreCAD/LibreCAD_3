#pragma once

// ScriptValue — language-neutral tagged variant used for callback args,
// event payloads, and InputGUI values.  Phase 4 PR-1.
//
// Design (see master plan decision 4 and phase-4 sub-plan "New types"):
//   nil | bool | int | double | std::string | lc::geo::Coordinate | lc::Color
//   | lc::entity::CADEntity_CSPtr | std::vector<Coordinate>
//   | std::vector<CADEntity_CSPtr> | OpaquePtr | Map
//
// (ScriptObject — the "operation instance" object shape — is declared in a
//  later commit within PR-1; this header includes a forward declaration so
//  ScriptValue can hold it, and defaults to Nil until the class exists.)
//
// Two design invariants that later slices rely on:
//   * `Map` is shared_ptr-backed so entries can ALIAS.  ListGUI's nested
//     table object IS the same value stored under the list key AND at the
//     top level (listgui.cpp:56 stores a reference, not a copy).  Copying
//     into two independent maps would break PropertyEditor's read path
//     (propertyeditor.cpp:219).
//   * OpaquePtr carries a type tag as a `const char*` (compile-time-unique
//     literal) so lcUI can register per-tag materializers WITHOUT
//     lcscripting knowing anything about lcUI types (CadMdiChild*,
//     api::Menu*) — those are forward-declared upstairs and materialized
//     into per-runtime handles by adapter extensions in lcUI.

#include <cad/base/cadentity.h>
#include <cad/geometry/geocoordinate.h>
#include <cad/meta/color.h>

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace lc {
namespace scripting {

class ScriptValue;   // fwd for Map

/// Opaque handle to a UI type declared elsewhere (CadMdiChild*, api::Menu*).
/// The type tag is a compile-time-unique string literal (`typeid(T).name()`
/// or a bespoke constant); the value pointer is caller-owned raw pointer.
/// Deliberately NON-owning — matches Lua's raw-pointer exposure of QWidget
/// subclasses (double-free trap if we added a holder here; see phase-3
/// sub-plan's "QWidget-owned classes are bound HOLDER-LESS" note).
struct OpaquePtr {
    void*       ptr = nullptr;
    const char* tag = nullptr;   // compile-time-unique string literal
};

/// Ordered map, shared-pImpl.  Iteration order preserves insertion (Lua
/// tables built by builder patterns rely on it in a few places).
///
/// Using `std::shared_ptr<std::map<...>>` so `Map a = b; a[k] = v;` mutates
/// `b` too — that is the plan's aliasing requirement.  Callers that want
/// copy semantics must deep-copy explicitly.
using Map = std::shared_ptr<std::map<std::string, ScriptValue>>;

/// Empty-Map factory (kept close to Map so callers don't have to spell out
/// the make_shared incantation).
Map makeMap();

// -----------------------------------------------------------------------------
class ScriptValue {
public:
    enum class Kind : std::uint8_t {
        Nil,
        Bool,
        Int,
        Double,
        String,
        Coordinate,
        Color,
        Entity,
        CoordinateList,
        EntityList,
        Opaque,
        MapKind,
    };

    // Nil default.
    ScriptValue() = default;

    // Value-type constructors — deliberately explicit for the "risky"
    // conversions (int vs double, entity_ptr vs opaque) and implicit for
    // the everyday ones.  If this bites, downgrade to fully-explicit.
    ScriptValue(bool v)        : _kind(Kind::Bool),   _bool(v)   {}
    ScriptValue(int v)         : _kind(Kind::Int),    _int(v)    {}
    ScriptValue(double v)      : _kind(Kind::Double), _double(v) {}
    ScriptValue(const char* v) : _kind(Kind::String), _str(v ? v : "") {}
    ScriptValue(std::string v) : _kind(Kind::String), _str(std::move(v)) {}
    ScriptValue(lc::geo::Coordinate v)
        : _kind(Kind::Coordinate), _coord(v) {}
    ScriptValue(lc::Color v)
        : _kind(Kind::Color), _color(v) {}
    ScriptValue(lc::entity::CADEntity_CSPtr v)
        : _kind(Kind::Entity), _entity(std::move(v)) {}
    ScriptValue(std::vector<lc::geo::Coordinate> v)
        : _kind(Kind::CoordinateList), _coordList(std::move(v)) {}
    ScriptValue(std::vector<lc::entity::CADEntity_CSPtr> v)
        : _kind(Kind::EntityList), _entityList(std::move(v)) {}
    ScriptValue(OpaquePtr v)   : _kind(Kind::Opaque), _opaque(v) {}
    ScriptValue(Map v)         : _kind(Kind::MapKind), _map(std::move(v)) {}

    Kind kind() const { return _kind; }
    bool isNil() const { return _kind == Kind::Nil; }

    // Typed accessors — each returns the stored value on Kind match, or a
    // default-constructed value otherwise (never throws; adapters check
    // kind() first for anything error-worthy).
    bool                                        asBool()       const { return _kind == Kind::Bool ? _bool : false; }
    int                                         asInt()        const { return _kind == Kind::Int ? _int : 0; }
    double                                      asDouble()     const { return _kind == Kind::Double ? _double : 0.0; }
    const std::string&                          asString()     const { return _str; }
    const lc::geo::Coordinate&                  asCoordinate() const { return _coord; }
    const lc::Color&                            asColor()      const { return _color; }
    const lc::entity::CADEntity_CSPtr&          asEntity()     const { return _entity; }
    const std::vector<lc::geo::Coordinate>&     asCoordinateList() const { return _coordList; }
    const std::vector<lc::entity::CADEntity_CSPtr>& asEntityList() const { return _entityList; }
    OpaquePtr                                   asOpaque()     const { return _opaque; }
    const Map&                                  asMap()        const { return _map; }

    // Structural equality.  Used by ScriptCallback::operator== for the
    // deleteEvent parity path (see luainterface.cpp:126).
    bool operator==(const ScriptValue& other) const;
    bool operator!=(const ScriptValue& other) const { return !(*this == other); }

private:
    Kind                                         _kind = Kind::Nil;
    // Trivially-copyable / small POD payload (default-constructed unused
    // fields are cheap; we do not attempt tagged-union tightness here — the
    // extra bytes vs the simplicity + safety of a POD struct is a fine
    // trade for a value type we hold on the stack).
    bool                                         _bool   = false;
    int                                          _int    = 0;
    double                                       _double = 0.0;
    std::string                                  _str;
    lc::geo::Coordinate                          _coord;
    lc::Color                                    _color;
    lc::entity::CADEntity_CSPtr                  _entity;
    std::vector<lc::geo::Coordinate>             _coordList;
    std::vector<lc::entity::CADEntity_CSPtr>     _entityList;
    OpaquePtr                                    _opaque;
    Map                                          _map;
};

} // namespace scripting
} // namespace lc

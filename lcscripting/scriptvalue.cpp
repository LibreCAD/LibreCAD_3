#include "scriptvalue.h"

namespace lc {
namespace scripting {

Map makeMap() {
    return std::make_shared<std::map<std::string, ScriptValue>>();
}

bool ScriptValue::operator==(const ScriptValue& other) const {
    if (_kind != other._kind) {
        return false;
    }
    switch (_kind) {
    case Kind::Nil:            return true;
    case Kind::Bool:           return _bool   == other._bool;
    case Kind::Int:            return _int    == other._int;
    case Kind::Double:         return _double == other._double;
    case Kind::String:         return _str    == other._str;
    case Kind::Coordinate:
        return _coord.x() == other._coord.x()
            && _coord.y() == other._coord.y()
            && _coord.z() == other._coord.z();
    case Kind::Color:
        // lc::Color has op== but returns non-const on non-const args
        // (color.h:37); const_cast around it for compatibility (upstream
        // bug we shouldn't fix in this plan).
        return const_cast<lc::Color&>(_color) == other._color;
    case Kind::Entity:
        return _entity.get() == other._entity.get();
    case Kind::CoordinateList: {
        if (_coordList.size() != other._coordList.size()) return false;
        for (std::size_t i = 0; i < _coordList.size(); ++i) {
            if (_coordList[i].x() != other._coordList[i].x()
             || _coordList[i].y() != other._coordList[i].y()
             || _coordList[i].z() != other._coordList[i].z()) {
                return false;
            }
        }
        return true;
    }
    case Kind::EntityList: {
        if (_entityList.size() != other._entityList.size()) return false;
        for (std::size_t i = 0; i < _entityList.size(); ++i) {
            if (_entityList[i].get() != other._entityList[i].get()) return false;
        }
        return true;
    }
    case Kind::Opaque:
        return _opaque.ptr == other._opaque.ptr
            && _opaque.tag == other._opaque.tag;
    case Kind::MapKind:
        // Structural equality on the underlying map.  Uses shared_ptr identity
        // as a fast path (aliased maps ARE the same map by design), then
        // falls back to element-wise compare.
        if (_map.get() == other._map.get()) return true;
        if (!_map || !other._map) return false;
        if (_map->size() != other._map->size()) return false;
        for (const auto& kv : *_map) {
            auto it = other._map->find(kv.first);
            if (it == other._map->end()) return false;
            if (!(kv.second == it->second)) return false;
        }
        return true;
    }
    return false;
}

} // namespace scripting
} // namespace lc

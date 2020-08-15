#pragma once

#include <map>

namespace lc {
namespace tools {
class MapHelper {
public:
    template <typename T, typename U>
    static std::map<U, T> reverse(const std::map<T, U>& map);
};

template<typename T, typename U>
std::map<U, T> MapHelper::reverse(const std::map<T, U>& map) {
    std::map<U, T> reversed;

    for(auto pair : map) {
        reversed[pair.second] = pair.first;
    }

    return reversed;
}
}
}
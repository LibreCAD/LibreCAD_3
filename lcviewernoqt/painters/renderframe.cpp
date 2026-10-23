#include "renderframe.h"
#include <cmath>
#include <stdexcept>

namespace lc { namespace viewer { namespace render {
void validateFrame(const Frame& f) {
    if (f.width < 0 || f.height < 0 || f.width > MaxFrameDimension || f.height > MaxFrameDimension ||
        std::uint64_t(f.width) * std::uint64_t(f.height) > MaxFramePixels ||
        f.vertices.size() > MaxFrameElements || f.indices.size() > MaxFrameElements ||
        (f.indices.empty() ? f.vertices.size() % 3 != 0 : f.indices.size() % 3 != 0) ||
        ((f.width == 0 || f.height == 0) && (!f.vertices.empty() || !f.indices.empty())))
        throw std::invalid_argument("Invalid frame dimensions or triangle resource limit");
    const auto color = [](float r, float g, float b, float a) {
        return std::isfinite(r) && std::isfinite(g) && std::isfinite(b) && std::isfinite(a) &&
               a >= 0 && a <= 1 && r >= 0 && g >= 0 && b >= 0 && r <= a && g <= a && b <= a;
    };
    if (!color(f.clear[0], f.clear[1], f.clear[2], f.clear[3]))
        throw std::invalid_argument("Frame clear requires premultiplied finite RGBA");
    for (auto index : f.indices) {
        if (index >= f.vertices.size()) throw std::invalid_argument("Frame triangle index is out of range");
    }
    for (const auto& v : f.vertices) {
        if (!std::isfinite(v.x) || !std::isfinite(v.y) || std::abs(v.x) > 1e8f || std::abs(v.y) > 1e8f ||
            !color(v.r, v.g, v.b, v.a))
            throw std::invalid_argument("Frame vertex requires finite coordinates and premultiplied RGBA");
    }
}
}}}

#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace lc { namespace viewer { namespace render {
// Initial private resource policy, not an accepted hardware/support floor.
constexpr int MaxFrameDimension = 16384;
constexpr std::uint64_t MaxFramePixels = 1u << 24;
constexpr std::size_t MaxFrameElements = 1u << 20;
// Private prepared-drawing boundary. Model/path preparation remains CPU-only.
// Vertices use target pixel coordinates and premultiplied SDR RGBA, not CAD doubles.
struct Vertex { float x, y, r, g, b, a; };
struct Frame {
    int width = 0, height = 0;
    std::array<float, 4> clear{{0, 0, 0, 1}}; // premultiplied, like vertex colors
    std::vector<Vertex> vertices;
    // Empty means non-indexed triangles; otherwise UInt32 triangle indices.
    std::vector<std::uint32_t> indices;
};
// Either zero dimension suspends the target; suspended frames contain no mesh.
void validateFrame(const Frame& frame);
struct Snapshot {
    int width = 0, height = 0;
    // Owned, tight RGBA8 rows, top-left origin, premultiplied alpha.
    std::vector<std::uint8_t> rgba;
    // Accepted command counts, published only after completed readback. A
    // suspended target has no pixels, no commands and targetReady == false.
    unsigned drawCount = 0, indexedDrawCount = 0;
    bool targetReady = false;
};
}}}

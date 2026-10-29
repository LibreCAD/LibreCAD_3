#pragma once
#include "painters/renderframe.h"
#include <memory>
#include <string>
#include <stdexcept>

namespace lc { namespace rhi {
enum class Api { Vulkan, Metal };
struct DeviceInfo {
    Api api;
    std::string name, deviceClass, qtVersion, shaderHash;
    int textureSizeMin = 0, textureSizeMax = 0, textureLimit = 0;
    int qrhiDeviceType = -1;
    std::string metalRegistryId, metalRegistryClass;
};
class TextureSizeLimitError : public std::invalid_argument {
public:
    static constexpr const char* code() noexcept { return "texture-size-limit"; }
    // Legacy one-argument callers assume the smallest non-suspended size is 1.
    explicit TextureSizeLimitError(int maximum) : TextureSizeLimitError(maximum, 1) {}
    TextureSizeLimitError(int maximum, int minimum)
        : std::invalid_argument(code()), maximum_(maximum), minimum_(minimum) {}
    int maximum() const noexcept { return maximum_; }
    int minimum() const noexcept { return minimum_; }
private:
    int maximum_;
    int minimum_;
};
// Private owned offscreen host. Requires a live QGuiApplication. Create, use and
// destroy on the same thread; it owns no document, widget or borrowed QRhi.
class OffscreenRenderer {
public:
    ~OffscreenRenderer();
    OffscreenRenderer(const OffscreenRenderer&) = delete;
    OffscreenRenderer& operator=(const OffscreenRenderer&) = delete;
    viewer::render::Snapshot render(const viewer::render::Frame& frame);
    DeviceInfo deviceInfo() const;
private:
    struct Impl;
    explicit OffscreenRenderer(Api api, int textureLimit);
    std::unique_ptr<Impl> impl;
    friend std::unique_ptr<OffscreenRenderer> createOffscreenRenderer(Api, int);
};
// Immutable per-instance policy, bounded by the CPU and actual device limits.
std::unique_ptr<OffscreenRenderer> createOffscreenRenderer(
    Api api, int textureLimit = viewer::render::MaxFrameDimension);
}}

#pragma once
#include <rhi/qrhi.h>
#include <cstdint>
#include <string>

namespace lc { namespace rhi { namespace detail {
// Private diagnostic policy. A name or unknown QRhi type never attests a device.
struct MetalIdentity {
    std::uint64_t registryId = 0;
    std::string registryClass;
};
inline std::string deviceClass(QRhiDriverInfo::DeviceType type, bool intelMetal,
                               const MetalIdentity& identity) {
    if (identity.registryId != 0 && identity.registryClass == "AppleParavirtAccelerator")
        return intelMetal && (type == QRhiDriverInfo::UnknownDevice || type == QRhiDriverInfo::VirtualDevice)
            ? "virtual" : "unknown"; // Contradictory native typing cannot qualify hardware/software.
    switch (type) {
    case QRhiDriverInfo::CpuDevice: return "software";
    case QRhiDriverInfo::IntegratedDevice: case QRhiDriverInfo::DiscreteDevice:
    case QRhiDriverInfo::ExternalDevice: return "hardware";
    default: break;
    }
    return "unknown";
}
}}}

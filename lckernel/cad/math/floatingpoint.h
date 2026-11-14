#pragma once

#include <cmath>
#include <cstdint>
#include <cstring>
#include <limits>

namespace lc {
namespace maths {

constexpr unsigned int DefaultComparisonUlps = 4;

enum class FloatingOrder { Less, Equivalent, Greater, Unordered };
enum class FloatingSign { Negative, Zero, Positive, NonFinite };

namespace detail {

constexpr bool hasIeeeBinary64Representation =
    sizeof(double) == sizeof(std::uint64_t) &&
    std::numeric_limits<double>::is_iec559 &&
    std::numeric_limits<double>::digits == 53 &&
    std::numeric_limits<double>::max_exponent == 1024;

inline std::uint64_t orderedBinary64(double value) noexcept {
    std::uint64_t bits;
    std::memcpy(&bits, &value, sizeof(bits));

    constexpr std::uint64_t signBit = std::uint64_t{1} << 63;
    // Reverse the ordering of negative encodings and fold the two signed
    // zeros onto one rank. Adjacent finite values then have adjacent ranks.
    return (bits & signBit) != 0 ? ~bits + std::uint64_t{1} : bits | signBit;
}

inline FloatingOrder compareByNextafter(double value, double boundary,
                                        unsigned int maxUlps) noexcept {
    const bool valueIsGreater = value > boundary;
    double steppedBoundary = boundary;
    const double direction = valueIsGreater
        ? std::numeric_limits<double>::infinity()
        : -std::numeric_limits<double>::infinity();

    for (unsigned int ulp = 0; ulp < maxUlps; ++ulp) {
        steppedBoundary = std::nextafter(steppedBoundary, direction);
        if ((valueIsGreater && steppedBoundary >= value) ||
            (!valueIsGreater && steppedBoundary <= value)) {
            return FloatingOrder::Equivalent;
        }
    }
    return valueIsGreater ? FloatingOrder::Greater : FloatingOrder::Less;
}

} // namespace detail

/** Compare doubles within maxUlps representable steps of one another.
 * NaNs are unordered; infinities compare equal only to the same infinity.
 * Signed zeros are equivalent. Equivalence is not transitive and must not
 * be used as an ordering predicate for associative containers or sorting.
 * IEEE binary64 values use a constant-time ULP distance; other representations
 * use a nextafter walk whose cost is linear in maxUlps.
 */
inline FloatingOrder compareWithinUlps(double value, double boundary,
                                       unsigned int maxUlps = DefaultComparisonUlps) noexcept {
    if (std::isnan(value) || std::isnan(boundary)) {
        return FloatingOrder::Unordered;
    }
    if (std::isinf(value) || std::isinf(boundary)) {
        if (std::isinf(value) && std::isinf(boundary)) {
            if (std::signbit(value) == std::signbit(boundary)) {
                return FloatingOrder::Equivalent;
            }
            return std::signbit(value) ? FloatingOrder::Less : FloatingOrder::Greater;
        }
        if (std::isinf(value)) {
            return std::signbit(value) ? FloatingOrder::Less : FloatingOrder::Greater;
        }
        return std::signbit(boundary) ? FloatingOrder::Greater : FloatingOrder::Less;
    }

    if (value == boundary) {
        return FloatingOrder::Equivalent;
    }

    if (detail::hasIeeeBinary64Representation) {
        const std::uint64_t valueRank = detail::orderedBinary64(value);
        const std::uint64_t boundaryRank = detail::orderedBinary64(boundary);
        const std::uint64_t distance = valueRank < boundaryRank
            ? boundaryRank - valueRank
            : valueRank - boundaryRank;
        if (distance <= maxUlps) {
            return FloatingOrder::Equivalent;
        }
        return valueRank < boundaryRank ? FloatingOrder::Less : FloatingOrder::Greater;
    }

    return detail::compareByNextafter(value, boundary, maxUlps);
}

/** Classify finite signs, treating values within maxUlps of zero as zero. */
inline FloatingSign signWithinUlpsOfZero(double value,
                                         unsigned int maxUlps = DefaultComparisonUlps) noexcept {
    if (!std::isfinite(value)) return FloatingSign::NonFinite;
    if (compareWithinUlps(value, 0., maxUlps) == FloatingOrder::Equivalent) {
        return FloatingSign::Zero;
    }
    return std::signbit(value) ? FloatingSign::Negative : FloatingSign::Positive;
}

} // namespace maths
} // namespace lc

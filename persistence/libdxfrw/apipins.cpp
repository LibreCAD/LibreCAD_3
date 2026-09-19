// The libdxfrw API values LibreCAD's behaviour depends on, pinned.
//
// The dependencies are not obvious from the call sites. DXFimpl compares
// DRW_Variant::TYPE tags to decide whether a header variable holds the type it
// asked for; writeLWPolyline down-converts on `_exportVersion <= DRW::AC1009`,
// which is a statement about the enumerators' *order*, not their names; and
// File::open reports libdxfrw's error codes to the user. A silent renumbering
// upstream would not fail to compile -- it would change what LibreCAD reads
// and writes.
//
// This lives in persistence rather than in the test suite on purpose:
// lcunittest does not link libdxfrw and cannot reach its headers, by design
// (persistence links it PRIVATE). Here they are already compiled
// at C++17, the static_asserts fail the *build* rather than a test run, and the
// fingerprint below gives the test suite something to compare without naming a
// single DRW type.

#include <string>
#include <type_traits>

#include <drw_base.h>
#include <drw_entities.h>
#include <drw_interface.h>

#include "../format.h"
#include "dxfimpl.h"

namespace {

// Revisions. The values matter because they are compared with <= and >, which
// is how "R12 or older" is expressed throughout the library and here.
static_assert(DRW::UNKNOWNV == 0, "DRW::Version renumbered");
static_assert(DRW::AC1006 == 9, "DRW::Version renumbered");
static_assert(DRW::AC1009 == 10, "DRW::Version renumbered");
static_assert(DRW::AC1012 == 11, "DRW::Version renumbered");
static_assert(DRW::AC1014 == 12, "DRW::Version renumbered");
static_assert(DRW::AC1015 == 13, "DRW::Version renumbered");
static_assert(DRW::AC1018 == 14, "DRW::Version renumbered");
static_assert(DRW::AC1021 == 15, "DRW::Version renumbered");
static_assert(DRW::AC1024 == 16, "DRW::Version renumbered");
static_assert(DRW::AC1027 == 17, "DRW::Version renumbered");
static_assert(DRW::AC1032 == 18, "DRW::Version renumbered");
static_assert(DRW::AC1009 < DRW::AC1015 && DRW::AC1015 < DRW::AC1032,
              "DRW::Version is no longer ordered oldest-first, which is what "
              "the R12 down-convert in DXFimpl::writeLWPolyline tests");

// Error codes. 13 and 14 are the two the DXF corpus actually produces.
static_assert(DRW::BAD_NONE == 0, "DRW::error renumbered");
static_assert(DRW::BAD_OPEN == 2, "DRW::error renumbered");
static_assert(DRW::BAD_READ_SECTION == 13, "DRW::error renumbered");
static_assert(DRW::BAD_CODE_PARSED == 14, "DRW::error renumbered");

// Variant tags. DXFimpl::addHeader reads the header map directly and checks
// these before dereferencing the union, so a renumbering is a wrong read of a
// live pointer, not a compile error.
static_assert(DRW_Variant::STRING == 0, "DRW_Variant::TYPE renumbered");
static_assert(DRW_Variant::INTEGER == 1, "DRW_Variant::TYPE renumbered");
static_assert(DRW_Variant::DOUBLE == 3, "DRW_Variant::TYPE renumbered");
static_assert(DRW_Variant::COORD == 4, "DRW_Variant::TYPE renumbered");
static_assert(DRW_Variant::INVALID == 6, "DRW_Variant::TYPE renumbered");

// A subclass that overrides nothing must stay abstract, and DXFimpl -- which
// overrides every pure virtual -- must stay concrete. Asserting only the second
// is one-sided: it passes just as well if DRW_Interface stopped having pure
// virtuals at all, which is exactly the change that would let a new callback
// appear unnoticed.
class OverridesNothing : public DRW_Interface {};

static_assert(std::is_abstract<OverridesNothing>::value,
              "DRW_Interface has no pure virtuals left: a new read or write "
              "callback can now be added without any implementation noticing");
static_assert(!std::is_abstract<lc::persistence::DXFimpl>::value,
              "DRW_Interface gained a pure virtual that DXFimpl does not "
              "override");

// The entity tags addHatch dispatches boundary edges on. Naming them is the
// assertion: a removal or rename fails this translation unit.
constexpr DRW::ETYPE kBoundaryKinds[] = {
    DRW::LINE, DRW::ARC, DRW::ELLIPSE, DRW::SPLINE, DRW::LWPOLYLINE,
};
static_assert(sizeof(kBoundaryKinds) / sizeof(kBoundaryKinds[0]) == 5, "");

}  // namespace

namespace lc {
namespace persistence {

std::string libdxfrwApiFingerprint() {
    // Built from the values themselves, so it cannot drift from what the
    // static_asserts above check -- and so a test can fail on a renumbering
    // without including a single libdxfrw header.
    return "version:UNKNOWNV=" + std::to_string(static_cast<int>(DRW::UNKNOWNV))
           + ",AC1009=" + std::to_string(static_cast<int>(DRW::AC1009))
           + ",AC1015=" + std::to_string(static_cast<int>(DRW::AC1015))
           + ",AC1027=" + std::to_string(static_cast<int>(DRW::AC1027))
           + ",AC1032=" + std::to_string(static_cast<int>(DRW::AC1032))
           + ";error:NONE=" + std::to_string(static_cast<int>(DRW::BAD_NONE))
           + ",OPEN=" + std::to_string(static_cast<int>(DRW::BAD_OPEN))
           + ",SECTION=" + std::to_string(static_cast<int>(DRW::BAD_READ_SECTION))
           + ",CODE=" + std::to_string(static_cast<int>(DRW::BAD_CODE_PARSED))
           + ";variant:STRING=" + std::to_string(static_cast<int>(DRW_Variant::STRING))
           + ",INTEGER=" + std::to_string(static_cast<int>(DRW_Variant::INTEGER))
           + ",DOUBLE=" + std::to_string(static_cast<int>(DRW_Variant::DOUBLE))
           + ",COORD=" + std::to_string(static_cast<int>(DRW_Variant::COORD))
           + ";interface:bare-abstract="
           + (std::is_abstract<OverridesNothing>::value ? "1" : "0")
           + ",dxfimpl-concrete="
           + (std::is_abstract<lc::persistence::DXFimpl>::value ? "0" : "1");
}

}  // namespace persistence
}  // namespace lc

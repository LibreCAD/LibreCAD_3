// parity_test.cpp — phase 2 slice 2.4 parity check for .lua vs .py.
//
// Primary exit criterion from the phase-2 sub-plan:
//   "a dump of entity count + coordinates from the .lua run and the .py
//    run of the same drawing diffs empty."
//
// This test provides the *Python half* of that diff — it produces a stable,
// language-independent representation of the document state after running
// test.py, and writes it to a text file (or stdout) that a CI job can diff
// against a matching Lua-side dump.  The Lua half will land alongside the
// full luacmdinterface CI job that actually runs `luacmdinterface -i file:test.lua`
// under xvfb — that's out of scope for this dev machine (no Lua 5.3 EXACT,
// no headless GL), but the sub-plan calls it out as a CI-only manual check
// on the Lua side.
//
// The diffable format: `<entity_class>\t<field1>=<val1>\t<field2>=<val2>...`,
// one entity per line, sorted by class then coordinates.  Any drift in
// entity count, layer name, or geometry values shows up as a plain-text diff.

#include <gtest/gtest.h>

#include <lcpython.h>

#include <cad/base/cadentity.h>
#include <cad/primitive/arc.h>
#include <cad/primitive/circle.h>
#include <cad/primitive/line.h>
#include <cad/primitive/point.h>
#include <cad/storage/documentimpl.h>
#include <cad/storage/entitycontainer.h>
#include <cad/storage/storagemanagerimpl.h>

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

namespace {

// Same path resolution as demoscript_test.cpp — env override for CI, or
// __FILE__-relative fallback (`../../luacmdinterface/test.py`).
std::string findTestPy() {
    if (const char* env = std::getenv("LIBRECAD_TEST_PY")) {
        return env;
    }
    namespace fs = std::filesystem;
    fs::path here = fs::path(__FILE__).parent_path();
    return (here.parent_path().parent_path()
            / "luacmdinterface" / "test.py").string();
}

std::string slurp(const std::string& path) {
    std::ifstream f(path);
    std::stringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

// Format a double so tiny FP jitter doesn't cause spurious diffs: fixed
// precision (6 fractional digits), no trailing zeros stripped.  Same
// precision the .lua side must adopt (documented in README.md).
std::string fmt(double d) {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(6) << d;
    return ss.str();
}

// One line per entity, class-tagged, all coords in fixed precision.  Sort
// afterwards so insertion order (Lua vs Python may differ in future) can't
// cause a false diff.  We diff on the *set* of geometry, not the order.
std::vector<std::string>
dumpEntities(const std::vector<lc::entity::CADEntity_CSPtr>& entities) {
    std::vector<std::string> out;
    out.reserve(entities.size());

    for (const auto& e : entities) {
        std::ostringstream row;
        // Layer name is the same across all: append once so the diff
        // spot-checks it.  If the .lua run somehow ended up on a different
        // layer, every line diffs.
        const auto layerName = e->layer() ? e->layer()->name() : "<null>";

        if (auto line = std::dynamic_pointer_cast<const lc::entity::Line>(e)) {
            row << "Line\tlayer=" << layerName
                << "\tstart=(" << fmt(line->start().x()) << "," << fmt(line->start().y()) << ")"
                << "\tend=("   << fmt(line->end().x())   << "," << fmt(line->end().y())   << ")";
        } else if (auto circle = std::dynamic_pointer_cast<const lc::entity::Circle>(e)) {
            row << "Circle\tlayer=" << layerName
                << "\tcenter=(" << fmt(circle->center().x()) << "," << fmt(circle->center().y()) << ")"
                << "\tradius=" << fmt(circle->radius());
        } else if (auto arc = std::dynamic_pointer_cast<const lc::entity::Arc>(e)) {
            row << "Arc\tlayer=" << layerName
                << "\tcenter=(" << fmt(arc->center().x()) << "," << fmt(arc->center().y()) << ")"
                << "\tradius="  << fmt(arc->radius())
                << "\tstart="   << fmt(arc->startAngle())
                << "\tend="     << fmt(arc->endAngle())
                << "\tccw="     << (arc->CCW() ? "true" : "false");
        } else if (auto point = std::dynamic_pointer_cast<const lc::entity::Point>(e)) {
            row << "Point\tlayer=" << layerName
                << "\tcoord=(" << fmt(point->x()) << "," << fmt(point->y()) << ")";
        } else {
            // Unknown entity type — dump the C++ typeid so a diff still shows
            // something meaningful.
            row << "Other\ttype=" << typeid(*e).name();
        }
        out.push_back(row.str());
    }

    std::sort(out.begin(), out.end());
    return out;
}

} // namespace

// -----------------------------------------------------------------------------
// The parity dump test — runs test.py, dumps entity list, verifies the shape.
//
// If LIBRECAD_PARITY_OUTFILE is set, also writes the sorted dump there so a
// CI job can pick it up for diffing against a matching test.lua dump.  This
// keeps CI's diff logic out of the test itself — the test only verifies
// self-consistency; the CI harness owns the cross-language diff.
// -----------------------------------------------------------------------------
TEST(ParityTest, TestPyProducesStableEntityDump) {
    auto sm  = std::make_shared<lc::storage::StorageManagerImpl>();
    auto doc = std::make_shared<lc::storage::DocumentImpl>(sm);

    lc::python::LCPython lcpy;
    auto ns = lcpy.makeNamespace();
    lcpy.setDocument(ns, doc);

    const std::string script = slurp(findTestPy());
    ASSERT_FALSE(script.empty());
    const std::string err = lcpy.runString(script.c_str(), ns);
    ASSERT_EQ(err, "") << err;

    const auto entities = sm->entityContainer().asVector();
    const auto lines = dumpEntities(entities);

    // Self-consistency: the dump must have as many lines as entities.
    ASSERT_EQ(lines.size(), entities.size());

    // Shape: at least one Circle, at least one Arc, and many Lines (spiral).
    // These are the entities test.py explicitly produces.
    int nLine = 0, nCircle = 0, nArc = 0, nOther = 0;
    for (const auto& row : lines) {
        if (row.rfind("Line\t", 0)   == 0) ++nLine;
        else if (row.rfind("Circle\t", 0) == 0) ++nCircle;
        else if (row.rfind("Arc\t", 0)    == 0) ++nArc;
        else                                     ++nOther;
    }
    EXPECT_EQ(nCircle, 1);
    EXPECT_EQ(nArc,    1);
    EXPECT_GE(nLine,   500);
    EXPECT_LE(nLine,   510);
    EXPECT_EQ(nOther,  0) << "Unexpected entity class in dump";

    // Diff-file output for the CI cross-language check.
    if (const char* out = std::getenv("LIBRECAD_PARITY_OUTFILE")) {
        std::ofstream f(out);
        for (const auto& row : lines) {
            f << row << '\n';
        }
    }
}

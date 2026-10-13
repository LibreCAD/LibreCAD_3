// parity_test.cpp — phase 2 slice 2.4 parity check for .lua vs .py.
//
// Primary exit criterion from the phase-2 sub-plan:
//   "a dump of entity count + coordinates from the .lua run and the .py
//    run of the same drawing diffs empty."
//
// This test runs both test.py and test.lua against independent real documents
// in-process, then compares canonical geometry, layer metadata, and undo/redo
// outcomes. This makes language parity a required local/product test rather
// than a manually diffed CI artifact.
//
// The diffable format: `<entity_class>\t<field1>=<val1>\t<field2>=<val2>...`,
// one entity per line, sorted by class then coordinates.  Any drift in
// entity count, layer name, or geometry values shows up as a plain-text diff.

#include <gtest/gtest.h>

#include <lcpython.h>
#include <lclua.h>
#include <kaguya/kaguya.hpp>

#include <cad/base/cadentity.h>
#include <cad/primitive/arc.h>
#include <cad/primitive/circle.h>
#include <cad/primitive/line.h>
#include <cad/primitive/point.h>
#include <cad/storage/documentimpl.h>
#include <cad/storage/entitycontainer.h>
#include <cad/storage/storagemanagerimpl.h>
#include <cad/storage/undomanagerimpl.h>

#include <algorithm>
#include <cstdlib>
#include <boost/filesystem.hpp>
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
    namespace fs = boost::filesystem;
    fs::path here = fs::path(__FILE__).parent_path();
    return (here.parent_path().parent_path()
            / "luacmdinterface" / "test.py").string();
}

std::string findTestLua() {
    if (const char* env = std::getenv("LIBRECAD_TEST_LUA")) {
        return env;
    }
    namespace fs = boost::filesystem;
    fs::path here = fs::path(__FILE__).parent_path();
    return (here.parent_path().parent_path()
            / "luacmdinterface" / "test.lua").string();
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
            const auto* entity = e.get();
            row << "Other\ttype=" << typeid(*entity).name();
        }
        out.push_back(row.str());
    }

    std::sort(out.begin(), out.end());
    return out;
}

std::vector<std::string>
dumpLayers(const std::shared_ptr<lc::storage::StorageManagerImpl>& manager) {
    std::vector<std::string> out;
    for (const auto& [name, layer] : manager->allLayers()) {
        const auto color = layer->color();
        std::ostringstream row;
        row << "Layer\tname=" << name
            << "\tlineWidth=" << fmt(layer->lineWidth().width())
            << "\tcolor=(" << fmt(color.red()) << "," << fmt(color.green())
            << "," << fmt(color.blue()) << "," << fmt(color.alpha()) << ")"
            << "\tfrozen=" << (layer->isFrozen() ? "true" : "false");
        const auto pattern = layer->linePattern();
        if (pattern == nullptr) {
            row << "\tlinePattern=<null>";
        } else {
            row << "\tlinePattern=" << pattern->name()
                << "\tdescription=" << pattern->description()
                << "\tlength=" << fmt(pattern->length())
                << "\tpath=";
            for (const double segment : pattern->path()) {
                row << fmt(segment) << ",";
            }
        }
        out.push_back(row.str());
    }
    std::sort(out.begin(), out.end());
    return out;
}

std::string runLuaScript(const std::string& script,
                         const lc::storage::Document_SPtr& document) {
    kaguya::State state;
    lc::lua::LCLua lua(state.state());
    lua.addLuaLibs();
    lua.importLCKernel();
    lua.setDocument(document);
    return lua.runString(script.c_str());
}

} // namespace

TEST(LuaRuntimeTest, NonStringErrorsReturnDiagnostic) {
    kaguya::State state;
    lc::lua::LCLua lua(state.state());

    EXPECT_EQ(lua.runString("error({})"),
              "Lua script failed with non-string error value of type table");
}

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

TEST(ParityTest, LuaAndPythonScriptsMatchCanonicalDocumentAndUndoRedo) {
    auto undoPy = std::make_shared<lc::storage::UndoManagerImpl>(8);
    auto smPy = std::make_shared<lc::storage::StorageManagerImpl>();
    auto docPy = std::make_shared<lc::storage::DocumentImpl>(smPy);
    docPy->commitProcessEvent().connect<lc::storage::UndoManagerImpl,
        &lc::storage::UndoManagerImpl::on_CommitProcessEvent>(undoPy.get());

    auto undoLua = std::make_shared<lc::storage::UndoManagerImpl>(8);
    auto smLua = std::make_shared<lc::storage::StorageManagerImpl>();
    auto docLua = std::make_shared<lc::storage::DocumentImpl>(smLua);
    docLua->commitProcessEvent().connect<lc::storage::UndoManagerImpl,
        &lc::storage::UndoManagerImpl::on_CommitProcessEvent>(undoLua.get());

    lc::python::LCPython python;
    auto pythonNamespace = python.makeNamespace();
    python.setDocument(pythonNamespace, docPy);
    const std::string pythonScript = slurp(findTestPy());
    ASSERT_FALSE(pythonScript.empty()) << "test.py path resolution failed";
    ASSERT_EQ(python.runString(pythonScript.c_str(), pythonNamespace), "");

    const std::string luaScript = slurp(findTestLua());
    ASSERT_FALSE(luaScript.empty()) << "test.lua path resolution failed";
    ASSERT_EQ(runLuaScript(luaScript, docLua), "");

    const auto pythonEntities = dumpEntities(smPy->entityContainer().asVector());
    const auto luaEntities = dumpEntities(smLua->entityContainer().asVector());
    const auto pythonLayers = dumpLayers(smPy);
    const auto luaLayers = dumpLayers(smLua);
    ASSERT_EQ(pythonEntities, luaEntities);
    ASSERT_EQ(pythonLayers, luaLayers);
    ASSERT_GE(pythonEntities.size(), 502u);
    ASSERT_LE(pythonEntities.size(), 512u);
    ASSERT_FALSE(pythonLayers.empty());
    ASSERT_TRUE(undoPy->canUndo());
    ASSERT_TRUE(undoLua->canUndo());

    const auto expectSameState = [&]() {
        EXPECT_EQ(dumpEntities(smPy->entityContainer().asVector()),
                  dumpEntities(smLua->entityContainer().asVector()));
        EXPECT_EQ(dumpLayers(smPy), dumpLayers(smLua));
    };

    // The scripts produce three committed operations (layer, spiral, and
    // circle+arc). Compare after each undo as well as at the empty state.
    for (int i = 0; i < 3; ++i) {
        ASSERT_TRUE(undoPy->canUndo());
        ASSERT_TRUE(undoLua->canUndo());
        undoPy->undo();
        undoLua->undo();
        expectSameState();
    }
    EXPECT_TRUE(smPy->entityContainer().asVector().empty());
    EXPECT_TRUE(smLua->entityContainer().asVector().empty());

    for (int i = 0; i < 3; ++i) {
        ASSERT_TRUE(undoPy->canRedo());
        ASSERT_TRUE(undoLua->canRedo());
        undoPy->redo();
        undoLua->redo();
        expectSameState();
    }
    EXPECT_EQ(dumpEntities(smPy->entityContainer().asVector()), pythonEntities);
    EXPECT_EQ(dumpEntities(smLua->entityContainer().asVector()), luaEntities);
    EXPECT_EQ(dumpLayers(smPy), pythonLayers);
    EXPECT_EQ(dumpLayers(smLua), luaLayers);
}

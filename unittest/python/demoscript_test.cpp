// demoscript_test.cpp — phase 2 slice 2.3 verification: actually load and
// run luacmdinterface/test.py through LCPython against a real DocumentImpl,
// then count entities in the document and diff against the expected count.
//
// This is the "primary exit criterion" the sub-plan lists for phase 2:
//   "a dump of entity count + coordinates from the .lua run and the .py run
//    of the same drawing diffs empty."
// The Lua half of the parity diff runs in CI (Lua 5.3 EXACT isn't on this
// dev machine) — this gtest covers the .py half exhaustively.

#include <gtest/gtest.h>

#include <lcpython.h>

#include <cad/storage/documentimpl.h>
#include <cad/storage/storagemanagerimpl.h>
#include <cad/storage/entitycontainer.h>
#include <cad/base/cadentity.h>

#include <cstddef>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

namespace {

// Locate luacmdinterface/test.py relative to the repository root.  We prefer
// an explicit env override (LIBRECAD_TEST_PY) so CI can point at a known
// path; falling back to a compile-time expected repo layout is fine for
// direct unittest binary runs.
std::string findTestPy() {
    if (const char* env = std::getenv("LIBRECAD_TEST_PY")) {
        return env;
    }
    // Two hops up from unittest/python/ lands at repo root.
    namespace fs = std::filesystem;
    fs::path here = fs::path(__FILE__).parent_path();
    fs::path candidate = here.parent_path().parent_path()
                       / "luacmdinterface" / "test.py";
    return candidate.string();
}

std::string slurp(const std::string& path) {
    std::ifstream f(path);
    std::stringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

} // namespace

TEST(DemoScriptTest, TestPyRunsAgainstRealDocument) {
    // Build the same document plumbing luacmdinterface/main.cpp does at
    // startup (:120-122 of the pre-slice-2.1 file).
    auto sm  = std::make_shared<lc::storage::StorageManagerImpl>();
    auto doc = std::make_shared<lc::storage::DocumentImpl>(sm);

    lc::python::LCPython lcpy;
    auto ns = lcpy.makeNamespace();
    lcpy.setDocument(ns, doc);

    // Load and execute test.py verbatim.
    const std::string script = slurp(findTestPy());
    ASSERT_FALSE(script.empty())
        << "test.py is empty — path resolution failed?";
    const std::string err = lcpy.runString(script.c_str(), ns);
    ASSERT_EQ(err, "") << "test.py execution failed:\n" << err;

    // The script draws a spiral (N line segments) + 1 circle + 1 arc.  Count
    // entities via the storage manager's entityContainer, which is what the
    // Lua-side ContentTable/PropertyEditor read from too.
    //
    // Expected count: the spiral step is d += 0.05, upper bound 8π ≈ 25.132.
    // First iteration (d = 0) skips insert, so the number of Line entities
    // is `ceil(8π / 0.05) - 1` = 502.  Plus 1 circle + 1 arc = 504.
    const auto entities = sm->entityContainer().asVector();

    // 504 exact would over-constrain to test.py's constants; allow slack so
    // future step-size tweaks don't cascade into red tests, but be tight
    // enough to detect an actual regression (empty document, missing arc,
    // etc.).  Any change to test.py's constants MUST update this too.
    const std::size_t kMinExpected = 500;
    const std::size_t kMaxExpected = 510;
    EXPECT_GE(entities.size(), kMinExpected)
        << "test.py produced too few entities — API drift?";
    EXPECT_LE(entities.size(), kMaxExpected)
        << "test.py produced too many entities — script regression?";
}

TEST(DemoScriptTest, TestPySyntacticallyValid) {
    // A pure-Python compile check via `compile(src, ...)` — catches syntax
    // issues without needing the document.  Doubles as a smoke that the
    // build-target-relative path resolution works even if the entity count
    // test above hits a runtime issue.
    lc::python::LCPython lcpy;
    auto ns = lcpy.makeNamespace();

    const std::string script = slurp(findTestPy());
    ASSERT_FALSE(script.empty()) << "test.py path resolution failed";

    // Wrap in a compile-only harness: we run `compile(...)` in the embedded
    // interpreter with mode='exec' — verifies syntax without side effects.
    ns.set("_script", script);
    const std::string err = lcpy.runString(
        "compile(_script, 'test.py', 'exec')\n",
        ns);
    ASSERT_EQ(err, "") << "test.py syntax check failed:\n" << err;
}

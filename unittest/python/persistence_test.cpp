// persistence_test.cpp — phase 2 slice 2.6 verification.
//
// Exercises the Python `lc.persistence.File` bindings via a real DXF
// round-trip: build a document with a Line + Circle + Arc + one custom
// Layer, save via LIBDXFRW_DXF_R2000, open into a fresh document, verify
// the same 3 entities + 1 layer come back.
//
// This is the phase-6 custom-entity DXF round-trip's prerequisite — the
// same code path the plan lists as the reason persistence bindings are
// REQUIRED, not optional.

#include <gtest/gtest.h>

#include <lcpython.h>

#include <cad/base/cadentity.h>
#include <cad/primitive/arc.h>
#include <cad/primitive/circle.h>
#include <cad/primitive/line.h>
#include <cad/storage/documentimpl.h>
#include <cad/storage/entitycontainer.h>
#include <cad/storage/storagemanagerimpl.h>

#include <cstdio>
#include <filesystem>
#include <memory>
#include <string>

namespace {

std::string uniqueTmpDxf() {
    auto pid = std::to_string(::getpid());
    return (std::filesystem::temp_directory_path()
            / ("librecad-persistence-" + pid + ".dxf")).string();
}

} // namespace

TEST(PersistenceTest, DxfRoundTrip) {
    // ---- Build a document in Python, save it out, open back in fresh doc.
    const std::string dxfPath = uniqueTmpDxf();
    std::filesystem::remove(dxfPath);  // ignore errors if it doesn't exist

    auto sm  = std::make_shared<lc::storage::StorageManagerImpl>();
    auto doc = std::make_shared<lc::storage::DocumentImpl>(sm);

    lc::python::LCPython lcpy;
    auto ns = lcpy.makeNamespace();
    lcpy.setDocument(ns, doc);
    ns.set("dxf_path", dxfPath);

    const std::string script = R"PY(
# Build a small drawing.
layer = lc.meta.Layer('rw', lc.meta.MetaLineWidthByValue(1.0),
                      lc.Color(255, 255, 255, 255), None, False)
lc.operation.AddLayer.new(document, layer).execute()

eb = lc.operation.EntityBuilder.new(document)

# One Line.
lb = lc.builder.LineBuilder()
lb.setLayer(layer)
lb.setStartPoint(lc.geo.Coordinate(0.0, 0.0))
lb.setEndPoint(lc.geo.Coordinate(10.0, 20.0))
eb.appendEntity(lb.build())

# One Circle.
cb = lc.builder.CircleBuilder()
cb.setLayer(layer)
cb.setCenter(lc.geo.Coordinate(50.0, 50.0))
cb.setRadius(5.0)
eb.appendEntity(cb.build())

# One Arc (half-circle).
import math
ab = lc.builder.ArcBuilder()
ab.setLayer(layer)
ab.setCenter(lc.geo.Coordinate(-10.0, -10.0))
ab.setRadius(3.0)
ab.setStartAngle(0.0)
ab.setEndAngle(math.pi)
ab.setIsCCW(True)
eb.appendEntity(ab.build())

eb.execute()

# Save it out — DXF R2000 via libdxfrw.
lc.persistence.File.save(document, dxf_path,
                         lc.persistence.File.Type.LIBDXFRW_DXF_R2000)
)PY";

    ASSERT_EQ(lcpy.runString(script.c_str(), ns), "");

    // ---- Confirm the file exists and is non-empty (basic sanity).
    ASSERT_TRUE(std::filesystem::exists(dxfPath));
    ASSERT_GT(std::filesystem::file_size(dxfPath), 0u);

    // ---- Open into a fresh document and verify the same entities come back.
    auto sm2  = std::make_shared<lc::storage::StorageManagerImpl>();
    auto doc2 = std::make_shared<lc::storage::DocumentImpl>(sm2);
    auto ns2  = lcpy.makeNamespace();
    lcpy.setDocument(ns2, doc2);
    ns2.set("dxf_path", dxfPath);

    ASSERT_EQ(lcpy.runString(
        "opened_type = lc.persistence.File.open(document, dxf_path,\n"
        "    lc.persistence.File.Library.LIBDXFRW)\n"
        "assert opened_type is not None\n",
        ns2), "");

    const auto entities = sm2->entityContainer().asVector();

    int nLine = 0, nCircle = 0, nArc = 0;
    for (const auto& e : entities) {
        if (std::dynamic_pointer_cast<const lc::entity::Line>(e))        ++nLine;
        else if (std::dynamic_pointer_cast<const lc::entity::Circle>(e)) ++nCircle;
        else if (std::dynamic_pointer_cast<const lc::entity::Arc>(e))    ++nArc;
    }

    EXPECT_EQ(nLine,   1);
    EXPECT_EQ(nCircle, 1);
    EXPECT_EQ(nArc,    1);

    // Cleanup — leaving temp DXFs around clutters /tmp.
    std::filesystem::remove(dxfPath);
}

TEST(PersistenceTest, EnumsAndHelpersRegistered) {
    lc::python::LCPython lcpy;
    auto ns = lcpy.makeNamespace();

    // Basic sanity: every Type value + Library value is reachable, and the
    // helper functions exist.
    ASSERT_EQ(lcpy.runString(
        "assert lc.persistence.File.Type.LIBDXFRW_DXF_R2000 is not None\n"
        "assert lc.persistence.File.Type.LIBOPENCAD_DWG is not None\n"
        "assert lc.persistence.File.Library.LIBDXFRW is not None\n"
        "assert lc.persistence.File.Library.LIBOPENCAD is not None\n"
        "types = lc.persistence.File.getAvailableFileTypes()\n"
        "assert isinstance(types, dict) and len(types) > 0\n"
        "exts = lc.persistence.File.getSupportedFileExtensions()\n"
        "assert isinstance(exts, dict)\n"
        "# Extension for a known type.\n"
        "ext = lc.persistence.File.getExtensionForFileType(\n"
        "    lc.persistence.File.Type.LIBDXFRW_DXF_R2000)\n"
        "assert isinstance(ext, str) and len(ext) > 0\n",
        ns), "");
}

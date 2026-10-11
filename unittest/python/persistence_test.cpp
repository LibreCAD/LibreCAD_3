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
#include <boost/filesystem.hpp>
#include <fstream>
#include <iterator>
#include <memory>
#include <string>
#include <vector>
#include <persistence/libdxfrw/dxfimpl.h>

namespace {

std::string uniqueTmpDxf() {
    return (boost::filesystem::temp_directory_path()
            / boost::filesystem::unique_path("librecad-persistence-%%%%%%%%.dxf")).string();
}

} // namespace

TEST(PersistenceTest, DxfRoundTrip) {
    // ---- Build a document in Python, save it out, open back in fresh doc.
    const std::string dxfPath = uniqueTmpDxf();
    boost::filesystem::remove(dxfPath);  // ignore errors if it doesn't exist

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
assert lc.persistence.File.save(document, dxf_path,
                                lc.persistence.File.Type.LIBDXFRW_DXF_R2000)
)PY";

    ASSERT_EQ(lcpy.runString(script.c_str(), ns), "");

    // ---- Confirm the file exists and is non-empty (basic sanity).
    ASSERT_TRUE(boost::filesystem::exists(dxfPath));
    ASSERT_GT(boost::filesystem::file_size(dxfPath), 0u);

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
    boost::filesystem::remove(dxfPath);
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

TEST(PersistenceTest, SaveFormatMatrixRoundTrip) {
    struct FormatCase {
        const char* name;
        const char* acadVersion;
        bool binary;
    };
    const std::vector<FormatCase> formats = {
        {"LIBDXFRW_DXF_R12",   "AC1009", false},
        {"LIBDXFRW_DXF_R14",   "AC1014", false},
        {"LIBDXFRW_DXF_R2000", "AC1015", false},
        {"LIBDXFRW_DXF_R2004", "AC1018", false},
        {"LIBDXFRW_DXF_R2007", "AC1021", false},
        {"LIBDXFRW_DXF_R2010", "AC1024", false},
        {"LIBDXFRW_DXF_R2013", "AC1027", false},
        {"LIBDXFRW_DXB_R12",   "AC1009", true},
        {"LIBDXFRW_DXB_R14",   "AC1014", true},
        {"LIBDXFRW_DXB_R2000", "AC1015", true},
        {"LIBDXFRW_DXB_R2004", "AC1018", true},
        {"LIBDXFRW_DXB_R2007", "AC1021", true},
        {"LIBDXFRW_DXB_R2010", "AC1024", true},
        {"LIBDXFRW_DXB_R2013", "AC1027", true},
    };

    const std::string pathBase = uniqueTmpDxf() + ".matrix";
    auto sourceStorage = std::make_shared<lc::storage::StorageManagerImpl>();
    auto sourceDocument = std::make_shared<lc::storage::DocumentImpl>(sourceStorage);
    lc::python::LCPython lcpy;
    auto sourceNamespace = lcpy.makeNamespace();
    lcpy.setDocument(sourceNamespace, sourceDocument);
    sourceNamespace.set("path_base", pathBase);

    const std::string createFixture = R"PY(
layer = lc.meta.Layer('matrix', lc.meta.MetaLineWidthByValue(1.0),
                      lc.Color(255, 255, 255, 255), None, False)
lc.operation.AddLayer.new(document, layer).execute()
eb = lc.operation.EntityBuilder.new(document)
lb = lc.builder.LineBuilder()
lb.setLayer(layer)
lb.setStartPoint(lc.geo.Coordinate(0.0, 0.0))
lb.setEndPoint(lc.geo.Coordinate(10.0, 20.0))
eb.appendEntity(lb.build())
cb = lc.builder.CircleBuilder()
cb.setLayer(layer)
cb.setCenter(lc.geo.Coordinate(50.0, 50.0))
cb.setRadius(5.0)
eb.appendEntity(cb.build())
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

available_types = lc.persistence.File.getAvailableFileTypes()
assert len(available_types) == 14, available_types
)PY";
    ASSERT_EQ(lcpy.runString(createFixture.c_str(), sourceNamespace), "");

    const std::string binarySignature("AutoCAD Binary DXF\r\n\x1a\0",
                                      sizeof("AutoCAD Binary DXF\r\n\x1a\0") - 1);
    std::vector<std::string> paths;
    paths.reserve(formats.size());

    for (const auto& format : formats) {
        const std::string path = pathBase + "." + format.name + ".dxf";
        paths.push_back(path);
        boost::filesystem::remove(path);

        auto saveNamespace = lcpy.makeNamespace();
        lcpy.setDocument(saveNamespace, sourceDocument);
        saveNamespace.set("dxf_path", path);
        saveNamespace.set("format_name", std::string(format.name));
        const std::string saveScript =
            "file_type = getattr(lc.persistence.File.Type, format_name)\n"
            "assert file_type in lc.persistence.File.getAvailableFileTypes()\n"
            "assert lc.persistence.File.save(document, dxf_path, file_type)\n";
        ASSERT_EQ(lcpy.runString(saveScript.c_str(), saveNamespace), "")
            << "save failed for " << format.name;

        ASSERT_TRUE(boost::filesystem::exists(path)) << format.name;
        ASSERT_GT(boost::filesystem::file_size(path), 0u) << format.name;
        std::ifstream input(path, std::ios::binary);
        ASSERT_TRUE(input.good()) << path;
        const std::string bytes((std::istreambuf_iterator<char>(input)),
                                std::istreambuf_iterator<char>());
        ASSERT_FALSE(bytes.empty()) << format.name;

        if (format.binary) {
            EXPECT_EQ(bytes.compare(0, binarySignature.size(), binarySignature), 0)
                << format.name << " should have the binary DXF signature";
        } else {
            EXPECT_NE(bytes.compare(0, binarySignature.size(), binarySignature), 0)
                << format.name << " should be text DXF";
            EXPECT_NE(bytes.find("$ACADVER"), std::string::npos) << format.name;
        }
        EXPECT_NE(bytes.find(format.acadVersion), std::string::npos)
            << format.name << " should write $ACADVER=" << format.acadVersion;

        auto reopenedStorage = std::make_shared<lc::storage::StorageManagerImpl>();
        auto reopenedDocument = std::make_shared<lc::storage::DocumentImpl>(reopenedStorage);

        // File::open currently exposes the libdxfrw return value only as a
        // File::Type. Probe the parser's bool/error channel here as well so a
        // silent read failure cannot masquerade as a successful empty import.
        auto probeDocument = std::make_shared<lc::storage::DocumentImpl>(
            std::make_shared<lc::storage::StorageManagerImpl>());
        auto probeBuilder = std::make_shared<lc::operation::Builder>(
            probeDocument, "DXF serialization matrix parser probe");
        lc::persistence::DXFimpl probeInterface(probeDocument, probeBuilder);
        dxfRW probeReader(path.c_str());
        const bool readSucceeded = probeReader.read(&probeInterface, true);
        ASSERT_TRUE(readSucceeded)
            << format.name << " libdxfrw read failed with error "
            << static_cast<int>(probeReader.getError());

        auto openNamespace = lcpy.makeNamespace();
        lcpy.setDocument(openNamespace, reopenedDocument);
        openNamespace.set("dxf_path", path);
        openNamespace.set("format_name", std::string(format.name));
        ASSERT_EQ(lcpy.runString(
            "opened_type = lc.persistence.File.open(document, dxf_path,\n"
            "    lc.persistence.File.Library.LIBDXFRW)\n"
            "expected_type = getattr(lc.persistence.File.Type, format_name)\n"
            "assert opened_type == expected_type, (opened_type, expected_type)\n",
            openNamespace), "") << "open failed for " << format.name;

        int lineCount = 0;
        int circleCount = 0;
        int arcCount = 0;
        for (const auto& entity : reopenedStorage->entityContainer().asVector()) {
            if (std::dynamic_pointer_cast<const lc::entity::Line>(entity)) {
                ++lineCount;
            } else if (std::dynamic_pointer_cast<const lc::entity::Circle>(entity)) {
                ++circleCount;
            } else if (std::dynamic_pointer_cast<const lc::entity::Arc>(entity)) {
                ++arcCount;
            }
        }
        EXPECT_EQ(lineCount, 1) << format.name;
        EXPECT_EQ(circleCount, 1) << format.name;
        EXPECT_EQ(arcCount, 1) << format.name;
    }

    for (const auto& path : paths) {
        boost::filesystem::remove(path);
    }
}

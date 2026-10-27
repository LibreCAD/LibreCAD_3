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
#include <persistence/file.h>

#include <cad/base/cadentity.h>
#include <cad/primitive/arc.h>
#include <cad/primitive/circle.h>
#include <cad/primitive/line.h>
#include <cad/storage/documentimpl.h>
#include <cad/storage/entitycontainer.h>
#include <cad/storage/storagemanagerimpl.h>

#include <boost/filesystem.hpp>
#include <fstream>
#include <iterator>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace {

std::string uniqueTmpDxf() {
    return (boost::filesystem::temp_directory_path()
            / boost::filesystem::unique_path("librecad-persistence-%%%%%%%%-%%%%%%%%.dxf"))
        .string();
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
lc.persistence.File.save(document, dxf_path,
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

// The result types, from Python.
//
// `lc.persistence.File.open` answers a read with a revision and nothing else,
// which is enough for a script that only wants to load one known-good file and
// no use at all to one walking a directory: it cannot tell a complete drawing
// from the readable half of a broken one. importFile gives it the same facts
// LibreCAD itself now acts on.
TEST(PersistenceTest, ImportResultFromPython) {
    const std::string dxfPath = uniqueTmpDxf();
    boost::filesystem::remove(dxfPath);

    auto sm  = std::make_shared<lc::storage::StorageManagerImpl>();
    auto doc = std::make_shared<lc::storage::DocumentImpl>(sm);

    lc::python::LCPython lcpy;
    auto ns = lcpy.makeNamespace();
    lcpy.setDocument(ns, doc);
    ns.set("dxf_path", dxfPath);

    const std::string build = R"PY(
layer = lc.meta.Layer('rw', lc.meta.MetaLineWidthByValue(1.0),
                      lc.Color(255, 255, 255, 255), None, False)
lc.operation.AddLayer.new(document, layer).execute()

eb = lc.operation.EntityBuilder.new(document)
lb = lc.builder.LineBuilder()
lb.setLayer(layer)
lb.setStartPoint(lc.geo.Coordinate(0.0, 0.0))
lb.setEndPoint(lc.geo.Coordinate(4.0, 3.0))
eb.appendEntity(lb.build())
eb.execute()

export = lc.persistence.File.exportFile(document, dxf_path,
                                        lc.persistence.File.Type.LIBDXFRW_DXF_R2000)
assert export.ok is True, 'export should have succeeded'
assert export.variantId == 'dxf.ac1015.ascii', export.variantId
assert list(export.diagnostics) == []
)PY";
    ASSERT_EQ(lcpy.runString(build.c_str(), ns), "") << "building and exporting failed";

    auto sm2  = std::make_shared<lc::storage::StorageManagerImpl>();
    auto doc2 = std::make_shared<lc::storage::DocumentImpl>(sm2);
    auto ns2 = lcpy.makeNamespace();
    lcpy.setDocument(ns2, doc2);
    ns2.set("dxf_path", dxfPath);
    ns2.set("missing_path", dxfPath + ".missing");

    const std::string read = R"PY(
result = lc.persistence.File.importFile(document, dxf_path,
                                        lc.persistence.File.Library.LIBDXFRW)

assert result.ok is True, 'a file we just wrote must read back'
assert result.partial is False
assert result.variantId == 'dxf.ac1015.ascii', result.variantId
assert result.sourceVersionTag == 'AC1015', result.sourceVersionTag
assert result.entitiesDelivered >= 1, result.entitiesDelivered
assert list(result.diagnostics) == []
assert result.loss.empty() is True
assert result.loss.total() == 0

# The variant table is readable too, so a script can offer the same choices
# the Save dialog does without hard-coding them.
variants = lc.persistence.formatVariants()
assert len(variants) >= 14
ascii_2000 = lc.persistence.formatVariantById('dxf.ac1015.ascii')
assert ascii_2000 is not None
assert ascii_2000.writable is True
assert ascii_2000.binary is False
assert ascii_2000.versionTag == 'AC1015'
assert lc.persistence.formatVariantById('dxf.nope') is None

assert lc.persistence.File.variantIdForType(
    lc.persistence.File.Type.LIBDXFRW_DXF_R2000) == 'dxf.ac1015.ascii'
assert lc.persistence.File.typeForVariantId('dxf.ac1015.ascii') == \
    lc.persistence.File.Type.LIBDXFRW_DXF_R2000
assert lc.persistence.File.typeForVariantId('dxf.nope') is None

# A file that is not there is reported, not raised, and not confused with an
# empty drawing.
missing = lc.persistence.File.importFile(document, missing_path,
                                         lc.persistence.File.Library.LIBDXFRW)
assert missing.ok is False
assert missing.partial is False
assert missing.entitiesDelivered == 0
assert len(missing.diagnostics) >= 1
assert missing.diagnostics[0].severity == lc.persistence.Severity.Error
assert missing.diagnostics[0].code != ''
)PY";
    ASSERT_EQ(lcpy.runString(read.c_str(), ns2), "") << "reading back failed";

    boost::filesystem::remove(dxfPath);
}

TEST(PersistenceTest, ImportFailuresAreTypedReadonlyAndRetainTheirLifetime) {
    lc::python::LCPython python;
    auto ns = python.makeNamespace();
    lc::persistence::ImportResult result;
    result.failures.push_back({"HATCH", 0xffffffffUL, "Layer", "Block", "Invalid geometry"});
    ns.set("result", std::move(result));
    ASSERT_EQ(python.runString(R"PY(
failure = result.failures[0]
assert isinstance(failure, lc.persistence.ImportFailure)
assert (failure.recordKind, failure.handle, failure.layer, failure.block, failure.reason) == \
       ('HATCH', 4294967295, 'Layer', 'Block', 'Invalid geometry')
for obj, attr, value in [(failure, 'recordKind', 'LINE'), (failure, 'handle', 0),
                         (failure, 'layer', ''), (failure, 'block', ''),
                         (failure, 'reason', ''), (result, 'failures', [])]:
    try:
        setattr(obj, attr, value)
    except AttributeError:
        pass
    else:
        raise AssertionError('writable result field: ' + attr)
del obj, result
import gc
gc.collect()
assert failure.reason == 'Invalid geometry'
)PY", ns), "");
}

TEST(PersistenceTest, DwgImportIdentityMatchesEnabledAndDisabledBuilds) {
    lc::python::LCPython python;
    auto ns = python.makeNamespace();
    python.setDocument(ns, std::make_shared<lc::storage::DocumentImpl>(
        std::make_shared<lc::storage::StorageManagerImpl>()));
    lc::storage::Document_SPtr legacy = std::make_shared<lc::storage::DocumentImpl>(
        std::make_shared<lc::storage::StorageManagerImpl>());
    ns.set("legacy_document", legacy);
    ns.set("dwg_path", std::string(DWG_FIXTURES_DIR) + "/ordinary_enc_AC1015.dwg");
    ns.set("enabled", USE_DWG_IMPORT != 0);
    ASSERT_EQ(python.runString(R"PY(
F = lc.persistence.File
assert int(F.Type.LIBDXFRW_DWG_IMPORT) == 15
assert F.LIBDXFRW_DWG_IMPORT == F.Type.LIBDXFRW_DWG_IMPORT
assert F.variantIdForType(F.Type.LIBDXFRW_DWG_IMPORT) == 'dwg.libdxfrw'
assert F.typeForVariantId('dwg.libdxfrw') == F.Type.LIBDXFRW_DWG_IMPORT
result = F.importFile(document, dwg_path, F.Library.LIBDXFRW)
assert result.ok == enabled
assert result.variantId == 'dwg.libdxfrw'
assert result.failures == []
assert result.entitiesDelivered > 0 if enabled else len(result.diagnostics) > 0
assert F.open(legacy_document, dwg_path, F.Library.LIBDXFRW) == F.Type.LIBDXFRW_DWG_IMPORT
)PY", ns), "");
}

TEST(PersistenceTest, AllWritableDxfVariantsRoundTripThroughPython) {
    struct FormatCase {
        const char* id;
        const char* versionTag;
        bool binary;
    };
    const std::vector<FormatCase> formats = {
        {"dxf.ac1009.ascii",  "AC1009", false},
        {"dxf.ac1014.ascii",  "AC1014", false},
        {"dxf.ac1015.ascii",  "AC1015", false},
        {"dxf.ac1018.ascii",  "AC1018", false},
        {"dxf.ac1021.ascii",  "AC1021", false},
        {"dxf.ac1024.ascii",  "AC1024", false},
        {"dxf.ac1027.ascii",  "AC1027", false},
        {"dxf.ac1009.binary", "AC1009", true},
        {"dxf.ac1014.binary", "AC1014", true},
        {"dxf.ac1015.binary", "AC1015", true},
        {"dxf.ac1018.binary", "AC1018", true},
        {"dxf.ac1021.binary", "AC1021", true},
        {"dxf.ac1024.binary", "AC1024", true},
        {"dxf.ac1027.binary", "AC1027", true},
    };

    const std::string pathBase = uniqueTmpDxf() + ".variants";
    auto sourceStorage = std::make_shared<lc::storage::StorageManagerImpl>();
    auto sourceDocument = std::make_shared<lc::storage::DocumentImpl>(sourceStorage);
    lc::python::LCPython lcpy;
    auto sourceNamespace = lcpy.makeNamespace();
    lcpy.setDocument(sourceNamespace, sourceDocument);

    const std::string createFixture = R"PY(
layer = lc.meta.Layer('variants', lc.meta.MetaLineWidthByValue(1.0),
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

writable_dxf = [v for v in lc.persistence.formatVariants()
                if v.formatId == 'dxf' and v.writable]
assert len(writable_dxf) == 14, [v.id for v in writable_dxf]
)PY";
    ASSERT_EQ(lcpy.runString(createFixture.c_str(), sourceNamespace), "");

    const std::string binarySignature("AutoCAD Binary DXF\r\n\x1a\0",
                                      sizeof("AutoCAD Binary DXF\r\n\x1a\0") - 1);
    std::vector<std::string> paths;
    paths.reserve(formats.size());

    for (const auto& format : formats) {
        const std::string path = pathBase + "." + format.id + ".dxf";
        paths.push_back(path);
        boost::filesystem::remove(path);

        auto exportNamespace = lcpy.makeNamespace();
        lcpy.setDocument(exportNamespace, sourceDocument);
        exportNamespace.set("dxf_path", path);
        exportNamespace.set("format_id", std::string(format.id));
        exportNamespace.set("version_tag", std::string(format.versionTag));
        exportNamespace.set("is_binary", format.binary);
        const std::string exportScript = R"PY(
variant = lc.persistence.formatVariantById(format_id)
assert variant is not None, format_id
assert variant.writable is True, format_id
assert variant.readable is True, format_id
assert variant.versionTag == version_tag, (format_id, variant.versionTag)
assert variant.binary is is_binary, (format_id, variant.binary)
file_type = lc.persistence.File.typeForVariantId(format_id)
assert file_type is not None, format_id
assert file_type in lc.persistence.File.getAvailableFileTypes(), format_id
export_result = lc.persistence.File.exportFile(document, dxf_path, file_type)
assert export_result.ok is True, list(export_result.diagnostics)
assert export_result.variantId == format_id, export_result.variantId
assert export_result.loss.total() == 0, export_result.loss.droppedByType
assert list(export_result.diagnostics) == [], list(export_result.diagnostics)
)PY";
        ASSERT_EQ(lcpy.runString(exportScript.c_str(), exportNamespace), "")
            << "Python export failed for " << format.id;

        ASSERT_TRUE(boost::filesystem::exists(path)) << format.id;
        ASSERT_GT(boost::filesystem::file_size(path), 0u) << format.id;
        std::ifstream input(path, std::ios::binary);
        ASSERT_TRUE(input.good()) << path;
        const std::string bytes((std::istreambuf_iterator<char>(input)),
                                std::istreambuf_iterator<char>());
        ASSERT_FALSE(bytes.empty()) << format.id;
        if (format.binary) {
            EXPECT_EQ(bytes.compare(0, binarySignature.size(), binarySignature), 0)
                << format.id << " should have the binary DXF signature";
        } else {
            EXPECT_NE(bytes.compare(0, binarySignature.size(), binarySignature), 0)
                << format.id << " should be ASCII DXF";
            EXPECT_NE(bytes.find("$ACADVER"), std::string::npos) << format.id;
            EXPECT_NE(bytes.find(format.versionTag), std::string::npos) << format.id;
        }

        auto importedStorage = std::make_shared<lc::storage::StorageManagerImpl>();
        auto importedDocument = std::make_shared<lc::storage::DocumentImpl>(importedStorage);
        auto importNamespace = lcpy.makeNamespace();
        lcpy.setDocument(importNamespace, importedDocument);
        importNamespace.set("dxf_path", path);
        importNamespace.set("format_id", std::string(format.id));
        importNamespace.set("version_tag", std::string(format.versionTag));
        const std::string importScript = R"PY(
result = lc.persistence.File.importFile(
    document, dxf_path, lc.persistence.File.Library.LIBDXFRW)
assert result.ok is True, list(result.diagnostics)
assert result.partial is False
assert result.variantId == format_id, result.variantId
assert result.sourceVersionTag == version_tag, result.sourceVersionTag
assert result.entitiesDelivered == 3, result.entitiesDelivered
assert list(result.diagnostics) == [], list(result.diagnostics)
assert result.loss.total() == 0, result.loss.droppedByType
)PY";
        ASSERT_EQ(lcpy.runString(importScript.c_str(), importNamespace), "")
            << "Python import failed for " << format.id;

        int lines = 0;
        int circles = 0;
        int arcs = 0;
        for (const auto& entity : importedStorage->entityContainer().asVector()) {
            if (std::dynamic_pointer_cast<const lc::entity::Line>(entity)) {
                ++lines;
            } else if (std::dynamic_pointer_cast<const lc::entity::Circle>(entity)) {
                ++circles;
            } else if (std::dynamic_pointer_cast<const lc::entity::Arc>(entity)) {
                ++arcs;
            }
        }
        EXPECT_EQ(lines, 1) << format.id;
        EXPECT_EQ(circles, 1) << format.id;
        EXPECT_EQ(arcs, 1) << format.id;

        // Keep the legacy File.open API's enum result in sync with the richer
        // stable variant ID, especially for binary files that share a DXF
        // revision with their ASCII counterpart.
        auto openedStorage = std::make_shared<lc::storage::StorageManagerImpl>();
        auto openedDocument = std::make_shared<lc::storage::DocumentImpl>(openedStorage);
        auto openNamespace = lcpy.makeNamespace();
        lcpy.setDocument(openNamespace, openedDocument);
        openNamespace.set("dxf_path", path);
        openNamespace.set("format_id", std::string(format.id));
        const std::string openScript = R"PY(
opened_type = lc.persistence.File.open(
    document, dxf_path, lc.persistence.File.Library.LIBDXFRW)
expected_type = lc.persistence.File.typeForVariantId(format_id)
assert opened_type == expected_type, (opened_type, expected_type)
)PY";
        ASSERT_EQ(lcpy.runString(openScript.c_str(), openNamespace), "")
            << "legacy Python File.open returned the wrong type for " << format.id;
        EXPECT_EQ(openedStorage->entityContainer().asVector().size(), 3u) << format.id;
    }

    for (const auto& path : paths) {
        boost::filesystem::remove(path);
    }
}

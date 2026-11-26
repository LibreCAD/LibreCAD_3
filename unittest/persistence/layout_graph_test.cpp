// SPDX-License-Identifier: GPL-2.0-or-later
#include <libdxfrw.h>
#include <drw_interface.h>
#include <gtest/gtest.h>
#include <cad/storage/documentimpl.h>
#include <cad/storage/storagemanagerimpl.h>
#include <cad/meta/block.h>
#include <cad/operations/builder.h>
#include <cad/operations/entitybuilder.h>
#include <cad/primitive/line.h>
#include "persistence/file.h"
#include "persistence/libdxfrw/dxfimpl.h"
#include "persistence/libdxfrw/preservedrecords.h"
#include <algorithm>
#include <cctype>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <sstream>

#if defined(LIBDXFRW_HAS_DXF_STAGED_LAYOUT_LINK_INSPECTOR) && defined(LIBDXFRW_HAS_DXF_FIXED_PAPER_SPACE_WRITER)
namespace {
using File = lc::persistence::File;
using Document = lc::storage::DocumentImpl;
std::shared_ptr<Document> makeDocument() {
    return std::make_shared<Document>(std::make_shared<lc::storage::StorageManagerImpl>());
}
std::string bytes(const std::filesystem::path& path) {
    std::ifstream in(path, std::ios::binary);
    return {std::istreambuf_iterator<char>(in), {}};
}
std::string groupInRecord(const std::string& contents, const std::string& kind,
                          int identityCode, const std::string& identity, int wantedCode) {
    std::istringstream in(contents); std::string code, value;
    bool record = false, match = false; std::string wanted;
    while (std::getline(in, code) && std::getline(in, value)) {
        const int group = std::stoi(code);
        if (group == 0) {
            if (record && match) return wanted;
            record = value == kind; match = false; wanted.clear();
        } else if (record) {
            if (group == identityCode && value == identity) match = true;
            if (group == wantedCode) wanted = value;
        }
    }
    return record && match ? wanted : std::string{};
}
struct Scratch {
    std::filesystem::path path = std::filesystem::temp_directory_path() /
        ("lc-layout-graph-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    Scratch() { std::filesystem::create_directories(path); }
    ~Scratch() { std::error_code error; std::filesystem::remove_all(path, error); }
};
void insertPaperLine(const std::shared_ptr<Document>& doc, const char* name) {
    auto line = std::make_shared<lc::entity::Line>(lc::geo::Coordinate(2,3,0), lc::geo::Coordinate(7,11,0),
        doc->layerByName("0"), nullptr, doc->blockByName(name));
    auto builder = std::make_shared<lc::operation::Builder>(doc, "paper line");
    auto entities = std::make_shared<lc::operation::EntityBuilder>(doc);
    entities->appendEntity(line); builder->append(entities); builder->execute();
}
void expectPaperLine(const std::shared_ptr<Document>& doc, const char* name) {
    auto paper = doc->blockByName(name); ASSERT_TRUE(paper);
    const auto entities = doc->entitiesByBlock(paper).asVector(); ASSERT_EQ(entities.size(), 1u);
    auto line = std::dynamic_pointer_cast<const lc::entity::Line>(entities.front()); ASSERT_TRUE(line);
    EXPECT_EQ(line->start().x(), 2); EXPECT_EQ(line->start().y(), 3);
    EXPECT_EQ(line->end().x(), 7); EXPECT_EQ(line->end().y(), 11);
}
TEST(LayoutGraph, SavesReopensAndRetriesWithoutLosingPaperContents) {
    Scratch dir; auto first = dir.path / "first.dxf", second = dir.path / "second.dxf";
    auto doc = makeDocument(); insertPaperLine(doc, "*Paper_Space"); insertPaperLine(doc, "*Paper_Space0");
    ASSERT_TRUE(File::exportFile(doc, first.string(), File::LIBDXFRW_DXF_R2004).ok);
    auto input = bytes(first); ASSERT_NE(input.find("  0\nLAYOUT\n"), std::string::npos);
    auto reopened = makeDocument(); auto read = File::importFile(reopened, first.string(), File::LIBDXFRW);
    ASSERT_TRUE(read.ok); EXPECT_EQ(read.loss.droppedByType.count("LAYOUT"), 0u);
    expectPaperLine(reopened, "*Paper_Space"); expectPaperLine(reopened, "*Paper_Space0");
    auto extra = std::make_shared<lc::meta::Block>("*Paper_Space9"); reopened->addDocumentMetaType(extra);
    { std::ofstream out(second); out << "preserve destination"; }
    EXPECT_FALSE(File::exportFile(reopened, second.string(), File::LIBDXFRW_DXF_R2004).ok);
    EXPECT_EQ(bytes(second), "preserve destination");
    reopened->removeDocumentMetaType(extra);
    ASSERT_TRUE(File::exportFile(reopened, second.string(), File::LIBDXFRW_DXF_R2004).ok);
    auto again = makeDocument(); auto reread = File::importFile(again, second.string(), File::LIBDXFRW);
    ASSERT_TRUE(reread.ok); EXPECT_EQ(reread.loss.droppedByType.count("LAYOUT"), 0u);
    expectPaperLine(again, "*Paper_Space"); expectPaperLine(again, "*Paper_Space0");
}
TEST(LayoutGraph, LegacyRefusalSupportsSameWriterRetry) {
    Scratch dir; const auto output = dir.path / "legacy.dxf";
    auto doc = makeDocument(); insertPaperLine(doc, "*Paper_Space0");
    auto paper = doc->blockByName("*Paper_Space0");
    auto children = doc->entitiesByBlock(paper).asVector(); ASSERT_EQ(children.size(), 1u);
    { std::ofstream out(output); out << "keep original"; }
    lc::persistence::DXFimpl writer(doc);
    EXPECT_FALSE(writer.writeDXF(output.string(), File::LIBDXFRW_DXF_R12));
    EXPECT_NE(writer.writeFailureReason().find("paper-space"), std::string::npos);
    EXPECT_EQ(bytes(output), "keep original");
    doc->removeEntity(children.front());
    ASSERT_TRUE(writer.writeDXF(output.string(), File::LIBDXFRW_DXF_R12));
    EXPECT_TRUE(writer.writeFailureReason().empty());
    EXPECT_NE(bytes(output), "keep original");
}
TEST(LayoutGraph, EmptyBinaryDrawingDoesNotIntroduceLayoutWarnings) {
    const File::Type targets[] = {File::LIBDXFRW_DXB_R2000, File::LIBDXFRW_DXB_R2004,
        File::LIBDXFRW_DXB_R2007, File::LIBDXFRW_DXB_R2010, File::LIBDXFRW_DXB_R2013};
    for (const auto target : targets) {
        Scratch dir; const auto output = dir.path / "empty.dxf";
        ASSERT_TRUE(File::exportFile(makeDocument(), output.string(), target).ok);
        EXPECT_EQ(bytes(output).find("*Paper_Space0"), std::string::npos);
        const auto read = File::importFile(makeDocument(), output.string(), File::LIBDXFRW);
        ASSERT_TRUE(read.ok); EXPECT_EQ(read.loss.droppedByType.count("LAYOUT"), 0u);
    }
}
TEST(LayoutGraph, HeaderlessImportedPaperBlockIsNotOmitted) {
    Scratch dir; const auto source = dir.path / "source.dxf", output = dir.path / "output.dxf";
    { std::ofstream out(source); out << "0\nSECTION\n2\nBLOCKS\n0\nBLOCK\n2\n*Paper_Space0\n70\n0\n10\n0\n20\n0\n30\n0\n0\nENDBLK\n0\nENDSEC\n0\nEOF\n"; }
    auto doc = makeDocument(); ASSERT_TRUE(File::importFile(doc, source.string(), File::LIBDXFRW).ok);
    ASSERT_TRUE(File::exportFile(doc, output.string(), File::LIBDXFRW_DXF_R12).ok);
    auto written = bytes(output);
    std::transform(written.begin(), written.end(), written.begin(),
                   [](unsigned char ch) { return static_cast<char>(std::toupper(ch)); });
    EXPECT_NE(written.find("*PAPER_SPACE0"), std::string::npos);
}
#if USE_DWG_IMPORT && defined(DWG_FIXTURES_DIR)
TEST(LayoutGraph, DwgImportRecordsProvenance) {
    auto doc = makeDocument();
    const auto result = File::importFile(doc, std::string(DWG_FIXTURES_DIR) + "/ordinary_enc_AC1015.dwg", File::LIBDXFRW);
    ASSERT_TRUE(result.ok);
    const auto meta = doc->allMetaTypes();
    const auto it = meta.find(lc::persistence::PreservedRecords::kId); ASSERT_NE(it, meta.end());
    const auto source = std::dynamic_pointer_cast<const lc::persistence::PreservedRecords>(it->second);
    ASSERT_TRUE(source); EXPECT_TRUE(source->sourceWasDwg);
    insertPaperLine(doc, "*Paper_Space");
    Scratch dir; const auto output = dir.path / "paper.dxf";
    { std::ofstream out(output); out << "keep original"; }
    lc::persistence::DXFimpl writer(doc);
    EXPECT_FALSE(writer.writeDXF(output.string(), File::LIBDXFRW_DXF_R2004));
    EXPECT_NE(writer.writeFailureReason().find("layout graph"), std::string::npos);
    EXPECT_EQ(bytes(output), "keep original");
    const auto children = doc->entitiesByBlock(doc->blockByName("*Paper_Space")).asVector();
    ASSERT_EQ(children.size(), 1u); doc->removeEntity(children.front());
    ASSERT_TRUE(writer.writeDXF(output.string(), File::LIBDXFRW_DXF_R2004));
}
#endif
TEST(LayoutGraph, BinaryLayoutsReopenSaveAndRetry) {
    const File::Type targets[] = {File::LIBDXFRW_DXB_R2000, File::LIBDXFRW_DXB_R2004,
        File::LIBDXFRW_DXB_R2007, File::LIBDXFRW_DXB_R2010, File::LIBDXFRW_DXB_R2013};
    for (const auto target : targets) {
        Scratch dir; const auto output = dir.path / "drawing.dxf", replay = dir.path / "replay.dxf";
        auto doc = makeDocument(); insertPaperLine(doc, "*Paper_Space"); insertPaperLine(doc, "*Paper_Space0");
        const auto written = File::exportFile(doc, output.string(), target);
        ASSERT_TRUE(written.ok); EXPECT_TRUE(written.loss.empty());
        auto reopened = makeDocument();
        const auto read = File::importFile(reopened, output.string(), File::LIBDXFRW);
        ASSERT_TRUE(read.ok); EXPECT_EQ(read.loss.droppedByType.count("LAYOUT"), 0u);
        expectPaperLine(reopened, "*Paper_Space"); expectPaperLine(reopened, "*Paper_Space0");
        auto extra = std::make_shared<lc::meta::Block>("*Paper_Space9"); reopened->addDocumentMetaType(extra);
        { std::ofstream out(replay); out << "keep original"; }
        lc::persistence::DXFimpl writer(reopened);
        EXPECT_FALSE(writer.writeDXF(replay.string(), target));
        EXPECT_EQ(bytes(replay), "keep original");
        reopened->removeDocumentMetaType(extra);
        ASSERT_TRUE(writer.writeDXF(replay.string(), target));
        auto again = makeDocument(); const auto reread = File::importFile(again, replay.string(), File::LIBDXFRW);
        ASSERT_TRUE(reread.ok); EXPECT_EQ(reread.loss.droppedByType.count("LAYOUT"), 0u);
        expectPaperLine(again, "*Paper_Space"); expectPaperLine(again, "*Paper_Space0");
    }
}
TEST(LayoutGraph, BinaryOtherBlockKeepsLayoutUnverified) {
    Scratch dir; const auto source = dir.path / "source.dxf";
    auto doc = makeDocument(); insertPaperLine(doc, "*Paper_Space0");
    doc->addDocumentMetaType(std::make_shared<lc::meta::Block>("OTHER_BLOCK"));
    ASSERT_TRUE(File::exportFile(doc, source.string(), File::LIBDXFRW_DXB_R2004).ok);
    auto reopened = makeDocument(); const auto read = File::importFile(reopened, source.string(), File::LIBDXFRW);
    ASSERT_TRUE(read.ok); EXPECT_EQ(read.loss.droppedByType.count("LAYOUT"), 1u);
    expectPaperLine(reopened, "*Paper_Space0");
}
TEST(LayoutGraph, BinaryUnknownObjectKeepsLayoutUnverifiedAndRefusesSave) {
    Scratch dir; const auto source = dir.path / "source.dxf", changed = dir.path / "changed.dxf", output = dir.path / "output.dxf";
    auto doc = makeDocument(); insertPaperLine(doc, "*Paper_Space0");
    ASSERT_TRUE(File::exportFile(doc, source.string(), File::LIBDXFRW_DXB_R2004).ok);
    auto input = bytes(source);
    const std::string endSection("\0\0ENDSEC\0", 9);
    const auto at = input.rfind(endSection); ASSERT_NE(at, std::string::npos);
    std::string xrecord;
    const auto stringGroup = [&](unsigned char code, const char* value) {
        xrecord.push_back(static_cast<char>(code)); xrecord.push_back(0); xrecord += value; xrecord.push_back(0);
    };
    stringGroup(0, "XRECORD"); stringGroup(5, "ABC");
    xrecord.push_back(74); xrecord.push_back(1); xrecord += "C"; xrecord.push_back(0); // 330
    stringGroup(100, "AcDbXrecord"); xrecord.push_back(24); xrecord.push_back(1); xrecord.push_back(1); xrecord.push_back(0); // 280
    input.insert(at, xrecord); { std::ofstream out(changed, std::ios::binary); out.write(input.data(), input.size()); }
    auto imported = makeDocument(); const auto read = File::importFile(imported, changed.string(), File::LIBDXFRW);
    ASSERT_TRUE(read.ok); EXPECT_EQ(read.loss.droppedByType.count("LAYOUT"), 1u);
    { std::ofstream out(output); out << "keep original"; }
    EXPECT_FALSE(File::exportFile(imported, output.string(), File::LIBDXFRW_DXB_R2004).ok);
    EXPECT_EQ(bytes(output), "keep original");
}
TEST(LayoutGraph, UnrecognizedLayoutSourceRemainsReported) {
    Scratch dir; auto original = dir.path / "first.dxf", changed = dir.path / "changed.dxf";
    auto originalDoc = makeDocument(); insertPaperLine(originalDoc, "*Paper_Space0");
    ASSERT_TRUE(File::exportFile(originalDoc, original.string(), File::LIBDXFRW_DXF_R2004).ok);
    auto input = bytes(original); const auto layout = input.find("  0\nLAYOUT\n"); ASSERT_NE(layout, std::string::npos);
    const auto subclass = input.find("100\nAcDbPlotSettings\n", layout); ASSERT_NE(subclass, std::string::npos);
    input.insert(subclass, "102\n{ACAD_REACTORS\n330\nC\n102\n}\n"); { std::ofstream out(changed); out << input; }
    auto imported = makeDocument(); const auto read = File::importFile(imported, changed.string(), File::LIBDXFRW);
    ASSERT_TRUE(read.ok); EXPECT_EQ(read.loss.droppedByType.count("LAYOUT"), 1u);
    expectPaperLine(imported, "*Paper_Space0");
    auto destination = dir.path / "refused.dxf"; { std::ofstream out(destination); out << "keep original"; }
    const auto refused = File::exportFile(imported, destination.string(), File::LIBDXFRW_DXF_R2004);
    EXPECT_FALSE(refused.ok); ASSERT_FALSE(refused.diagnostics.empty());
    EXPECT_NE(refused.diagnostics.front().message.find("layout graph"), std::string::npos);
    EXPECT_EQ(bytes(destination), "keep original");
}
TEST(LayoutGraph, UnmodeledPaperBlockMetadataRemainsReported) {
    Scratch dir; auto original = dir.path / "first.dxf", changed = dir.path / "changed.dxf";
    ASSERT_TRUE(File::exportFile(makeDocument(), original.string(), File::LIBDXFRW_DXF_R2004).ok);
    auto input = bytes(original); const auto block = input.find("  0\nBLOCK\n  5\n1C\n"); ASSERT_NE(block, std::string::npos);
    const auto subclass = input.find("100\nAcDbBlockBegin\n", block); ASSERT_NE(subclass, std::string::npos);
    input.insert(subclass, " 62\n     1\n"); { std::ofstream out(changed); out << input; }
    auto read = File::importFile(makeDocument(), changed.string(), File::LIBDXFRW);
    ASSERT_TRUE(read.ok); EXPECT_EQ(read.loss.droppedByType.count("LAYOUT"), 1u);
}
TEST(LayoutGraph, ExternalPlotResourceRemainsReported) {
    Scratch dir; auto original = dir.path / "first.dxf", changed = dir.path / "changed.dxf";
    ASSERT_TRUE(File::exportFile(makeDocument(), original.string(), File::LIBDXFRW_DXF_R2004).ok);
    auto input = bytes(original); const auto layout = input.find("  0\nLAYOUT\n"); ASSERT_NE(layout, std::string::npos);
    const auto printer = input.find("  2\n\n", layout); ASSERT_NE(printer, std::string::npos);
    input.insert(printer + 4, "MissingPrinter.pc3"); { std::ofstream out(changed); out << input; }
    auto read = File::importFile(makeDocument(), changed.string(), File::LIBDXFRW);
    ASSERT_TRUE(read.ok); EXPECT_EQ(read.loss.droppedByType.count("LAYOUT"), 1u);
}
TEST(LayoutGraph, RemapsReferencesToThePreservedPaperBlockBoundary) {
    Scratch dir; auto original = dir.path / "original.dxf", source = dir.path / "source.dxf", output = dir.path / "output.dxf";
    ASSERT_TRUE(File::exportFile(makeDocument(), original.string(), File::LIBDXFRW_DXF_R2004).ok);
    auto data = bytes(original);
    const auto oldBegin = groupInRecord(data, "BLOCK", 2, "*Paper_Space0", 5); ASSERT_FALSE(oldBegin.empty());
    const auto oldOwner = groupInRecord(data, "BLOCK", 2, "*Paper_Space0", 330); ASSERT_FALSE(oldOwner.empty());
    const auto oldEnd = groupInRecord(data, "ENDBLK", 330, oldOwner, 5); ASSERT_FALSE(oldEnd.empty());
    const auto root = data.find("  0\nDICTIONARY\n  5\nC\n"); ASSERT_NE(root, std::string::npos);
    const auto rootEnd = data.find("  0\n", root + 6); ASSERT_NE(rootEnd, std::string::npos);
    data.insert(rootEnd, "  3\nLC_TEST\n350\nABC\n");
    const auto end = data.rfind("  0\nENDSEC\n"); ASSERT_NE(end, std::string::npos);
    data.insert(end, "  0\nXRECORD\n  5\nABC\n330\nC\n100\nAcDbXrecord\n280\n     1\n340\n" + oldBegin + "\n341\n" + oldEnd + "\n");
    { std::ofstream out(source); out << data; }
    auto doc = makeDocument(); const auto imported = File::importFile(doc, source.string(), File::LIBDXFRW);
    ASSERT_TRUE(imported.ok); EXPECT_EQ(imported.loss.droppedByType.count("LAYOUT"), 0u);
    ASSERT_TRUE(File::exportFile(doc, output.string(), File::LIBDXFRW_DXF_R2004).ok);
    const auto result = bytes(output);
    const auto newBegin = groupInRecord(result, "BLOCK", 2, "*Paper_Space0", 5);
    const auto newOwner = groupInRecord(result, "BLOCK", 2, "*Paper_Space0", 330);
    const auto newEnd = groupInRecord(result, "ENDBLK", 330, newOwner, 5);
    ASSERT_FALSE(newBegin.empty()); EXPECT_NE(newBegin, oldBegin);
    EXPECT_EQ(groupInRecord(result, "XRECORD", 5, "ABC", 340), newBegin);
    ASSERT_FALSE(newEnd.empty()); EXPECT_NE(newEnd, oldEnd);
    EXPECT_EQ(groupInRecord(result, "XRECORD", 5, "ABC", 341), newEnd);
}
}
#endif

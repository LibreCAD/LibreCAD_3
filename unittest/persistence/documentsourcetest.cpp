// The Save decision, tested where it can be: outside the Qt class.
//
// lcUI cannot be link-tested here -- it needs GLEW and GLFW -- so the rule that
// decides whether Save may overwrite the file a document came from lives in
// persistence as a free function over plain data. What is left in CadMdiChild
// is the part that genuinely needs a window: showing the message and opening
// the file dialog.

#include <string>

#include <gtest/gtest.h>

#include "persistence/documentsource.h"

namespace {

lc::persistence::DocumentSource openedFrom(const char* path, const char* variantId) {
    lc::persistence::DocumentSource source;
    source.hasPath = true;
    source.path = path;
    source.variantId = variantId;
    source.writable = true;
    return source;
}

}  // namespace

// NOLINTNEXTLINE(readability-identifier-naming)
TEST(DocumentSourceTest, ACleanReadSavesBackToItself) {
    for (const char* variantId : {"dxf.ac1009.ascii", "dxf.ac1027.ascii", "dxf.ac1015.binary"}) {
        const auto decision = decideSave(openedFrom("/drawings/plan.dxf", variantId));

        EXPECT_EQ(decision.action, lc::persistence::SaveAction::WriteToPath) << variantId;
        EXPECT_EQ(decision.path, "/drawings/plan.dxf");
        EXPECT_EQ(decision.variantId, variantId)
            << "Save must keep the drawing in the revision and encoding it arrived in.";
        EXPECT_TRUE(decision.reason.empty()) << "Nothing to explain when Save just saves.";
    }
}

// NOLINTNEXTLINE(readability-identifier-naming)
TEST(DocumentSourceTest, ADrawingWithNoFileIsSavedAs) {
    lc::persistence::DocumentSource fresh;

    const auto decision = decideSave(fresh);

    EXPECT_EQ(decision.action, lc::persistence::SaveAction::AskForTarget);
    EXPECT_FALSE(decision.reason.empty()) << "The user is being asked; say why.";
    EXPECT_TRUE(decision.path.empty());
}

// The case the old File::Type could not express at all. A file that fails
// half-way is still loaded up to the failure, and looks like a drawing.
// Overwriting its own file replaces it with the part that was readable.
//
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(DocumentSourceTest, APartialReadIsNeverWrittenBackOverItsSource) {
    auto source = openedFrom("/drawings/broken.dxf", "dxf.ac1015.ascii");
    source.partial = true;

    const auto decision = decideSave(source);

    EXPECT_EQ(decision.action, lc::persistence::SaveAction::AskForTarget);
    EXPECT_NE(decision.reason.find("partly read"), std::string::npos) << decision.reason;
    EXPECT_TRUE(decision.path.empty())
        << "A decision to ask must not carry a path a caller could use anyway.";
}

// NOLINTNEXTLINE(readability-identifier-naming)
TEST(DocumentSourceTest, AFormatWeCannotWriteIsSavedAs) {
    auto dwg = openedFrom("/drawings/plan.dwg", "dwg");
    dwg.writable = false;

    const auto decision = decideSave(dwg);
    EXPECT_EQ(decision.action, lc::persistence::SaveAction::AskForTarget);
    EXPECT_FALSE(decision.reason.empty());

    // An id nothing in the table knows is treated the same way, rather than
    // trusted because the document claimed it.
    auto unknown = openedFrom("/drawings/plan.dxf", "dxf.ac9999.ascii");
    EXPECT_EQ(decideSave(unknown).action, lc::persistence::SaveAction::AskForTarget);
}

// NOLINTNEXTLINE(readability-identifier-naming)
TEST(DocumentSourceTest, SourceFollowsFromTheImportResult) {
    lc::persistence::ImportResult clean;
    clean.ok = true;
    clean.variantId = "dxf.ac1015.ascii";

    const auto fromClean = lc::persistence::sourceFromImport("/drawings/plan.dxf", clean);
    EXPECT_TRUE(fromClean.hasPath);
    EXPECT_FALSE(fromClean.partial);
    EXPECT_TRUE(fromClean.writable);
    EXPECT_EQ(decideSave(fromClean).action, lc::persistence::SaveAction::WriteToPath);

    lc::persistence::ImportResult partial;
    partial.ok = false;
    partial.partial = true;
    partial.entitiesDelivered = 12;
    partial.variantId = "dxf.ac1015.ascii";

    const auto fromPartial = lc::persistence::sourceFromImport("/drawings/plan.dxf", partial);
    EXPECT_TRUE(fromPartial.partial);
    EXPECT_EQ(decideSave(fromPartial).action, lc::persistence::SaveAction::AskForTarget);

    // A read that failed without delivering anything is not "partial", but it
    // is not a complete reading of the file either, and must not overwrite it.
    lc::persistence::ImportResult failed;
    failed.ok = false;
    failed.variantId = "dxf.ac1015.ascii";

    const auto fromFailed = lc::persistence::sourceFromImport("/drawings/plan.dxf", failed);
    EXPECT_EQ(decideSave(fromFailed).action, lc::persistence::SaveAction::AskForTarget);
}

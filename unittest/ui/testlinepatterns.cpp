#include <gtest/gtest.h>

#include <cad/meta/dxflinepattern.h>

#include <widgets/linepatternselect.h>

#include "uitests.h"
#include "dialogs/addlinepatterndialog.h"

TEST(LinePatternTest, Creation) {
    QApplication app(argc, argv);
    auto document = createDocument();
    ASSERT_EQ(0, document->linePatterns().size()) << "Document already contains line patterns";


    auto linePattern = std::make_shared<const lc::meta::DxfLinePatternByValue>("LP", "Line Pattern", std::vector<double> {1, -1, 0, -1}, 3);

    AddLinePatternDialogTest linePatternDialog(document);

    linePatternDialog.setName(linePattern->name());
    linePatternDialog.setDescription(linePattern->description());
    linePatternDialog.setPath(linePattern->path());

    linePatternDialog.create();

    ASSERT_EQ(1, document->linePatterns().size()) << "Line pattern was not added to document";

    auto addedLinePattern = document->linePatterns()[0];

    EXPECT_EQ(linePattern->name(), addedLinePattern->name());
    EXPECT_EQ(linePattern->description(), addedLinePattern->description());
    EXPECT_EQ(linePattern->lcPattern(), addedLinePattern->lcPattern());
}

TEST(LinePatternTest, Edition) {
    QApplication app(argc, argv);
    auto document = createDocument();

    auto linePattern = std::make_shared<const lc::meta::DxfLinePatternByValue>("LP", "Line Pattern", std::vector<double> {1, -1, 0, -1}, 3);
    auto newLinePattern = std::make_shared<const lc::meta::DxfLinePatternByValue>("NLP", "New Line Pattern", std::vector<double> {0, -1, 1, -1}, 3);

    auto operation = std::make_shared<lc::operation::AddLinePattern>(document, linePattern);
    operation->execute();

    ASSERT_EQ(1, document->linePatterns().size()) << "Line pattern was not added to the document";

    AddLinePatternDialogTest dialog(document, linePattern);
    EXPECT_EQ(linePattern->name(), dialog.name());
    EXPECT_EQ(linePattern->description(), dialog.description());
    EXPECT_EQ(linePattern->path(), dialog.path());

    dialog.setName(newLinePattern->name());
    dialog.setDescription(newLinePattern->description());
    dialog.setPath(newLinePattern->path());

    dialog.edit();

    ASSERT_EQ(1, document->linePatterns().size()) << "Line pattern was not edited";
    auto documentLinePattern = document->linePatterns()[0];

    EXPECT_EQ(newLinePattern->name(), documentLinePattern->name());
    EXPECT_EQ(newLinePattern->description(), documentLinePattern->description());
    EXPECT_EQ(newLinePattern->path(), documentLinePattern->path());
}

TEST(LinePatternTest, Selection) {
    QApplication app(argc, argv);
    auto document = createDocument();

    auto linePatternSelect = new lc::ui::widgets::LinePatternSelect(document);
    EXPECT_EQ(3, linePatternSelect->count()) << "Select should contains New/Manage/Separator";
    delete linePatternSelect;

    linePatternSelect = new lc::ui::widgets::LinePatternSelect(document, 0, true, true);
    EXPECT_EQ(5, linePatternSelect->count()) << "Select should contains New/Manage/Separator/ByBlock/ByLayer";

    auto linePattern1 = std::make_shared<const lc::meta::DxfLinePatternByValue>("LP", "Line Pattern", std::vector<double> {1, -1, 0, -1}, 3);
    auto linePattern2 = std::make_shared<const lc::meta::DxfLinePatternByValue>("NLP", "New Line Pattern", std::vector<double> {0, -1, 1, -1}, 3);

    auto operation = std::make_shared<lc::operation::AddLinePattern>(document, linePattern1);
    operation->execute();
    operation = std::make_shared<lc::operation::AddLinePattern>(document, linePattern2);
    operation->execute();

    EXPECT_EQ(7, linePatternSelect->count()) << "Line patterns were not added to select";

    linePatternSelect->setCurrentText(linePattern1->name().c_str());
    EXPECT_EQ(linePattern1, linePatternSelect->linePattern()) << "Selected line pattern wasn't returned";

    linePatternSelect->setCurrentText(linePattern2->name().c_str());
    EXPECT_EQ(linePattern2, linePatternSelect->linePattern()) << "Selected line pattern wasn't returned";

    delete linePatternSelect;
}
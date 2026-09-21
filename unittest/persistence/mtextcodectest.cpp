// The escape language an MTEXT's text is written in, both directions.
//
// These are pure string functions with no libdxfrw type in their signatures,
// which is why this test can include the header at all: persistence links
// libdxfrw PRIVATE and lcunittest cannot reach its headers.

#include <gtest/gtest.h>

#include <string>
#include <vector>

#include <libdxfrw/mtextcodec.h>

using lc::persistence::mtextToPlain;
using lc::persistence::plainToMText;
using lc::persistence::textToPlain;

namespace {

const char* kDegree = "\xC2\xB0";
const char* kNbsp = "\xC2\xA0";

}  // namespace

// The headline defect: an imported MTEXT showed \P between its lines.
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(MTextCodecTest, ParagraphBreaksBecomeNewlines) {
    EXPECT_EQ(mtextToPlain("one\\Ptwo\\Pthree"), "one\ntwo\nthree");
    EXPECT_EQ(mtextToPlain("\\Pleading"), "\nleading");
    EXPECT_EQ(mtextToPlain("trailing\\P"), "trailing\n");
}

// And the inverse, which is what makes a save round-trip.
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(MTextCodecTest, NewlinesBecomeParagraphBreaks) {
    EXPECT_EQ(plainToMText("one\ntwo"), "one\\Ptwo");
    EXPECT_EQ(plainToMText("a\r\nb"), "a\\Pb") << "a CR beside the LF is not a second break";
}

// The trap that makes the two halves have to be written together: a user's
// own backslash must not be read as the start of an escape.
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(MTextCodecTest, ALiteralBackslashSurvivesBothDirections) {
    EXPECT_EQ(plainToMText("C:\\Path"), "C:\\\\Path");
    EXPECT_EQ(mtextToPlain("C:\\\\Path"), "C:\\Path")
        << "the escaped backslash must not be seen as the \\P of a paragraph break";
    EXPECT_EQ(mtextToPlain(plainToMText("C:\\Path")), "C:\\Path");
}

// Braces group formatting and are not text; escaped, they are.
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(MTextCodecTest, BracesGroupUnlessEscaped) {
    EXPECT_EQ(mtextToPlain("{plain}"), "plain");
    EXPECT_EQ(mtextToPlain("\\{kept\\}"), "{kept}");
    EXPECT_EQ(plainToMText("{kept}"), "\\{kept\\}");
}

// The most common shape in a file AutoCAD wrote. Stripping is what makes it
// legible; the text inside must survive intact.
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(MTextCodecTest, FormattingRunsAreStrippedAndTheirTextKept) {
    EXPECT_EQ(mtextToPlain("{\\fArial|b0|i0|c0|p34;Hello}"), "Hello");
    EXPECT_EQ(mtextToPlain("{\\H2.5x;Big} normal"), "Big normal");
    EXPECT_EQ(mtextToPlain("{\\C1;red} black"), "red black");
    EXPECT_EQ(mtextToPlain("\\LUnderlined\\l done"), "Underlined done");
}

// A missing terminator must cost a formatting code, not the text after it.
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(MTextCodecTest, AnUnterminatedCodeDoesNotEatTheLine) {
    EXPECT_EQ(mtextToPlain("\\H2xWord"), "Word")
        << "the argument ends where it stops looking like one";
    EXPECT_EQ(mtextToPlain("\\C1 text"), " text");
}

// %%d and friends are a TEXT convention too, which is why both decoders share
// them and why a dimension typed as 45%%d has never read correctly.
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(MTextCodecTest, PercentFormsDecodeInBothTextKinds) {
    EXPECT_EQ(mtextToPlain("45%%d"), std::string("45") + kDegree);
    EXPECT_EQ(textToPlain("45%%d"), std::string("45") + kDegree);
    EXPECT_EQ(textToPlain("45%%D"), std::string("45") + kDegree) << "case insensitive";
    EXPECT_EQ(textToPlain("%%u still"), "%%u still") << "underline toggles are not ours";
}

// ^J is a line feed, so the caret forms have to be read before anything
// reasons about where the lines are.
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(MTextCodecTest, CaretFormsDecode) {
    EXPECT_EQ(mtextToPlain("a^Ib"), "a\tb");
    EXPECT_EQ(mtextToPlain("a^Jb"), "a\nb");
    // Caret-space is a literal caret, and the space is the escape marker, so
    // it goes with it: (32 - 64) mod 126 == 94 == '^', which is what ezdxf's
    // caret_decode computes too.
    EXPECT_EQ(mtextToPlain("a^ b"), "a^b");
    // Unlike ezdxf, the formula is not applied to every following character --
    // `^a` would become `!` under a literal reading, and silently rewriting a
    // letter in the middle of a drawing's text is worse than leaving a rare
    // caret alone.
    EXPECT_EQ(mtextToPlain("a^zb"), "a^zb");
}

// \~ is the non-breaking space; left alone it reads as two stray characters.
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(MTextCodecTest, NonBreakingSpaceDecodes) {
    EXPECT_EQ(mtextToPlain("a\\~b"), std::string("a") + kNbsp + "b");
}

// The property that matters for a save: whatever a user typed, writing it and
// reading it back has to give the same thing.
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(MTextCodecTest, EncodeThenDecodeIsIdentity) {
    const std::vector<std::string> corpus = {
        "plain",
        "two\nlines",
        "C:\\Path\\to\\file",
        "{braced}",
        "a \\P that the user typed",
        "trailing backslash \\",
        "",
        "\n",
        "mixed {\\H2x;} \\ and \n newline",
    };
    for (const auto& original : corpus) {
        EXPECT_EQ(mtextToPlain(plainToMText(original)), original)
            << "round trip changed: [" << original << "]";
    }
}

// The one place the round trip is deliberately not an identity, and it is not
// a defect: in DXF `%%d` *is* the degree sign, so a user who types those four
// characters and saves gets a degree sign back, exactly as AutoCAD does. There
// is no escape for a literal `%%d`, so there is nothing to preserve it with.
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(MTextCodecTest, PercentFormsAreDeliberatelyNotRoundTripped) {
    EXPECT_EQ(mtextToPlain(plainToMText("45%%d")), std::string("45") + kDegree);
    // Once decoded, the degree sign itself survives any number of round trips.
    const std::string decoded = std::string("45") + kDegree;
    EXPECT_EQ(mtextToPlain(plainToMText(decoded)), decoded);
}

// Malformed input must not throw: the alternative is a drawing that will not
// open at all.
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(MTextCodecTest, MalformedInputIsSurvivable) {
    EXPECT_NO_THROW(mtextToPlain("\\"));
    EXPECT_NO_THROW(mtextToPlain("^"));
    EXPECT_NO_THROW(mtextToPlain("%%"));
    EXPECT_NO_THROW(mtextToPlain("{\\f"));
    EXPECT_EQ(mtextToPlain("\\"), "\\") << "a lone trailing backslash is kept";
}

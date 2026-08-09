// dispatch_test.cpp — phase 2 slice 2.1 unit test for the URL-suffix
// runtime chooser.  Header-only helper, so no linkage dependencies —
// buildable and runnable everywhere gtest is present.

#include <gtest/gtest.h>

#include <luacmdinterface/dispatch.h>

using lc::cli::ScriptRuntime;
using lc::cli::chooseScriptRuntime;
using lc::cli::stripUrlTail;
using lc::cli::endsWithNoCase;

TEST(DispatchTest, BasicLuaAndPython) {
    EXPECT_EQ(chooseScriptRuntime("file:test.lua"), ScriptRuntime::Lua);
    EXPECT_EQ(chooseScriptRuntime("file:test.py"),  ScriptRuntime::Python);
    EXPECT_EQ(chooseScriptRuntime("test.lua"),      ScriptRuntime::Lua);
    EXPECT_EQ(chooseScriptRuntime("test.py"),       ScriptRuntime::Python);
}

TEST(DispatchTest, UnknownSuffix) {
    EXPECT_EQ(chooseScriptRuntime("file:test.txt"), ScriptRuntime::Unknown);
    EXPECT_EQ(chooseScriptRuntime(""),              ScriptRuntime::Unknown);
    EXPECT_EQ(chooseScriptRuntime("file:noext"),    ScriptRuntime::Unknown);
    EXPECT_EQ(chooseScriptRuntime("file:test.rb"),  ScriptRuntime::Unknown);
}

TEST(DispatchTest, StripsQueryBeforeMatching) {
    // Naive `rfind(".py")` on the whole URL would trip here — the ?flavor…
    // and #fragment must not defeat the suffix check. This is the exact case
    // the sub-plan calls out ("strip query/fragment before matching").
    EXPECT_EQ(chooseScriptRuntime("file:test.py?flavor=old"),  ScriptRuntime::Python);
    EXPECT_EQ(chooseScriptRuntime("file:test.lua?x=1"),        ScriptRuntime::Lua);
    EXPECT_EQ(chooseScriptRuntime("http://s/foo.py?a=1&b=2"),  ScriptRuntime::Python);
}

TEST(DispatchTest, StripsFragmentBeforeMatching) {
    EXPECT_EQ(chooseScriptRuntime("file:test.py#top"),    ScriptRuntime::Python);
    EXPECT_EQ(chooseScriptRuntime("file:test.lua#end"),   ScriptRuntime::Lua);
    // Fragment before query (per RFC 3986 the fragment is always last, but
    // callers may pass odd inputs — verify both trims happen).
    EXPECT_EQ(chooseScriptRuntime("file:test.py#a?b"),    ScriptRuntime::Python);
    EXPECT_EQ(chooseScriptRuntime("file:test.py?x=1#y"),  ScriptRuntime::Python);
}

TEST(DispatchTest, CaseInsensitiveSuffix) {
    EXPECT_EQ(chooseScriptRuntime("file:test.PY"),    ScriptRuntime::Python);
    EXPECT_EQ(chooseScriptRuntime("file:test.LUA"),   ScriptRuntime::Lua);
    EXPECT_EQ(chooseScriptRuntime("file:Test.Py"),    ScriptRuntime::Python);
}

TEST(DispatchTest, StripUrlTailHelper) {
    EXPECT_EQ(stripUrlTail("file:test.py?x=1"), "file:test.py");
    EXPECT_EQ(stripUrlTail("file:test.py#top"), "file:test.py");
    EXPECT_EQ(stripUrlTail("file:test.py"),     "file:test.py");
    EXPECT_EQ(stripUrlTail(""),                 "");
    EXPECT_EQ(stripUrlTail("no-tail"),          "no-tail");
    // Fragment before query: RFC 3986 says fragment is always last, but if a
    // caller sends `foo.py#frag?q` we still strip both.
    EXPECT_EQ(stripUrlTail("foo.py#frag?q"),    "foo.py");
}

TEST(DispatchTest, EndsWithNoCaseHelper) {
    EXPECT_TRUE(endsWithNoCase("test.py", ".py"));
    EXPECT_TRUE(endsWithNoCase("test.PY", ".py"));
    EXPECT_TRUE(endsWithNoCase("test.Py", ".pY"));
    EXPECT_FALSE(endsWithNoCase("test.py", ".lua"));
    EXPECT_FALSE(endsWithNoCase(".py", ".py.zip"));  // suffix longer than string
    EXPECT_TRUE(endsWithNoCase(".py", ".py"));       // exact
}

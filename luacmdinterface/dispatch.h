#pragma once

// -----------------------------------------------------------------------------
// dispatch.h — URL-path-suffix based script-runtime chooser for
// luacmdinterface. Slice 2.1 of the phase-2 sub-plan.
//
// Header-only + pure so it can be exercised by a standalone unit test WITHOUT
// pulling in the full luacmdinterface build (which requires Lua 5.3 EXACT,
// libcurl, GLFW3, and a real GL context). The coordinator's independent
// verification found that phase-1 slice 1.12's exit-criteria gtest suite had
// never been actually built + run before I claimed it passed; the fix is to
// give every slice a real, buildable, runnable verification harness. This
// header is that harness for slice 2.1.
//
// Contract:
//   - Input is a *URL* (`-i file:test.py`, `-i http://…/foo.py?x=1#frag`, ...)
//     — the plan's coupling reference confirms libcurl is what fetches it.
//   - The Lua-side suffix check that a naive `rfind(".py")` on the whole URL
//     would trip on is the query/fragment: `file:test.py?flavor=old` must map
//     to Python, not fall through to Unknown.
//   - Matching is case-insensitive on the suffix so `.PY` / `.LUA` work.
//   - Unknown → caller emits a helpful error to stderr and exits 1.
// -----------------------------------------------------------------------------

#include <algorithm>
#include <cctype>
#include <string>

namespace lc {
namespace cli {

enum class ScriptRuntime {
    Lua,
    Python,
    Unknown,
};

/// Strip URL query (`?…`) and fragment (`#…`) from the path component so
/// downstream suffix matching sees only the file part.
///
/// Order matters: the fragment starts at the first `#` in the URL, so trim it
/// first; the query starts at the first `?` in what remains. Applying the
/// trims in the other order is fine for well-formed URLs but is not
/// well-defined for oddly-quoted inputs. We use the RFC-3986 order.
inline std::string stripUrlTail(const std::string& url) {
    std::string s = url;
    const auto hashPos = s.find('#');
    if (hashPos != std::string::npos) {
        s.erase(hashPos);
    }
    const auto questionPos = s.find('?');
    if (questionPos != std::string::npos) {
        s.erase(questionPos);
    }
    return s;
}

/// Case-insensitive check: does @p s end with @p suffix?
inline bool endsWithNoCase(const std::string& s, const std::string& suffix) {
    if (suffix.size() > s.size()) {
        return false;
    }
    return std::equal(
        suffix.rbegin(), suffix.rend(), s.rbegin(),
        [](unsigned char a, unsigned char b) {
            return std::tolower(a) == std::tolower(b);
        });
}

/// Pick a runtime by URL path suffix. Strips query/fragment first so
/// `file:foo.py?x=1#bar` picks Python (not Unknown).
inline ScriptRuntime chooseScriptRuntime(const std::string& url) {
    const std::string cleaned = stripUrlTail(url);
    if (endsWithNoCase(cleaned, ".py")) {
        return ScriptRuntime::Python;
    }
    if (endsWithNoCase(cleaned, ".lua")) {
        return ScriptRuntime::Lua;
    }
    return ScriptRuntime::Unknown;
}

} // namespace cli
} // namespace lc

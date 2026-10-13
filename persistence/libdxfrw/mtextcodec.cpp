#include "mtextcodec.h"

#include <cstddef>

namespace lc {
namespace persistence {

namespace {

/// The degree, diameter and plus-minus signs, UTF-8, as the %% forms mean them.
constexpr const char* kDegree = "\xC2\xB0";      // U+00B0
constexpr const char* kDiameter = "\xE2\x8C\x80"; // U+2300
constexpr const char* kPlusMinus = "\xC2\xB1";   // U+00B1

bool equalsIgnoringCase(char c, char lower) {
    return c == lower || c == (lower - ('a' - 'A'));
}

/// The `%%x` overscore/underscore/degree family, shared by TEXT and MTEXT.
///
/// Returns true and advances `i` past the sequence when one was consumed.
bool takePercentForm(const std::string& in, std::size_t& i, std::string& out) {
    if (i + 2 >= in.size() || in[i] != '%' || in[i + 1] != '%') {
        return false;
    }
    const char which = in[i + 2];
    if (equalsIgnoringCase(which, 'd')) {
        out += kDegree;
    } else if (equalsIgnoringCase(which, 'c')) {
        out += kDiameter;
    } else if (equalsIgnoringCase(which, 'p')) {
        out += kPlusMinus;
    } else {
        return false;  // %%u and friends toggle underline; not ours to decide
    }
    i += 3;
    return true;
}

/// `^X` control forms. `^I` is a tab and `^J` a line feed, which is why this
/// has to run before anything reasons about where the lines are.
bool takeCaretForm(const std::string& in, std::size_t& i, std::string& out) {
    if (in[i] != '^' || i + 1 >= in.size()) {
        return false;
    }
    const char next = in[i + 1];
    if (next == ' ') {
        out += '^';       // `^` followed by a space is a literal caret
        i += 2;
        return true;
    }
    // The classic caret notation: the character 64 below the letter.
    //
    // Narrower than ezdxf, which applies (c - 64) mod 126 to whatever follows
    // the caret -- that turns `^a` into `!`. Rewriting a letter in the middle
    // of a drawing's text is a worse failure than leaving a rare caret alone,
    // so only the range that caret notation actually covers is decoded.
    if (next >= '?' && next <= '_') {
        const char control = static_cast<char>((next - 64) & 0x7F);
        if (control == '\r') {
            i += 2;       // a bare CR is noise in a text body
            return true;
        }
        out += control;
        i += 2;
        return true;
    }
    return false;
}

/// Consume a formatting introducer and its argument, emitting nothing.
///
/// The terminator is the trap. It is a semicolon for `\H`, `\C`, `\c`, `\A`,
/// `\W`, `\Q` and `\T` -- but the DXF specification makes it OPTIONAL, and the
/// switches `\L \l \O \o \K \k` have no argument at all. A rule that ate
/// everything up to the next semicolon would delete visible text whenever one
/// was omitted, which is a worse failure than leaking a digit.
///
/// So: switches consume two characters, argument-taking codes consume their
/// argument only while it still looks like an argument, and a semicolon is
/// consumed when it is the very next thing.
bool takeFormattingCode(const std::string& in, std::size_t& i) {
    if (in[i] != '\\' || i + 1 >= in.size()) {
        return false;
    }
    const char code = in[i + 1];

    // Switches: no argument.
    if (code == 'L' || code == 'l' || code == 'O' || code == 'o'
        || code == 'K' || code == 'k' || code == 'X') {
        i += 2;
        return true;
    }

    // Everything else here takes an argument.
    if (code != 'H' && code != 'W' && code != 'Q' && code != 'T'
        && code != 'A' && code != 'C' && code != 'c' && code != 'p'
        && code != 'f' && code != 'F' && code != 'S') {
        return false;
    }

    std::size_t j = i + 2;

    if (code == 'S') {
        // Stacked text: `\S<upper><divider><lower>;`. Keep the parts, drop the
        // stacking -- losing the numerator of a fraction is worse than losing
        // its arrangement.
        return false;
    }

    if (code == 'f' || code == 'F' || code == 'p') {
        // A font or paragraph specification runs to its semicolon; these two
        // always have one, and their bodies contain characters that would
        // otherwise end the scan early.
        while (j < in.size() && in[j] != ';') {
            j++;
        }
    } else {
        // A numeric or flag argument: digits, sign, decimal point, and the
        // trailing `x` of a relative height. Anything else ends it.
        while (j < in.size()
               && ((in[j] >= '0' && in[j] <= '9') || in[j] == '.'
                   || in[j] == '-' || in[j] == '+' || in[j] == 'x'
                   || in[j] == 'X')) {
            j++;
        }
    }

    if (j < in.size() && in[j] == ';') {
        j++;
    }
    i = j;
    return true;
}

}  // namespace

std::string mtextToPlain(const std::string& dxfText) {
    std::string out;
    out.reserve(dxfText.size());

    for (std::size_t i = 0; i < dxfText.size();) {
        const char c = dxfText[i];

        if (c == '^' && takeCaretForm(dxfText, i, out)) {
            continue;
        }
        if (c == '%' && takePercentForm(dxfText, i, out)) {
            continue;
        }

        if (c == '\\') {
            if (i + 1 < dxfText.size()) {
                const char next = dxfText[i + 1];
                // The escapes, first and unconditionally: a `\\` must never be
                // read as the start of a `\P`.
                if (next == '\\' || next == '{' || next == '}') {
                    out += next;
                    i += 2;
                    continue;
                }
                if (next == 'P') {
                    out += '\n';
                    i += 2;
                    continue;
                }
                if (next == '~') {
                    out += "\xC2\xA0";  // U+00A0 no-break space
                    i += 2;
                    continue;
                }
                if (takeFormattingCode(dxfText, i)) {
                    continue;
                }
            }
            // A lone backslash before something we do not recognise: keep it.
            out += c;
            i++;
            continue;
        }

        if (c == '{' || c == '}') {
            // Grouping braces. AutoCAD wraps nearly every formatted run in
            // them, and they are not text.
            i++;
            continue;
        }

        out += c;
        i++;
    }

    return out;
}

std::string plainToMText(const std::string& plain) {
    std::string out;
    out.reserve(plain.size());

    for (const char c : plain) {
        switch (c) {
        case '\\':
            out += "\\\\";
            break;
        case '{':
            out += "\\{";
            break;
        case '}':
            out += "\\}";
            break;
        case '\n':
            out += "\\P";
            break;
        case '\r':
            break;  // a CR beside the LF we just wrote would be a second break
        default:
            out += c;
            break;
        }
    }

    return out;
}

std::string textToPlain(const std::string& dxfText) {
    std::string out;
    out.reserve(dxfText.size());

    for (std::size_t i = 0; i < dxfText.size();) {
        if (dxfText[i] == '%' && takePercentForm(dxfText, i, out)) {
            continue;
        }
        out += dxfText[i];
        i++;
    }

    return out;
}

}  // namespace persistence
}  // namespace lc

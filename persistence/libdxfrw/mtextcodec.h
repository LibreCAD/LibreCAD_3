#pragma once

#include <string>

namespace lc {
namespace persistence {

/**
 * The text of an MTEXT is not the text a user typed.
 *
 * DXF wraps it in an escape language: paragraphs break on `\P`, not on a
 * newline; a literal backslash or brace is doubled; `%%d` means a degree sign;
 * and a font, height or colour run is introduced by `\f`, `\H` or `\C` inside
 * braces. Handing that string to a renderer unchanged is why an imported MTEXT
 * shows `\P` between its lines instead of breaking there.
 *
 * These two functions are the whole boundary. Everything above them -- the
 * entity, the renderer, the dialog -- deals in plain text with real newlines.
 *
 * They are deliberately free functions over std::string, naming no libdxfrw
 * type, so the unit tests can include this header directly; persistence links
 * libdxfrw PRIVATE and lcunittest cannot reach its headers.
 */

/**
 * DXF MTEXT text -> plain text.
 *
 * Interprets what changes the reading of the text: `\P` becomes a newline,
 * `\~` a non-breaking space, `^I`/`^J` and the rest of the caret forms their
 * control characters, `%%d`/`%%c`/`%%p` the degree, diameter and plus-minus
 * signs, and `\\`, `\{`, `\}` their literal characters.
 *
 * Strips what only changes its appearance: a `\f`, `\H`, `\C`, `\W`, `\S` and
 * the other formatting introducers, along with the braces that group them.
 * Stripping is not the same as supporting them -- it is what makes a file
 * written by AutoCAD legible rather than a wall of `{\fArial|b0|i0;`.
 *
 * Nothing here can fail: malformed input keeps its characters rather than
 * throwing, because the alternative is a drawing that will not open.
 */
std::string mtextToPlain(const std::string& dxfText);

/**
 * Plain text -> DXF MTEXT text.
 *
 * The inverse of the interpreting half: newline becomes `\P`, and `\`, `{`
 * and `}` are escaped. It does not reintroduce formatting, because by this
 * point there is none left to reintroduce.
 *
 * Escaping happens before the newline substitution, so the `\` of a `\P` this
 * function just wrote is never escaped a second time.
 */
std::string plainToMText(const std::string& plain);

/**
 * The `%%d` family, for single-line TEXT.
 *
 * A TEXT record has no escape language, but it shares this one convention with
 * MTEXT, and LibreCAD has never decoded it either -- a dimension typed as
 * `45%%d` reads as `45%%d` on the canvas.
 */
std::string textToPlain(const std::string& dxfText);

}  // namespace persistence
}  // namespace lc

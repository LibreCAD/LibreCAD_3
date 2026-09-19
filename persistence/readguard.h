#pragma once

#include <exception>
#include <string>

namespace lc {
namespace persistence {

/**
 * Run one read callback and survive it throwing, reporting why.
 *
 * libdxfrw's read path is not a function-try-block the way its write path is,
 * so an exception thrown inside a callback leaves dxfRW::read() reporting
 * DRW::BAD_NONE: the caller is told the file read cleanly while the process is
 * already unwinding. LibreCAD's own kernel throws from inside these callbacks
 * -- geo::Arc for an unusable radius, geo::Area for a volume -- and one bad
 * record used to end the program.
 *
 * The `...` handler is not belt and braces. lc::geo::Area threw a bare
 * `const char*` for years, which a `catch (const std::exception&)` walks
 * straight past and which no amount of careful std::exception handling would
 * have caught.
 *
 * This guard must not be introduced before the crashes it would hide are
 * understood: it turns a reproducible abort into a recorded skip, which is an
 * improvement only once the aborts have been root-caused, and otherwise deletes
 * the signal that finds them.
 *
 * It lives in its own header, naming no libdxfrw type, so that the behaviour
 * can be tested from lcunittest -- which is gnu++14 and cannot include
 * libdxfrw's C++17 headers at all.
 *
 * @return true when the body completed; false when it threw, with `reason` set.
 */
template<typename Body>
bool runGuarded(Body body, std::string& reason) {
    try {
        body();
        return true;
    } catch (const std::exception& e) {
        reason = e.what();
    } catch (const char* e) {
        reason = e == nullptr ? "unrecognised exception" : e;
    } catch (...) {
        reason = "unrecognised exception";
    }

    return false;
}

}  // namespace persistence
}  // namespace lc

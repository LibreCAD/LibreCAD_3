#include <CoreGraphics/CGSession.h>

const char *spikeGuiSessionIssue() {
    CFDictionaryRef session = CGSessionCopyCurrentDictionary();
    if (!session) return "macOS widget checks require an accessible Quartz GUI session";
    const char *issue = nullptr;
    if (CFDictionaryGetValue(session, kCGSessionOnConsoleKey) != kCFBooleanTrue ||
        CFDictionaryGetValue(session, kCGSessionLoginDoneKey) != kCFBooleanTrue)
        issue = "macOS widget checks require a logged-in console GUI session";
    // This diagnostic key is not a public API promise. If absent, the real
    // exposure/focus assertions remain authoritative; absence never fakes focus.
    const auto locked = CFDictionaryGetValue(session, CFSTR("CGSSessionScreenIsLocked"));
    if (locked && CFGetTypeID(locked) == CFBooleanGetTypeID() &&
        CFBooleanGetValue(static_cast<CFBooleanRef>(locked)))
        issue = "macOS console is locked; unlock the desktop before widget checks";
    CFRelease(session);
    return issue;
}

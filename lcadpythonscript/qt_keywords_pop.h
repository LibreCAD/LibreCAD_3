// Restore the Qt keyword macros suppressed by qt_keywords_push.h.
// See that header for why this exists.
//
// Deliberately NOT `#pragma once`: this header must take effect at every use.

#pragma pop_macro("emit")
#pragma pop_macro("signals")
#pragma pop_macro("slots")

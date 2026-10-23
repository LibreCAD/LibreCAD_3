#include "renderer.h"
#include <type_traits>
static_assert(std::is_destructible<lc::viewer::render::Snapshot>::value, "Owned CPU snapshots");
// Compiled separately in C++14 without any Qt include or link usage requirements.

#pragma once

#include <QtCore/qglobal.h>

#ifdef LCUI_BUILDING
#define LCUI_API Q_DECL_EXPORT
#else
#define LCUI_API Q_DECL_IMPORT
#endif

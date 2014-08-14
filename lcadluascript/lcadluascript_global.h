#pragma once

#include <QtCore/qglobal.h>

#if defined(LCADLUASCRIPT_LIBRARY)
#  define LCADLUASCRIPTSHARED_EXPORT Q_DECL_EXPORT
#else
#  define LCADLUASCRIPTSHARED_EXPORT Q_DECL_IMPORT
# endif

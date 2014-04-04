#ifndef LCADLUASCRIPT_H
#define LCADLUASCRIPT_H

#include "lcadluascript_global.h"

#include <cad/document/abstractdocument.h>


class LCADLUASCRIPTSHARED_EXPORT LCadLuaScript
{

public:
    LCadLuaScript(lc::AbstractDocument* document);
    QString run(const QString &script);
private:
    lc::AbstractDocument* _document;
};

#endif // LCADLUASCRIPT_H

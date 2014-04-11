
#ifndef LUALIBRECADBRIDGE_H
#define LUALIBRECADBRIDGE_H

#include <cad/operations/create.h>
#include <cad/document/abstractdocument.h>


struct lua_State;
void lua_openlckernel(lua_State*L);


class LuaCreateEntities : public lc::operation::Create
{
public:
    LuaCreateEntities(lc::AbstractDocument* document, const char* layerName) : lc::operation::Create(document, layerName) {
    }

    void append(shared_ptr<lc::CADEntity> cadEntity) {
        lc::operation::Create::append(cadEntity);
    }
};

#endif // LUALIBRECADBRIDGE_H

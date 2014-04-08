
#ifndef LUALIBRECADBRIDGE_H
#define LUALIBRECADBRIDGE_H

#include <cad/operations/createentities.h>

#include <cad/document/abstractdocument.h>


struct lua_State;
void lua_openlckernel(lua_State*L);


class LuaCreateEntities : public lc::CreateEntities
{
public:
    LuaCreateEntities(lc::AbstractDocument* document, const char* layerName) : lc::CreateEntities(document, layerName) {
    }

    void append(shared_ptr<lc::CADEntity> cadEntity) {
        lc::CreateEntities::append(cadEntity);
    }
};

#endif // LUALIBRECADBRIDGE_H

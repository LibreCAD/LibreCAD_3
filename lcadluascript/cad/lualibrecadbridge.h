
#ifndef LUALIBRECADBRIDGE_H
#define LUALIBRECADBRIDGE_H

#include <cad/operations/builder.h>
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

class LuaBuilder : public lc::operation::Builder
{
public:
    LuaBuilder(lc::AbstractDocument* document) : lc::operation::Builder(document) {
    }

     void append(shared_ptr<lc::CADEntity> cadEntity) {
          lc::operation::Builder::append(cadEntity);
    }

     void move(const lc::geo::Coordinate& offset) {
         lc::operation::Builder::move(offset);
    }
     void copy(const lc::geo::Coordinate& offset) {
         lc::operation::Builder::copy(offset);
    }
    void repeat(const int numTimes) {
         lc::operation::Builder::repeat(numTimes);
    }

};

#endif // LUALIBRECADBRIDGE_H

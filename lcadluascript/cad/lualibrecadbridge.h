
#ifndef LUALIBRECADBRIDGE_H
#define LUALIBRECADBRIDGE_H

#include <cad/operations/builder.h>
#include <cad/document/abstractdocument.h>


struct lua_State;
void lua_openlckernel(lua_State*L);


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
     void rotate(const lc::geo::Coordinate& rotation_center, const double rotation_angle) {
          lc::operation::Builder::rotate(rotation_center, rotation_angle);
     }
     void begin() {
          lc::operation::Builder::begin();
     }
     void push() {
          lc::operation::Builder::push();
     }

};

#endif // LUALIBRECADBRIDGE_H

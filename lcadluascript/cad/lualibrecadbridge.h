
#ifndef LUALIBRECADBRIDGE_H
#define LUALIBRECADBRIDGE_H

#include <cad/operations/builder.h>
#include <cad/operations/create.h>
#include <cad/operations/rotate.h>
#include <cad/document/abstractdocument.h>


struct lua_State;
void lua_openlckernel(lua_State*L);


class LuaLcOperationCreate : public lc::operation::Create
{
public:
    LuaLcOperationCreate(lc::AbstractDocument* document, const QString& layerName) : lc::operation::Create(document, layerName) {
    }

    void append(shared_ptr<lc::CADEntity> cadEntity) {
        lc::operation::Create::append(cadEntity);
    }
};

class LuaLcOperationRotate : public lc::operation::Rotate
{
public:
    LuaLcOperationRotate(lc::AbstractDocument* document, const lc::geo::Coordinate& rotation_center, const double rotation_angle, const long no_copies) : lc::operation::Rotate(document, rotation_center, rotation_angle, no_copies) {
    }

    void append(shared_ptr<lc::CADEntity> cadEntity) {
        lc::operation::Rotate::append(cadEntity);
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
     void rotate(const lc::geo::Coordinate& rotation_center, const double rotation_angle) {
          lc::operation::Builder::rotate(rotation_center, rotation_angle);
     }
     void begin() {
          lc::operation::Builder::begin();
     }

};

#endif // LUALIBRECADBRIDGE_H

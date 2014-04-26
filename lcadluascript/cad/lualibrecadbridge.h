
#ifndef LUALIBRECADBRIDGE_H
#define LUALIBRECADBRIDGE_H

#include <cad/operations/builder.h>
#include <cad/document/document.h>
#include <cad/document/entitymanager.h>
#include <cad/dochelpers/layermanagerimpl.h>
#include <cad/meta/layer.h>


struct lua_State;
void lua_openlckernel(lua_State* L);

/**
 * @brief The LuaBuilderProxy class
 * this class is a proxy class untill we tell Lua the return Builder refrences
 */
class LuaBuilderProxy : public lc::operation::Builder {
    public:
        LuaBuilderProxy(lc::Document* document, shared_ptr<lc::EntityManager> entityManager) : lc::operation::Builder(document, entityManager) {
            qDebug() << "LuaBuilderProxy instantiated";
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

class LuaLayerManagerImplProxy : public lc::LayerManagerImpl {
    public:
        LuaLayerManagerImplProxy(lc::Document* document) : LayerManagerImpl(document) {
        }


};

#endif // LUALIBRECADBRIDGE_H

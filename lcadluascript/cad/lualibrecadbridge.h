#ifndef LUALIBRECADBRIDGE_H
#define LUALIBRECADBRIDGE_H

#include <memory>

#include <cad/operations/builder.h>
#include <cad/document/document.h>
#include <cad/meta/layer.h>
#include <cad/dochelpers/storagemanagerimpl.h>


struct lua_State;
void lua_openlckernel(lua_State* L);

#endif // LUALIBRECADBRIDGE_H

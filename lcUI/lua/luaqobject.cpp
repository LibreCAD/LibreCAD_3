#include "luaqobject.h"
#include <iostream>

LuaQObject::LuaQObject(QObject* object, lua_State* L):
	_object(object),
	_L(L)
{
}

int LuaQObject::connect(int signalId, std::string luaFunction) {
	_slotId = 1;
	
	if(QMetaObject::connect(_object, signalId, this, this->metaObject()->methodCount() + _slotId)) {
		_slotFunction = LuaIntf::LuaRef(_L, luaFunction.c_str()); 
		
		lua_pushboolean(_L, true);
		return true;
	}

	lua_pushboolean(_L, false);

	return false;
}

int LuaQObject::qt_metacall(QMetaObject::Call c, int id, void **a)
{
	id = QObject::qt_metacall(c, id, a);
	
	if(id == _slotId) {
		_slotFunction();
	}
	
	return -1;
}

std::string LuaQObject::objectName(QObject* object) {
	return object->objectName().toStdString();
}


std::string LuaQObject::objectName() {
	if(_object) {
		return objectName(_object);
	}

	return "null";
}

QObject* LuaQObject::findChild(QObject* object, std::string name) {
	for(auto *child : object->children()) {
		if(objectName(child) == name) {
			return child;
		}
	}

	std::cout << "Child " << name << " not found." << std::endl;

	return 0;
}

QObject* LuaQObject::findChild(std::string name) {
	if(_object) {
		return findChild(_object, name);
	}

	return 0;
}
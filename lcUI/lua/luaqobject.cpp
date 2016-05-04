#include "luaqobject.h"
#include <iostream>

LuaQObject::LuaQObject(QObject* object):
	_object(object)
{
	//Connect QObject destroyed() signal
	const int destroySignalId = _object->metaObject()->indexOfSignal("destroyed()");
	QMetaObject::connect(_object, destroySignalId, this, metaObject()->methodCount());
}

LuaQObject::~LuaQObject() {
	_object = 0;
	
}

bool LuaQObject::connect(int signalId, LuaIntf::LuaRef slot) {
	if(slot.isFunction()) {
		_slotId = 1;
		
		if(QMetaObject::connect(_object, signalId, this, this->metaObject()->methodCount() + _slotId)) {
			_slotFunction = slot; 
			
			return true;
		}
	}
	else {
		std::cerr << "Given slot is not a function" << std::endl;
	}

	return false;
}

int LuaQObject::qt_metacall(QMetaObject::Call c, int id, void **a)
{
	id = QObject::qt_metacall(c, id, a);
	if(id == _slotId && _slotFunction) {
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
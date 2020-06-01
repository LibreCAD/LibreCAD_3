#include "luaqobject.h"

LuaQObject::LuaQObject(QObject* object):
	_object(object),
	_slotId(-1),
	_signalId(-1),
	_valid(false)
{
	//Connect QObject destroyed() signal
	const int destroySignalId = _object->metaObject()->indexOfSignal("destroyed()");
	QMetaObject::connect(_object, destroySignalId, this, metaObject()->methodCount()); //NOLINT
}

LuaQObject::~LuaQObject() {
	_object = nullptr;
}

bool LuaQObject::connect(int signalId, const kaguya::LuaRef& slot) {
	_signalId = signalId;
	if(slot.type() == LUA_TFUNCTION) {
		_slotId = 1;
		
		if(QMetaObject::connect(_object, signalId, this, this->metaObject()->methodCount() + _slotId) != nullptr) {
			_slotFunction = slot;
			_valid = true;
		}
	}
	else {
		std::cerr << "Given slot is not a function" << std::endl;
	}

	return _valid;
}

bool LuaQObject::disconnect(int signalId, const kaguya::LuaRef& slot) {
    if (_signalId != signalId) {
        return false;
    }

    if (slot.type() == LUA_TFUNCTION) {
        _slotId = 1;

        if (_slotFunction == slot) {
            if (QMetaObject::disconnect(_object, signalId, this, this->metaObject()->methodCount() + _slotId)) {
                _valid = false;
                return true;
            }
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

	if(id == 0) {
		_valid = false;
	}
	else if(id == _slotId) {
		if(_slotFunction) {
			kaguya::State s(_slotFunction.state());
			auto parameters = _object->metaObject()->method(_signalId).parameterTypes();
			unsigned int i = 0;

			_slotFunction.push();
			for (const auto& parameter : parameters) {
				i++;
				pushArg(s, QMetaType::type(parameter.constData()), a[i]);
			}

			lua_call(s.state(), i, 0);
		}

		return -1;
	}
	
	return id;
}

std::string LuaQObject::objectName(QObject* object) {
	return object->objectName().toStdString();
}


std::string LuaQObject::objectName() {
	if(_object != nullptr) {
		return objectName(_object);
	}

	return "null";
}

QObject* LuaQObject::findChild(QObject* object, const std::string& name) {
	for(auto child : object->children()) {
		if(objectName(child) == name) {
			return child;
		}
	}

	std::cout << "Child " << name << " not found." << std::endl;

	return nullptr;
}

QObject* LuaQObject::findChild(const std::string& name) {
	if(_object != nullptr) {
		return findChild(_object, name);
	}

	return nullptr;
}

void LuaQObject::pushArg(kaguya::State& s, int const type, void const* arg) {
	if(type == qRegisterMetaType<lc::geo::Coordinate>()) {
		lua_pushlightuserdata(s.state(), (lc::geo::Coordinate*) arg);
		return;
	}

	switch (type) {
		case QMetaType::Void:
			lua_pushnil(s.state());
			break;
		case QMetaType::Bool:
			lua_pushboolean(s.state(), *(bool *) arg);
			break;
		case QMetaType::Int:
			lua_pushinteger(s.state(), *(int *) arg);
			break;
		case QMetaType::UInt:
			lua_pushinteger(s.state(), *(unsigned int *) arg);
			break;
		case QMetaType::Long:
			lua_pushinteger(s.state(), *(long *) arg);
			break;
		case QMetaType::LongLong:
			lua_pushinteger(s.state(), *(long long *) arg);
			break;
		case QMetaType::Short:
			lua_pushinteger(s.state(), *(short *) arg);
			break;
		case QMetaType::Char:
			lua_pushinteger(s.state(), *(char *) arg);
			break;
		case QMetaType::ULong:
			lua_pushnumber(s.state(), *(unsigned long *) arg);
			break;
		case QMetaType::ULongLong:
			lua_pushnumber(s.state(), *(unsigned long long *) arg);
			break;
		case QMetaType::UShort:
			lua_pushnumber(s.state(), *(unsigned short *) arg);
			break;
		case QMetaType::UChar:
			lua_pushnumber(s.state(), *(unsigned char *) arg);
			break;
		case QMetaType::Double:
			lua_pushnumber(s.state(), *(double *) arg);
			break;
		case QMetaType::Float:
			lua_pushnumber(s.state(), *(float *) arg);
			break;
		case QMetaType::QString:
			lua_pushlightuserdata(s.state(), const_cast<QString*>(static_cast<const QString*>(arg)));
			break;
		default:
			lua_pushnil(s.state());
			break;
	}
}

bool LuaQObject::valid() {
	return _valid;
}

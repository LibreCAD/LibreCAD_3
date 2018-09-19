#include "luaqobject.h"

LuaQObject::LuaQObject(QObject* object):
	_object(object),
	_slotId(-1),
	_signalId(-1),
	_valid(false)
{
	//Connect QObject destroyed() signal
	const int destroySignalId = _object->metaObject()->indexOfSignal("destroyed()");
	QMetaObject::connect(_object, destroySignalId, this, metaObject()->methodCount());
}

LuaQObject::~LuaQObject() {
	_object = nullptr;
}

bool LuaQObject::connect(int signalId, const LuaIntf::LuaRef& slot) {
	_signalId = signalId;
	if(slot.isFunction()) {
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

int LuaQObject::qt_metacall(QMetaObject::Call c, int id, void **a)
{
	id = QObject::qt_metacall(c, id, a);

	if(id == 0) {
		_valid = false;
	}
	else if(id == _slotId) {
		if(_slotFunction) {
			LuaIntf::LuaState s = _slotFunction.state();
			auto parameters = _object->metaObject()->method(_signalId).parameterTypes();
			unsigned int i = 0;

			_slotFunction.pushToStack();
			for (const auto& parameter : parameters) {
				i++;
				pushArg(s, QMetaType::type(parameter.constData()), a[i]);
			}

			s.call(i, 0);
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

void LuaQObject::pushArg(const LuaIntf::LuaState& s, int const type, void const* arg) {
	if(type == qRegisterMetaType<lc::geo::Coordinate>()) {
		s.push(*(lc::geo::Coordinate*) arg);
		return;
	}

	switch (type) {
		case QMetaType::Void:
			s.push(nullptr);
			break;
		case QMetaType::Bool:
			s.push(*(bool *) arg);
			break;
		case QMetaType::Int:
			s.push(*(int *) arg);
			break;
		case QMetaType::UInt:
			s.push(*(unsigned int *) arg);
			break;
		case QMetaType::Long:
			s.push(*(long *) arg);
			break;
		case QMetaType::LongLong:
			s.push(*(long long *) arg);
			break;
		case QMetaType::Short:
			s.push(*(short *) arg);
			break;
		case QMetaType::Char:
			s.push(*(char *) arg);
			break;
		case QMetaType::ULong:
			s.push(*(unsigned long *) arg);
			break;
		case QMetaType::ULongLong:
			s.push(*(unsigned long long *) arg);
			break;
		case QMetaType::UShort:
			s.push(*(unsigned short *) arg);
			break;
		case QMetaType::UChar:
			s.push(*(unsigned char *) arg);
			break;
		case QMetaType::Double:
			s.push(*(double *) arg);
			break;
		case QMetaType::Float:
			s.push(*(float *) arg);
			break;
		case QMetaType::QString:
			s.push(static_cast<const QString *>(arg));
			break;
		default:
			s.push(nullptr);
			break;
	}
}

bool LuaQObject::valid() {
	return _valid;
}
#include "viewport.h"

using namespace lc::meta;

Viewport::Viewport(std::string name) :
		DocumentMetaType(),
        _name(std::move(name)){
}

Viewport::Viewport(const builder::ViewportBuilder& builder) :
    DocumentMetaType(),
    _name(builder.name()){
}

const std::string Viewport::id() const {
    return VIEWPORT_ID_PREFIX "_" + _name;
}

const std::string Viewport::name() const {
    return _name;
}
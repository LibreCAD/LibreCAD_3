#include "viewport.h"
#include <cad/meta/viewport.h>

using namespace lc;
using namespace builder;

ViewportBuilder::ViewportBuilder() :
    _name()
    {

}

const std::string& ViewportBuilder::name() const {
    return _name;
}

ViewportBuilder* ViewportBuilder::setName(const std::string& name) {
    _name = name;
    
    return this;
}

meta::Viewport_CSPtr ViewportBuilder::build() {
    if(!checkValues()) {
        throw std::runtime_error("Missing values");
    }

    return meta::Viewport_CSPtr(new meta::Viewport(*this));
}

bool ViewportBuilder::checkValues() {
    return !_name.empty();
}

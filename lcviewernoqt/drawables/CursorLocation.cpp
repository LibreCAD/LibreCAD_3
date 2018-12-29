//
// Created by R. van Twisk on 4/20/15.
//

#include <cad/meta/color.h>
#include "CursorLocation.h"

using namespace lc;
using namespace lc::viewer;
using namespace lc::viewer::drawable;

CursorLocation::CursorLocation(int textSize, std::shared_ptr<manager::EventManager> const &manager, lc::Color &color)
        : _textSize(textSize), _color(color) {

    manager->locationEvents().connect<CursorLocation, &CursorLocation::on_locationEvent>(this);
}


void CursorLocation::on_locationEvent(const event::LocationEvent &l) const {
    std::cout << l.location();
}

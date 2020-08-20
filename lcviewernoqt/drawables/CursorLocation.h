//
// Created by R. van Twisk on 4/20/15.
//
#pragma once

#include <cad/meta/color.h>
#include "managers/eventmanager.h"
#include <memory>

namespace lc {
namespace viewer {
namespace drawable {
class CursorLocation {
public:
    CursorLocation(int textSize, std::shared_ptr<manager::EventManager> const&, lc::Color& color);

    void on_locationEvent(const event::LocationEvent&) const;

private:
    int _textSize;
    lc::Color _color;

};
}
}

}

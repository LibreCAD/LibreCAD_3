//
// Created by R. van Twisk on 4/20/15.
//
#pragma once

#include <cad/meta/color.h>
#include "../managers/EventManager.h"

class CursorLocation {
public:
    CursorLocation(int textSize, std::shared_ptr<EventManager> const &, lc::Color& color);

    void on_locationEvent(const LocationEvent&) const;

private:
    int _textSize;
    lc::Color _color;

};


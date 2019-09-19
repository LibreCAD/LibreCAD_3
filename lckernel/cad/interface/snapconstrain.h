//
// Created by R. van Twisk on 4/30/15.
//

#pragma once

#include <cad/geometry/geocoordinate.h>

namespace lc {
    /**
     * A SimpleSnapConstrain allow's setting up snapping where we are only interested in possible snap point's
     * based on a location.
     * Each entity that implements Snapable needs to take into the rules of SimpleSnapConstrain where possible and/or where it makes sense
     */
    class SimpleSnapConstrain {
    public:
        const static uint16_t NONE = 0x00;
        const static uint16_t ON_ENTITY = 0x01; // Find a point anywhere on it's entity.
        const static uint16_t ON_ENTITYPATH = 0x02; // Find a point anywhere on it's entity's path
        const static uint16_t ENTITY_CENTER = 0x04; // Get a snap point centered along the entity. For a arc with will be in the middle between arc start and arc end, for line it will be the center, for a circle this is not possible.
        // ENTITY_GRAVCENTER = 8; // Entities gravitationally center, for a circle this would be the circle's center
        const static uint16_t LOGICAL = 0x08; // enable to entities 'handles' There can be for example end point's of a line, or the center of a circle, or the 4 corners of a circle, grid point's etc..
        // basically anything that might makes sense to snap into.
        const static uint16_t DIVIDED = 0x10; // DIVIDED divides the entity into X equal portions given by divisions


        SimpleSnapConstrain &operator=(const SimpleSnapConstrain &other) {
            if (this != &other) {
                _angle = other._angle;
                _divisions = other._divisions;
                _constrain = other._constrain;
            }
            return *this;
        }


        SimpleSnapConstrain() :
                _constrain(LOGICAL),
                _divisions(0),
                _angle(0.) { }

        SimpleSnapConstrain(uint16_t constrain, int divisions, double angle) :
                _constrain(constrain),
                _divisions(divisions),
                _angle(angle) { }

        const uint16_t constrain() const {
            return _constrain;
        }

        int divisions() const {
            return _divisions;
        }

        double angle() const {
            return _angle;
        }

        SimpleSnapConstrain setDivisions(int divisions) const {
            return SimpleSnapConstrain(_constrain, divisions, _angle);
        }

        SimpleSnapConstrain setAngle(double angle) const {
            return SimpleSnapConstrain(_constrain, _divisions, angle);
        }

        SimpleSnapConstrain enableConstrain(uint16_t constrain) const {
            return SimpleSnapConstrain(_constrain | constrain, _divisions, _angle);
        }

        SimpleSnapConstrain disableConstrain(uint16_t constrain) const {
            return SimpleSnapConstrain(_constrain & ~constrain, _divisions, _angle);
        }

        bool hasConstrain(uint16_t constrain) const {
            return (_constrain & constrain) != 0;
        }


    private:
        uint16_t _constrain;
        int _divisions;
        double _angle;
    };
}

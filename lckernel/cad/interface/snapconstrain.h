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
        enum Constrain {
            NONE = 0,
            ON_ENTITY = 1, // Find a point anywhere on it's entity.
            ON_ENTITYPATH = 2, // Find a point anywhere on it's entity's path
            ENTITY_CENTER = 4, // Get a snap point centered along the entity. For a arc with will be in the middle between arc start and arc end, for line it will be the center, for a circle this is not possible.
            // ENTITY_GRAVCENTER = 8, // Entities gravitationally center, for a circle this would be the circle's center
            LOGICAL = 8, // enable to entities 'handles' There can be for example end point's of a line, or the center of a circle, or the 4 corners of a circle, grid point's etc..
            // basically anything that might makes sense to snap into.
            DIVIDED = 32, // DIVIDED divides the entity into X equal portions given by divisions
        };

        SimpleSnapConstrain &operator=(const SimpleSnapConstrain &other) {
            if (this != &other) {
                _angle = other._angle;
                _divisions = other._divisions;
                _constrain = other._constrain;
            }
            return *this;
        }


        SimpleSnapConstrain() :
                _constrain(Constrain::LOGICAL),
                _divisions(0),
                _angle(0.) { }

        SimpleSnapConstrain(const Constrain &constrain, int divisions, double angle) :
                _constrain(constrain),
                _divisions(divisions),
                _angle(angle) { }

        const Constrain constrain() const {
            return _constrain;
        }

        int divisions() const {
            return _divisions;
        }

        double angle() const {
            return _angle;
        }

    private:
         Constrain _constrain;
         int _divisions;
         double _angle;
    };
}

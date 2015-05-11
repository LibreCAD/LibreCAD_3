#pragma once

#include <vector>
#include "cad/geometry/geocoordinate.h"
#include "cad/vo/entitycoordinate.h"
#include "snapconstrain.h"

namespace lc {
    /**
      * Any entity that we can snap against needs to implement the snapable interface
      *
      */
    class Snapable {
        public:
            /*!
             * \brief Find a number of snap points the line has available
             * This function returns a ordered list, closest to \em coord and can return multiple snap points
             *
             * \param coord Coordinate to lookup the nearest coordinate from
             * \param minDistanceToSnap Minimum distance to the path to snap into. THis is a hint parameter and we should follow this, but it will be later filtered again
             * \param maxNumberOfSnapPoints Maximum number of snappoints that have to be looked up
             *
             * \sa lc::EntityCoordinate
             */
            virtual std::vector<lc::EntityCoordinate> snapPoints(const geo::Coordinate& coord, const SimpleSnapConstrain &simpleSnapConstrain, double minDistanceToSnap, int maxNumberOfSnapPoints) const = 0;

            /*!
             * \brief Find the nearest point on the path for this entity for the coordinate \em coord
             * The path of a entity that it can possibly take. For example for any Arc the path is the full circle if thge start/stop angle is 0 to 2*pi
             * \param coord Coordinate to lookup the nearest coordinate from
             * \sa lc::entity::CADEntity
             */
            virtual geo::Coordinate nearestPointOnPath(const geo::Coordinate& coord) const = 0;


            /**
             * Remove point's from vector if the distance to references is > distance
             */
            static void remove_ifDistanceGreaterThen(std::vector<EntityCoordinate> &points, const geo::Coordinate &reference, const double distance) {
                points.erase(
                        std::remove_if(points.begin(), points.end(),
                                       [reference, distance](const EntityCoordinate &entity) {
                                           return entity.coordinate().distanceTo(reference) > distance;

                                       }),
                        points.end());
            }

            /**
             * Use this function in your snapPoints function to cleanup any additional point's and confirm to
             * the requirements of maxNumberOfSnapPoints and minDistanceToSnap
             */
            static void snapPointsCleanup(std::vector<EntityCoordinate> &points, const geo::Coordinate &reference, const uint maxNumberOfSnapPoints, const double minDistanceToSnap) {
                // Remove point's further away then given by minDistanceToSnap
                Snapable::remove_ifDistanceGreaterThen(points, reference, minDistanceToSnap);
                // Sort in order of distance
                std::sort(points.begin(), points.end(), EntityCoordinateSorter(reference));
                // Only get maxNumberOfSnapPoints
                if (points.size() > maxNumberOfSnapPoints) {
                    points.erase(points.begin() + maxNumberOfSnapPoints, points.end());
                }
            }
    };
    typedef std::shared_ptr<Snapable> Snapable_SPtr;
    typedef std::shared_ptr<const Snapable> Snapable_CSPtr;
}

// SNAPABLE_H



#pragma once

#include "cad/const.h"
#include "cad/interface/entitydispatch.h"
#include "cad/base/cadentity.h"
#include "cad/meta/layer.h"

#include "cad/vo/entitycoordinate.h"
#include "cad/geometry/geobase.h"
#include "cad/interface/snapable.h"
#include "cad/interface/draggable.h"
#include <vector>

namespace lc {
    namespace entity {
        /**
        * Vertex for a Light Weight polyline
        */
        class LWVertex2D {
        public:
            LWVertex2D(geo::Coordinate const location, double const bulge = 0., double const startWidth = 0.,
                       double const endWidth = 0.)
                    : _location(location), _startWidth(startWidth), _endWidth(endWidth), _bulge(bulge) {
            }

            LWVertex2D(LWVertex2D const &other)
                    : _location(other._location),
                      _startWidth(other._startWidth),
                      _endWidth(other._endWidth),
                      _bulge(other._bulge) {
            }

            geo::Coordinate const location() const {
                return _location;
            }

            LWVertex2D const move(geo::Coordinate const &offset) {
                return LWVertex2D(_location + offset);
            }

            LWVertex2D const rotate(const geo::Coordinate &rotation_center, double rotation_angle) {
                return LWVertex2D(_location.rotate(rotation_center, rotation_angle));
            }

            /**
        * Scale up/Down a polyline. Be aware that non-uniform scaling won't turn a arc into a ellipse.
        */
            LWVertex2D const scale(const geo::Coordinate &scale_center, const geo::Coordinate &scale_factor) {
                if (scale_factor.x() == scale_factor.y()) {
                    return LWVertex2D(_location.scale(scale_center, scale_factor), _bulge * scale_factor.x(),
                                      _startWidth * scale_factor.x(), _endWidth * scale_factor.x());
                } else {
                    throw;
                }
            }

            double startWidth() const {
                return _startWidth;
            }

            double endWidth() const {
                return _endWidth;
            }

            double bulge() const {
                return _bulge;
            }

        private:
            friend std::ostream &operator<<(std::ostream &os, const LWVertex2D &a) {
                os << "LWVertex2D(location=" << a._location << " startWidth=" << a._startWidth << " startHeight=" <<
                a._endWidth << " bulge=" << a._bulge << ")";
                return os;
            }

        public:
            const geo::Coordinate _location;
            const double _startWidth;
            const double _endWidth;
            const double _bulge;
        };

        /**
         * Lightweight polyline
         */
        class LWPolyline : public std::enable_shared_from_this<LWPolyline>, public CADEntity, public Snapable, public Draggable {
        public:

            /**
        * LWPolyline Constructor
        * @param width of the vertex on each entity
        * @param elevation of the polyline
        * @param tickness of the polyline
        * @param extrusionDirection of the polyline
        * @param vertex entries of the polyline
        * @param layer
        * @param metaInfo
        */
            LWPolyline(const std::vector<LWVertex2D> &vertex, double width, double elevation, double tickness,
                       bool closed,
                       geo::Coordinate const &extrusionDirection,
                       const Layer_CSPtr layer, const MetaInfo_CSPtr metaInfo = nullptr);

            LWPolyline(const LWPolyline_CSPtr other, bool sameID = false);


            double width() const {
                return _width;
            }

            double elevation() const {
                return _elevation;
            }

            double tickness() const {
                return _tickness;
            }

            geo::Coordinate const &extrusionDirection() const {
                return _extrusionDirection;
            }

            std::vector<LWVertex2D> const &vertex() const {
                return _vertex;
            }

            bool closed() const {
                return _closed;
            }

        public:
            virtual std::vector<EntityCoordinate> snapPoints(const geo::Coordinate &coord,
                                                             const SimpleSnapConstrain &constrain,
                                                             double minDistanceToSnap,
                                                             int maxNumberOfSnapPoints) const override;

            virtual geo::Coordinate nearestPointOnPath(const geo::Coordinate &coord) const override;
            std::tuple<geo::Coordinate, std::shared_ptr<const geo::Vector>, std::shared_ptr<const geo::Arc>> nearestPointOnPath2(const geo::Coordinate &coord) const;

        private:
            const std::vector<LWVertex2D> _vertex;
            const double _width;
            const double _elevation;
            const double _tickness;
            const bool _closed; // If we had more 'flag' options we shoudl consuder using a enum instead of seperate variables to make constructors easer
            const geo::Coordinate _extrusionDirection;


            // LW LWPolyline this seems to be the facto standard
            // int vertexnum;            /*!< number of vertex, code 90 */
            // int flags;                /*!< LWPolyline flag, code 70, default 0 */
            // double width;             /*!< constant width, code 43 */
            // double elevation;         /*!< elevation, code 38 */
            // double thickness;         /*!< thickness, code 39 */
            // DRW_Coord extPoint;       /*!<  Dir extrusion normal vector, code 210, 220 & 230 */
            // std::vector<DRW_LWVertex2D *> vertlist;  /*!< vertex list */

            //DRW_LWVertex2D
            //double x;                 /*!< x geo::Coordinate, code 10 */
            //double y;                 /*!< y geo::Coordinate, code 20 */
            //double stawidth;          /*!< Start width, code 40 */
            //double endwidth;          /*!< End width, code 41 */
            //double bulge;             /*!< bulge, code 42 */




            // LWPolyline
            //int flags;               /*!< LWPolyline flag, code 70, default 0 */
            //double defstawidth;      /*!< Start width, code 40, default 0 */
            //double defendwidth;      /*!< End width, code 41, default 0 */
            //int vertexcount;         /*!< polygon mesh M vertex or  polyface vertex num, code 71, default 0 */
            //int facecount;           /*!< polygon mesh N vertex or  polyface face num, code 72, default 0 */
            //int smoothM;             /*!< smooth surface M density, code 73, default 0 */
            //int smoothN;             /*!< smooth surface M density, code 74, default 0 */
            //int curvetype;           /*!< curves & smooth surface type, code 75, default 0 */
            //std::vector<DRW_Vertex *> vertlist;  /*!< vertex list */


            //DRW_Vertex
            //double stawidth;          /*!< Start width, code 40 */
            //double endwidth;          /*!< End width, code 41 */
            //double bulge;             /*!< bulge, code 42 */
            //int flags;                 /*!< vertex flag, code 70, default 0 */
            //double tgdir;           /*!< curve fit tangent direction, code 50 */
            //int vindex1;             /*!< polyface mesh vertex index, code 71, default 0 */
            //int vindex2;             /*!< polyface mesh vertex index, code 72, default 0 */
            //int vindex3;             /*!< polyface mesh vertex index, code 73, default 0 */
            //int vindex4;             /*!< polyface mesh vertex index, code 74, default 0 */
            //int identifier;           /*!< vertex identifier, code 91, default 0 */


        public:
            /**
        * @brief move, moves by an offset
        * @param geo::Coordinate offset
        * @return CADEntity_CSPtr moved entity
        */
            virtual CADEntity_CSPtr move(const geo::Coordinate &offset) const override;

            /**
        * @brief copy, copies by an offset
        * @param geo::Coordinate offset
        * @return CADEntity_CSPtr copied entity
        */
            virtual CADEntity_CSPtr copy(const geo::Coordinate &offset) const override;

            /**
        * @brief rotate, rotate operation
        * @param geo::Coordinate rotation_center
        * @param double rotation_angle
        * @return CADEntity_CSPtr rotated entity
        */
            virtual CADEntity_CSPtr rotate(const geo::Coordinate &rotation_center, const double rotation_angle) const override;

            /**
        * @brief scale, scales the entity
        * @param geo::Coordinate scale_center
        * @param double scale_factor
        * @return
        */
            virtual CADEntity_CSPtr scale(const geo::Coordinate &scale_center,
                                          const geo::Coordinate &scale_factor) const override;

            /**
        * @brief boundingBox of the entity
        * @return geo::Area area
        */
            virtual const geo::Area boundingBox() const override;

            virtual CADEntity_CSPtr modify(Layer_CSPtr layer, const MetaInfo_CSPtr metaInfo) const override;

            /**
        * Return a vector of geometry entities for this polyline
        * The vector will contain geo::vector and geo::Arc items
        * In the current implementation no caching of the bounding box is done
        * If we see that bounding box calcualtion takes up a lot of time we
        * can consider calculating the bounding box in the constructor
        * and just return the cached result. We also use this for snapPoint's
       * and if performance is a issue, we can cache it.
        */
            std::vector<std::shared_ptr<const geo::Base>> const asGeometrics() const;


        public:
            virtual void accept(GeoEntityVisitor &v) const override { v.visit(*this); }

            virtual void dispatch(EntityDispatch &ed) const override {
                ed.visit(shared_from_this());
            }

        public:
            virtual std::map<unsigned int, lc::geo::Coordinate> dragPoints() const override;
            virtual CADEntity_CSPtr setDragPoints(std::map<unsigned int, lc::geo::Coordinate> dragPoints) const override;
        };

        using LWPolyline_SPtr = std::shared_ptr<LWPolyline>;
        using LWPolyline_CSPtr = std::shared_ptr<const LWPolyline>;
    }
}

// geo::Coordinate_H

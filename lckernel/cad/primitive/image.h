#pragma once

#include <list>

#include "cad/interface/entitydispatch.h"

#include "cad/geometry/geocoordinate.h"
#include "cad/geometry/geovector.h"
#include "cad/base/cadentity.h"
#include "cad/interface/snapable.h"
#include "cad/vo/entitycoordinate.h"
#include "cad/meta/layer.h"

namespace lc {
    namespace entity {
        /*!
         * \brief A Image that can be put in a drawing
         *
         * \sa CreateEntities::append
         * \author R. van Twisk
         * \date 2016-04-16
         */
        class Image : public std::enable_shared_from_this<Image>, public CADEntity, public Snapable, virtual public Visitable  {
        public:

            Image(const std::string &name, const geo::Coordinate &base, const geo::Coordinate &uv, const geo::Coordinate &vv, double width, double height,
                        double brightness, double contrast, double fade, const Layer_CSPtr layer);

            Image(const std::string &name, const geo::Coordinate &base, const geo::Coordinate &uv, const geo::Coordinate &vv, double width, double height,
                        const Layer_CSPtr layer);

            Image(const std::string &name, const geo::Coordinate &base, const geo::Coordinate &uv, const geo::Coordinate &vv, double width, double height,
                        double brightness, double contrast, double fade, const Layer_CSPtr layer, const MetaInfo_CSPtr metaInfo);

            Image(const std::string &name, const geo::Coordinate &base, const geo::Coordinate &uv, const geo::Coordinate &vv, double width, double height,
                        const Layer_CSPtr layer, const MetaInfo_CSPtr metaInfo);

            /*!
             * \brief Construct a new line
             *
             * \param vector Coordinate the line should start and end from
             * \param metaTypes A list of metatypes associated with this line
             * \sa Color
             * \sa LineWidth
             * \sa MetaType
             */
            Image(const Image_CSPtr other, bool sameID = false);

        public:
            virtual std::vector<EntityCoordinate> snapPoints(const geo::Coordinate &coord, const SimpleSnapConstrain & constrain,  double minDistanceToSnap,
                                                             int maxNumberOfSnapPoints) const override;

            virtual geo::Coordinate nearestPointOnPath(const geo::Coordinate &coord) const override;

        public:
            /**
             * @brief move, moves by an offset
             * @param geo::Coordinate offset
             * @return CADEntity_CSPtr moved entity
             */
            virtual CADEntity_CSPtr move(const geo::Coordinate &offset) const override;

            /**
             * @brief copy, copies Image by an offset
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

            virtual CADEntity_CSPtr mirror(const geo::Coordinate& axis1,
                    const geo::Coordinate& axis2) const override {
                return NULL; // TODO: no return statement, not implemented in lwpolyline.cpp
            }

            /**
             * @brief boundingBox of the entity
             * @return geo::Area area
             */
            virtual const geo::Area boundingBox() const override;

            virtual CADEntity_CSPtr modify(Layer_CSPtr layer, const MetaInfo_CSPtr metaInfo) const override;

            const std::string &name() const {
                return _name;
            }

            const geo::Coordinate &base() const {
                return _base;
            }

            const geo::Coordinate &uv() const {
                return _uv;
            }

            const geo::Coordinate &vv() const {
                return _vv;
            }


            double width() const {
                return _width;
            }

            double height() const {
                return _height;
            }

            double brightness() const {
                return _brightness;
            }

            double contrast() const {
                return _contrast;
            }

            double fade() const {
                return _fade;
            }

        public:
            virtual void accept(GeoEntityVisitor &v) const override { v.visit(*this); }

            virtual void dispatch(EntityDispatch &ed) const override {
                ed.visit(shared_from_this());
            }

        private:
            std::string _name;
            geo::Coordinate _base;
            geo::Coordinate _uv;
            geo::Coordinate _vv;
            double _width; // I really want tthese tome come from the image and not given by the DXF, for now I will
            double _height; // leave them in place for development and testing without loading the image here.

            double _brightness;            /*!< Brightness value, code 281, (0-100) default 50 */
            double _contrast;              /*!< Brightness value, code 282, (0-100) default 50 */
            double _fade;                  /*!< Brightness value, code 283, (0-100) default 0 */

        };
        using Image_SPtr = std::shared_ptr<Image>;
        using Image_CSPtr = std::shared_ptr<const Image>;

    }

}


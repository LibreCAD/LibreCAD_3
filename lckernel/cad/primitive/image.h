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
        class Image : public std::enable_shared_from_this<Image>,
                      public CADEntity,
                      public Snapable,
                      virtual public Visitable  {
            public:
                Image(std::string name,
                      geo::Coordinate base,
                      geo::Coordinate uv,
                      geo::Coordinate vv,
                      double width,
                      double height,
                      double brightness,
                      double contrast,
                      double fade,
                      Layer_CSPtr layer,
                      meta::MetaInfo_CSPtr metaInfo = nullptr,
                      meta::Block_CSPtr block = nullptr
                );

                /**
                 * @brief Copy an image
                 * @param other Image
                 * @param sameID True to keep the same ID
                 */
                Image(const Image_CSPtr& other, bool sameID = false);

                std::vector<EntityCoordinate> snapPoints(const geo::Coordinate &coord,
                                                         const SimpleSnapConstrain& constrain,
                                                         double minDistanceToSnap,
                                                         int maxNumberOfSnapPoints) const override;

                geo::Coordinate nearestPointOnPath(const geo::Coordinate &coord) const override;

                /**
                 * @brief move, moves by an offset
                 * @param geo::Coordinate offset
                 * @return CADEntity_CSPtr moved entity
                 */
                CADEntity_CSPtr move(const geo::Coordinate &offset) const override;

                /**
                 * @brief copy, copies Image by an offset
                 * @param geo::Coordinate offset
                 * @return CADEntity_CSPtr copied entity
                 */
                CADEntity_CSPtr copy(const geo::Coordinate &offset) const override;

                /**
                 * @brief rotate, rotate operation
                 * @param geo::Coordinate rotation_center
                 * @param double rotation_angle
                 * @return CADEntity_CSPtr rotated entity
                 */
                CADEntity_CSPtr rotate(const geo::Coordinate &rotation_center, double rotation_angle) const override;

                /**
                 * @brief scale, scales the entity
                 * @param geo::Coordinate scale_center
                 * @param double scale_factor
                 * @return
                 */
                CADEntity_CSPtr scale(const geo::Coordinate &scale_center,
                                              const geo::Coordinate &scale_factor) const override;

                CADEntity_CSPtr mirror(const geo::Coordinate& axis1, const geo::Coordinate& axis2) const override {
                    return NULL; // TODO: no return statement, implement in image.cpp
                }

                /**
                 * @brief boundingBox of the entity
                 * @return geo::Area area
                 */
                const geo::Area boundingBox() const override;

                CADEntity_CSPtr modify(Layer_CSPtr layer,
                                       const meta::MetaInfo_CSPtr metaInfo,
                                       meta::Block_CSPtr block) const override;

                const std::string& name() const {
                    return _name;
                }

                const geo::Coordinate& base() const {
                    return _base;
                }

                const geo::Coordinate& uv() const {
                    return _uv;
                }

                const geo::Coordinate& vv() const {
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

                void accept(GeoEntityVisitor &v) const override {
                    v.visit(*this);
                }

                void dispatch(EntityDispatch &ed) const override {
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

        DECLARE_SHORT_SHARED_PTR(Image)
    }

}


#pragma once

#include <cad/meta/block.h>
#include <cad/builders/cadentity.h>
#include "cad/const.h"
#include "cad/base/id.h"
#include "cad/base/metainfo.h"
#include "cad/base/visitor.h"
#include "cad/interface/metatype.h"
#include "cad/meta/layer.h"
#include "cad/geometry/geoarea.h"

namespace lc {
    namespace entity {
        /**
         *Class that all CAD entities must inherit
         *
         */
        class CADEntity : public ID, virtual public Visitable {
        template <typename T>
        friend class lc::builder::CADEntityBuilder;

        public:
            CADEntity() {
            }

            /*!
             * \brief Default CADEntity Constructor.
             * \sa lc::ID
             * \sa lc::MetaInfo
             */
            CADEntity(Layer_CSPtr _layer);

            /*!
             * \brief CADEntity Constructor
             *
             * \param metaTypes A list of metatypes associated with this line
             * \sa lc::Color
             * \sa lc::LineWidth
             * \sa lc::MetaType
             */
            CADEntity(Layer_CSPtr layer, const MetaInfo_CSPtr metaInfo);

            CADEntity(CADEntity_CSPtr cadEntity, bool sameID);

            CADEntity(CADEntity_CSPtr cadEntity);

            virtual ~CADEntity() = default;

            /*!
             * \brief Function for Move
             *
             * \param offset the offset by which entity is to be moved
             * \return CADEntity std::shared_ptr
             */
            virtual CADEntity_CSPtr move(const geo::Coordinate &offset) const = 0;

            /*!
             * \brief Function for Copy
             *
             * \param offset the offset by which entity is to be copied
             * \return CADEntity std::shared_ptr
             */
            virtual CADEntity_CSPtr copy(const geo::Coordinate &offset) const = 0;

            /*!
             * \brief Function implementation for rotate.
             *
             * \param angle angle by which the entity is to be rotated.
             * \return CADEntity std::shared_ptr
             */
            virtual CADEntity_CSPtr rotate(const geo::Coordinate &rotation_center,
                                           const double rotation_angle) const = 0;

            /*!
             * \brief Function implementation for Scale.
             *
             * \param scale_center
             * \param scale_factor
             * \return CADEntity std::shared_ptr
             */
            virtual CADEntity_CSPtr scale(const geo::Coordinate &scale_center,
                                          const geo::Coordinate &scale_factor) const = 0;

            virtual CADEntity_CSPtr mirror(const geo::Coordinate& axis1,
                                           const geo::Coordinate& axis2) const = 0;

            /**
             * @brief boundingBox
             * Return the bounding box of this entity.
             * This should be as tight around the entity as possible but care should be taken that this routine
             * is also as fast as possible. in essense, favor speed over tightness.
             * This routine will in generally be used during testing if entities cross for example to find snap points
             * and to build relationships of tree for example using quad trees
             * @return geo::Area
             */
            virtual const geo::Area boundingBox() const = 0;

            /**
            * @brief modify
            * Return a new entity with the same ID bit with possible modified metainfo and/pr layer information
            * #return new entity with same ID
            */
            virtual CADEntity_CSPtr modify(Layer_CSPtr layer, const MetaInfo_CSPtr metaInfo) const = 0;

            /*!
             * \brief layer
             * return the layer this entity is placed on
             * \return Layer_CSPtr
             */
            Layer_CSPtr layer() const;

            /**
            * Retreive meta information back from this entity
            * returns nullptr when the specific meta info wasn't found
            * example: auto metaData = myEntity.metaInfo<lc::MetaColorByValue>(lc::MetaInfo::_COLOR);
            */
            template<typename T>
            const std::shared_ptr<const T> metaInfo(std::string metaName) const {
                if (_metaInfo && (_metaInfo->find(metaName) != _metaInfo->end())) {
                    auto a=_metaInfo->at(metaName);
                    auto b=std::dynamic_pointer_cast<const T>(a);
                    return b;
                }

                return nullptr;
            }

            MetaInfo_CSPtr metaInfo() const {
                return _metaInfo;
            }

            virtual void accept(GeoEntityVisitor &v) const override { v.visit(*this); }

            virtual void dispatch(EntityDispatch &) const = 0;

            /**
             * @brief Return the current entity block
             * @return Entity block or nullptr if not defined
             */
            Block_CSPtr block() const;

        protected:
            template<typename ReturnType>
            CADEntity(const lc::builder::CADEntityBuilder<ReturnType>& builder) :
                        ID(),
                        _layer(builder.layer()),
                        _metaInfo(builder.metaInfo()),
                        _block(builder.block()) {
            }

        private:
            Layer_CSPtr _layer;
            MetaInfo_CSPtr _metaInfo;
            Block_CSPtr _block;
        };

        DECLARE_SHORT_SHARED_PTR(CADEntity)
    }
}

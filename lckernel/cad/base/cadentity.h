#pragma once

#include <cad/meta/block.h>
#include "cad/const.h"
#include "cad/base/id.h"
#include "cad/base/metainfo.h"
#include "cad/base/visitor.h"
#include "cad/interface/metatype.h"
#include "cad/meta/layer.h"
#include "cad/geometry/geoarea.h"
#include "cad/meta/viewport.h"

namespace lc {
    namespace builder {
        class CADEntityBuilder;
    }
    namespace entity {
        /**
         *Class that all CAD entities must inherit
         *
         */
        class CADEntity : public ID, virtual public Visitable {
        friend class lc::builder::CADEntityBuilder;

        public:
            CADEntity() = default;

            /*!
             * \brief CADEntity Constructor
             *
             * \param metaTypes A list of metatypes associated with this line
             * \sa lc::Color
             * \sa lc::LineWidth
             * \sa lc::MetaType
             */
            CADEntity(meta::Layer_CSPtr layer, meta::MetaInfo_CSPtr metaInfo = nullptr, meta::Block_CSPtr block = nullptr, meta::Viewport_CSPtr viewport = nullptr);

            CADEntity(const CADEntity_CSPtr& cadEntity, bool sameID);

            CADEntity(const CADEntity_CSPtr& cadEntity);

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
                                           double rotation_angle) const = 0;

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
             * is also as fast as possible. in essence, favor speed over tightness.
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
            virtual CADEntity_CSPtr modify(meta::Layer_CSPtr layer, meta::MetaInfo_CSPtr metaInfo,
                                           meta::Block_CSPtr block) const = 0;

            /*!
             * \brief layer
             * return the layer this entity is placed on
             * \return Layer_CSPtr
             */
            meta::Layer_CSPtr layer() const;

            /**
            * Retrieve meta information back from this entity
            * returns nullptr when the specific meta info wasn't found
            * example: auto metaData = myEntity.metaInfo<lc::MetaColorByValue>(lc::MetaInfo::_COLOR);
            */
            template<typename T>
            const std::shared_ptr<const T> metaInfo(const std::string& metaName) const {
                if (_metaInfo && (_metaInfo->find(metaName) != _metaInfo->end())) {
                    auto a=_metaInfo->at(metaName);
                    auto b=std::dynamic_pointer_cast<const T>(a);
                    return b;
                }

                return nullptr;
            }

                meta::MetaInfo_CSPtr metaInfo() const {
                return _metaInfo;
            }

            void accept(GeoEntityVisitor &v) const override {
                v.visit(*this);
            }

            virtual void dispatch(EntityDispatch &) const = 0;

            /**
             * @brief Return the current entity block
             * @return Entity block or nullptr if not defined
             */
            meta::Block_CSPtr block() const;

            meta::Viewport_CSPtr viewport() const{return _viewport;};

        protected:
            CADEntity(const lc::builder::CADEntityBuilder& builder);

        private:
            meta::Layer_CSPtr _layer;
            meta::MetaInfo_CSPtr _metaInfo;
            meta::Block_CSPtr _block;
            meta::Viewport_CSPtr _viewport;
        };

        DECLARE_SHORT_SHARED_PTR(CADEntity)
    }
}

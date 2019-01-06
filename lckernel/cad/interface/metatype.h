#pragma once

#include <cad/const.h>
#include <string>
#include <memory>
#include <iostream>

/**
  * If a object is a meta object, eg something that can be attached to an entity then this object must extend MetaType
  * and provide a unique meta name.
  * meta types can also be put into a variant so it can be stored in a map
  */
namespace lc {
    namespace meta {
        /**
         * A base meta type
         */
        class MetaType {
            public:
                MetaType() {
                }

                virtual ~MetaType() = default;

                /**
                 * ID or primary key of this MetaType. This can be used in a storage manager to identify each individual
                 * meta type.
                 */
                virtual const std::string id() const = 0;

            private:
                friend std::ostream& operator<<(std::ostream& os, const MetaType& metaType) {
                    os << "MetaType(" << metaType.id() << ")" << std::endl;
                    return os;
                }

        };

        /**
         * Meta type that can be attached to the document
         */
        class DocumentMetaType : public MetaType {
            public:
                /**
                 * Name of this meta type, for layer this can be '0' 'My Layer' etc,
                 * for LineStyles this can be 'DASHDOT' .
                */
                virtual const std::string name() const = 0;

            private:
                friend std::ostream& operator<<(std::ostream& os, const DocumentMetaType& documentMetaType) {
					os << "DocumentMetaType(" << ((const MetaType&) documentMetaType) << "; "<< documentMetaType.name() << ")" << std::endl;
                    return os;
                }
        };

        /**
         * A meta type that can be attached to an entity
         */
        class EntityMetaType : public MetaType {
            public:
                /**
                 * metaTypeName, for example '_LAYER' this method isn't meant to be used
                 * for RTTI purpose, do use dynamic_casts or static_casts for that.
                */
                virtual const std::string metaTypeID() const = 0;

            private:
                friend std::ostream& operator<<(std::ostream& os, const EntityMetaType& entityMetaType) {
					os << "EntityMetaType(" << ((const MetaType&)entityMetaType) << "; " << entityMetaType.metaTypeID() << ")" << std::endl;;
                    return os;
                }
        };

        DECLARE_SHORT_SHARED_PTR(MetaType)
        DECLARE_SHORT_SHARED_PTR(DocumentMetaType)
        DECLARE_SHORT_SHARED_PTR(EntityMetaType)
    }
}

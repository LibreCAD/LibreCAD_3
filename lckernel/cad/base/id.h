#ifndef ID_H
#define ID_H

#include "cad/const.h"


#define ID_DATATYPE unsigned long


/**
  * Class that will provide a unique ID for the lifetime of this object
  * There can only be one ID
  *
  */
namespace lc {
    class ID {
        public:

            /*!
             * \brief Default constructor, provides a new unique ID to each entity
             * \sa lc::CADEntity
             */
            ID();

            /*!
             * \brief returns the ID of the entity
             * \return ID
             */
            ID_DATATYPE id() const ;

            bool operator == (const ID& id) const {
                return (id._id == _id);
            }

            /*!
             * \brief Sets the ID of an entity
             * \param _ID
             */
            void setID(ID_DATATYPE _ID);

        private:
            unsigned long _id;
            ID_DATATYPE _ID;
    };
}
#endif // ID_H

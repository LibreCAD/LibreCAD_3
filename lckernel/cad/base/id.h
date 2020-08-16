/**
* @file
* @section DESCRIPTION
*
* This file implements ID
*/

#pragma once

#include <atomic>

#define ID_DATATYPE unsigned long

/**
  * Class that will provide a unique ID for the lifetime of this object
  * There can only be one ID
  *
  */
namespace lc {
namespace entity {
class ID {
public:

    /*!
     * \brief Default constructor, provides a new unique ID to each entity
     * \sa lc::entity::CADEntity
     */
    ID();

    ID(ID_DATATYPE);

    virtual ~ID() = default;

    /*!
     * \brief returns the ID of the entity
     * \return ID
     */
    ID_DATATYPE id() const;

    bool operator==(const ID& id) const {
        return (id._id == _id);
    }

    void setID(ID_DATATYPE id);

    static std::atomic<ID_DATATYPE> __idCounter;
private:
    ID_DATATYPE _id;
};
}
}

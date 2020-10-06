/**
* @file
* @section DESCRIPTION
*
* This file implements CADObject
*/

#pragma once

#include <cad/meta/block.h>
#include "cad/const.h"
#include "cad/base/id.h"
#include "cad/base/metainfo.h"
#include "cad/interface/metatype.h"
#include "cad/meta/layer.h"

namespace lc {
namespace object {
/**
 *Class that all CAD objects must inherit
 *
 */
class CADObject {
    DECLARE_SHORT_SHARED_PTR(CADObject)
public:
    CADObject() = default;

    /*!
     * \brief CADObject Constructor
     *
     */
    CADObject(const CADObject_CSPtr& cadObject, bool sameID);

    CADObject(const CADObject_CSPtr& cadObject);

    virtual ~CADObject() = default;

protected:

private:

};

DECLARE_SHORT_SHARED_PTR(CADObject)
}
}

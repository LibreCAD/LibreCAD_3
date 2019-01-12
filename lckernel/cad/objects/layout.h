#pragma once

#include "cad/base/cadobject.h"

namespace lc {
    namespace object {

        class Layout : public std::enable_shared_from_this<Layout>,
                       public CADObject
                       {
        DECLARE_SHORT_SHARED_PTR(Layout)
        public:
            Layout();

            Layout(const Layout_CSPtr& other, bool sameID = false);

        private:

        };
    }
}

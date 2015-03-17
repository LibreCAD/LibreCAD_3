#pragma once

namespace lc {
    namespace geo {
        /**
        * For now it will function as a base class but if required we can add pure virtuals in here
        * if we see they are required. For example store a vector of lines/Arc's and we need to get the bounding box
        * of each. This will prevent doing a dynamic_cast... we will see...
        */
        class Base  {
            public:
                virtual ~Base() = default;
        };
    }
}

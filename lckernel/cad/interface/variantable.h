#pragma once

#include "cad/const.h"

/**
  * If there are any types that needs to be put into a QVariant, then that class needs to extend Variantable whish enforces
  * the use of a valid function to validate the object before it can be put into the kernel.
  * The rationale is that only valid object are allowed to be added to the kernel
  *
  * The reason behind is that the QVariant system neds to have a default constructor available and this must be public.
  * Using a friend function it didn't work. Feel free to modify and remove the valid and default constructors if there is a better solution.
  * DEPRECATED
  */
namespace lc {
    class Variantable {
        public:
            virtual bool variantValid() const = 0;
            virtual ~Variantable() {}
    };
}

// VARIANTABLE_H

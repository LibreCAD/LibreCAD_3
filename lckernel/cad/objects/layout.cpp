#include "layout.h"
using namespace lc;
using namespace object;

Layout::Layout() :
        CADObject()
        {
}

Layout::Layout(const Layout_CSPtr& other, bool sameID) : CADObject(other, sameID)
{
}
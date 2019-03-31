#ifndef DEBUG_H
#define DEBUG_H
#include <iostream>

namespace lc
{
  namespace viewer
  {
    namespace opengl
   {

    inline void DebugMessage(std::string str)
    {
	   std::cout<<str<<std::endl;
    }

   }
  }
}

#endif // DEBUG_H
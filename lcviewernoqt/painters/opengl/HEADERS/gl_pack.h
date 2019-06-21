#ifndef GL_PACK_H
#define GL_PACK_H

#include <vector>

#include "gl_entity.h"

namespace lc
{
	namespace viewer
	{
		namespace opengl
		{


class GL_Pack
{
  private:   
    std::vector< GL_Entity* > _gl_entities;   // vector of gl_entity

  public:

	GL_Pack();

	~GL_Pack();

	//-------------------------------------
    
    int Pack_Size();

    GL_Entity* Get_GL_Entity_At(int i);

    void Push_Entity_In_Pack( GL_Entity* glentity);

};


}

}

}
#endif // GL_PACK_H
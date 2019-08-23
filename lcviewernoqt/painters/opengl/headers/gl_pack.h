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
	  
	int packSize();
    GL_Entity* getEntityAt(int i);
    void pushEntityInPack( GL_Entity* glentity);
    void freePackGPU();

};
		}
	}

}
#endif // GL_PACK_H
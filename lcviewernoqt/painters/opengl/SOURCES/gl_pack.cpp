#include "gl_pack.h"

using namespace lc::viewer::opengl;

GL_Pack::GL_Pack()
{
	
}

GL_Pack::~GL_Pack()
{
	
}

int GL_Pack::Pack_Size()
{
	return _gl_entities.size();
}

GL_Entity* GL_Pack::Get_GL_Entity_At(int i)
{
	return _gl_entities[i];
}

void GL_Pack::Push_Entity_In_Pack( GL_Entity* glentity )
{
	_gl_entities.push_back( glentity );
}

void GL_Pack::Free_GPU_Pack()
{
	std::vector< GL_Entity* >::iterator it;

	for(it=_gl_entities.begin();it!=_gl_entities.end();it++)
	{
		(*it)->FreeGPU();
		delete (*it);
	}

}
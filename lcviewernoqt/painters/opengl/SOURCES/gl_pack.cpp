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
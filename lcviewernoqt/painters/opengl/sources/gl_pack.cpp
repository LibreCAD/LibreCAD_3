#include "gl_pack.h"

using namespace lc::viewer::opengl;

GL_Pack::GL_Pack()
{
}

GL_Pack::~GL_Pack()
{
}

int GL_Pack::packSize()
{
  return _gl_entities.size();
}

GL_Entity* GL_Pack::getEntityAt(int i)
{
  return _gl_entities[i];
}

void GL_Pack::pushEntityInPack( GL_Entity* glentity )
{
  _gl_entities.push_back( glentity );
}

void GL_Pack::freePackGPU()
{
  std::vector< GL_Entity* >::iterator it;

  for(it=_gl_entities.begin();it!=_gl_entities.end();it++)
  {
    (*it)->freeGPU();
    delete (*it);
  }
}
#include "vertexbuffer.h"
#include "renderer.h"

using namespace lc::viewer::opengl;

VertexBuffer::VertexBuffer()
{
  
}

VertexBuffer::~VertexBuffer()
{
   glDeleteBuffers(1,&m_RendererID);
}

void VertexBuffer::Gen(const void* data,unsigned int size)
{	
   glGenBuffers(1,&m_RendererID);
   glBindBuffer(GL_ARRAY_BUFFER,m_RendererID);
   glBufferData(GL_ARRAY_BUFFER, size, data , GL_STATIC_DRAW);
}

void VertexBuffer::Bind() const
{
   glBindBuffer(GL_ARRAY_BUFFER,m_RendererID);  
}

void VertexBuffer::UnBind() const
{
   glBindBuffer(GL_ARRAY_BUFFER,0);  
}
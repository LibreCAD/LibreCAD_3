#include "indexbuffer.h"
using namespace lc::viewer::opengl;

IndexBuffer::IndexBuffer()
{

}

IndexBuffer::~IndexBuffer()
{
   
}

void IndexBuffer::Gen(const unsigned int* data,unsigned int count)
{
	m_Count=count;
   glGenBuffers(1,&m_RendererID);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,m_RendererID);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, (count * sizeof(unsigned int)), data , GL_STATIC_DRAW);

}

void IndexBuffer::Bind() const
{
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,m_RendererID);  
}

void IndexBuffer::UnBind() const
{
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);  
}

void IndexBuffer::FreeGPU() const
{
   UnBind();
   glDeleteBuffers(1,&m_RendererID);
}
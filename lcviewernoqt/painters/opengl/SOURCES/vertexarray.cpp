#include "vertexarray.h"
#include "renderer.h"

#include <iostream>

using namespace lc::viewer::opengl;

VertexArray::VertexArray()
{
    
}

VertexArray::~VertexArray()
{
    
}

void VertexArray::Gen()
{
	glGenVertexArrays(1,&m_RendererID);	
	glBindVertexArray(m_RendererID);
}

void VertexArray::AddBuffer(const VertexBuffer& vb,const VertexBufferLayout& layout)
{
   vb.Bind();  //  layout of this vb (vertex buffer) (binding it for being sure its is the vb)
   
   const auto& elements =layout.GetElements();
   
   unsigned int offset=0;

   for(int i=0;i<elements.size();i++)
   {
   	  const auto& element=elements[i];
   	  
	glVertexAttribPointer(i,element.count,element.type,element.normalized,layout.GetStride(), (const void*)offset);
	glEnableVertexAttribArray(i);

	offset+=element.count * VertexBufferElement::GetSizeOfType(element.type);

   }
}

void VertexArray::Bind() const
{
   glBindVertexArray(m_RendererID);
}
void VertexArray::UnBind() const
{
	glBindVertexArray(0);
}

void VertexArray::FreeGPU() const
{
   UnBind();
   glDeleteVertexArrays(1,&m_RendererID); 
}
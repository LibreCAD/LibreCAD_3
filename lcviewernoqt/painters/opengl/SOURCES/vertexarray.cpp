#include "vertexarray.h"

using namespace lc::viewer::opengl;

VertexArray::VertexArray()
{
    
}

VertexArray::~VertexArray()
{
    
}

void VertexArray::gen()
{
	glGenVertexArrays(1,&_va_id);	
	glBindVertexArray(_va_id);
}

void VertexArray::addBuffer(const VertexBuffer& vb,const VertexBufferLayout& layout)
{
   vb.bind();  //  layout of this vb (vertex buffer) (binding it for being sure its is the vb)
   
   const auto& elements =layout.getElements();
   
   unsigned int offset=0;

   for(int i=0;i<elements.size();i++)
   {
   	  const auto& element=elements[i];
   	  
	glVertexAttribPointer(i,element.count,element.type,element.normalized,layout.getStride(), (const void*)offset);
	glEnableVertexAttribArray(i);

	offset+=element.count * VertexBufferElement::getSizeOfType(element.type);

   }
}

void VertexArray::bind() const
{
   glBindVertexArray(_va_id);
}
void VertexArray::unbind() const
{
	glBindVertexArray(0);
}

void VertexArray::freeGPU() const
{
   unbind();
   glDeleteVertexArrays(1,&_va_id); 
}
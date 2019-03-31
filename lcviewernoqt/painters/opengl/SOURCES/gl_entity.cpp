#include "gl_entity.h"

using namespace lc::viewer::opengl;

GL_Entity :: GL_Entity()
{

}

GL_Entity::~GL_Entity()
{
	
}

void GL_Entity::LoadData(float* vertices,int size,unsigned int* indices,int count)
{

 //--------VAO-----------
  VAO.Gen();

 //--------VBO ------	
  VBO.Gen(vertices , size);

 //---------IB---------
  IBO.Gen(indices, count );
  
 //--------layout--------                  TODO: Here the layout is fixed (only for x,y,z coord)
  VertexBufferLayout layout;              // To be flexible with color, Texture
  layout.Push<float>(3);

 //--------attaching VB and (its)layout to VA---  
  VAO.AddBuffer(VBO,layout);

}

void GL_Entity::Bind()
{
	//---------bind--------------
	VBO.Bind();
    VAO.Bind();
    IBO.Bind();
   
}

void GL_Entity::UnBind()
{
	//---------unbind--------------
    VBO.UnBind();
    VAO.UnBind();
    IBO.UnBind();
}

unsigned int GL_Entity::GetIndices()
{
	return IBO.GetCount();
}

void GL_Entity::ClearData()
{

}

void GL_Entity::Delete()
{
  
  VBO.~VertexBuffer();
  IBO.~IndexBuffer();
  VAO.~VertexArray();

  this->~GL_Entity();
}

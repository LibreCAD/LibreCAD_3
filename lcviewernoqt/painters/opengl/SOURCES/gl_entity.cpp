#include "gl_entity.h"

using namespace lc::viewer::opengl;

GL_Entity :: GL_Entity()
{
  _model=glm::mat4(1.0f);
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

void GL_Entity::SetModelMatrix(glm::mat4 model)
{
   _model=model;
}

void GL_Entity::SetRenderMode(GLenum rendermode)
{
   _render_mode=rendermode;
}

void GL_Entity::SetFillMode(GLenum fillmode)
{
   _fill_mode=fillmode;
}

glm::mat4 GL_Entity::GetModelMatrix()
{
   return _model;
}
  
GLenum GL_Entity::GetRenderMode()
{
   return _render_mode;
}
 
GLenum GL_Entity::GetFillMode()
{
   return _fill_mode;
}


void GL_Entity::Delete()
{
  
  VBO.~VertexBuffer();
  IBO.~IndexBuffer();
  VAO.~VertexArray();

  this->~GL_Entity();
}

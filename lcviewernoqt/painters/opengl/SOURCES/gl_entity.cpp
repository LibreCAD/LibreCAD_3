#include "gl_entity.h"

using namespace lc::viewer::opengl;

GL_Entity :: GL_Entity()
{
    _fill_mode=GL_LINE;
    _render_mode=GL_LINES;
    _linewidth=1.0f;
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

void GL_Entity::ClearData()
{

}

void GL_Entity::SetShader(Shader* shader)
{
   _basic_shader=shader;
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

void GL_Entity::SetLineWidth(float width)
{
    _linewidth=width;
}


void GL_Entity::SetColor(float R,float G,float B,float A)
{
  _basic_shader->Bind();
  _basic_shader->SetUniform4f("u_Color",R,G,B,A);
  _basic_shader->UnBind();
}


void GL_Entity::Delete()
{
  
  VBO.~VertexBuffer();
  IBO.~IndexBuffer();
  VAO.~VertexArray();

  this->~GL_Entity();
}


void GL_Entity::Draw(glm::mat4 _proj,glm::mat4 _view)
{
    //Set the Fill Mode
    glPolygonMode(GL_FRONT_AND_BACK, _fill_mode);

    //Set the Width
    glLineWidth(_linewidth);
    
    //compute the MVP matrix ( received -V ,-P  .. already have -M)
    glm::mat4 mvp=_proj * _view * _model;
    _basic_shader->Bind();
    _basic_shader->SetUniformMat4f("u_MVP",mvp);
  
    // Bind this Entity
    this->Bind();

    //finally draw
     glDrawElements( _render_mode,
                    IBO.GetCount(),
                    GL_UNSIGNED_INT,
                    0 );

   
}
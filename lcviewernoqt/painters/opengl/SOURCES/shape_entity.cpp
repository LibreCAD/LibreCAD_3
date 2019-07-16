#include "shape_entity.h"
#include <QtDebug>
using namespace lc::viewer::opengl;

Shape_Entity :: Shape_Entity()
{
    _fill_mode=GL_LINE;
    _render_mode=GL_LINES;
    _linewidth=1.0f;
    _model=glm::mat4(1.0f);
}

Shape_Entity::~Shape_Entity()
{
	
}

void Shape_Entity::LoadData(float* vertices,int size,unsigned int* indices,int count)
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

void Shape_Entity::Bind()
{
	//---------bind--------------
	VBO.Bind();
    VAO.Bind();
    IBO.Bind();
   
}

void Shape_Entity::UnBind()
{
	//---------unbind--------------
    VBO.UnBind();
    VAO.UnBind();
    IBO.UnBind();
}

void Shape_Entity::ClearData()
{

}

void Shape_Entity::SetShader(Shader* shader)
{
   _basic_shader=shader;
}

void Shape_Entity::SetModelMatrix(glm::mat4 model)
{
   _model=model;
}

void Shape_Entity::SetRenderMode(GLenum rendermode)
{
   _render_mode=rendermode;
}

void Shape_Entity::SetFillMode(GLenum fillmode)
{
   _fill_mode=fillmode;
}

void Shape_Entity::SetLineWidth(float width)
{
    _linewidth=width;
}


void Shape_Entity::SetColor(float R,float G,float B,float A)
{
  _basic_shader->Bind();
  _basic_shader->SetUniform4f("u_Color",R,G,B,A);
  _basic_shader->UnBind();
}

void Shape_Entity::AddLinearGradient(float x0,float y0,float x1,float y1)
{
   // NO Need (Used by Gradient_Entity)
}

void Shape_Entity::AddGradientColorPoint(float R,float G,float B,float A)
{
   // NO Need (Used by Gradient_Entity)
}

void Shape_Entity::ApplyGradient(float* vertices,int size)
{
   // NO Need (Used by Gradient_Entity)
}


void Shape_Entity::Delete()
{
  
  VBO.~VertexBuffer();
  IBO.~IndexBuffer();
  VAO.~VertexArray();

  this->~Shape_Entity();
}


void Shape_Entity::Draw(glm::mat4 _proj,glm::mat4 _view)
{
    //Set the Fill Mode
    glPolygonMode(GL_FRONT_AND_BACK, _fill_mode);

    //Set the Width
   // glLineWidth(_linewidth);
    
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
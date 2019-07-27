#include "shape_entity.h"
#include <QtDebug>
using namespace lc::viewer::opengl;

Shape_Entity :: Shape_Entity()
{
    _fill_mode=GL_LINE;
    _render_mode=GL_LINE_STRIP_ADJACENCY;
    _linewidth=1.0f;
    _model=glm::mat4(1.0f);
}

Shape_Entity::~Shape_Entity()
{
	
}

void Shape_Entity::LoadData(float* vertices,int size,std::vector<int> &jumps)
{
  
 //--------VAO-----------
  VAO.Gen();

 //--------VBO ------	
  VBO.Gen(vertices , size);

  _jumps=jumps;
  
 //--------layout--------                 
  VertexBufferLayout layout;           
  layout.Push<float>(3);      // (x,y,z)         
  layout.Push<float>(1);      // (d)

 //--------attaching VB and (its)layout to VA---  
  VAO.AddBuffer(VBO,layout);

}

void Shape_Entity::Bind()
{
	//---------bind--------------
	VBO.Bind();
  VAO.Bind();
   
}

void Shape_Entity::UnBind()
{
	//---------unbind--------------
    VBO.UnBind();
    VAO.UnBind();
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


void Shape_Entity::FreeGPU()
{
  
  VBO.FreeGPU();
  VAO.FreeGPU();
}



void Shape_Entity::Draw(glm::mat4 _proj,glm::mat4 _view)
{
    //Set the Fill Mode
    

    //Set the Width
   // glLineWidth(_linewidth);
    
    //compute the MVP matrix ( received -V ,-P  .. already have -M)
    glm::mat4 mvp=_proj * _view * _model;
    _basic_shader->Bind();
    _basic_shader->SetUniformMat4f("u_MVP",mvp);
  
    // Bind this Entity
    this->Bind();

    //finally draw
   
    std::vector<int> :: iterator it;
    int l=0;
    for(it=_jumps.begin();it!=_jumps.end();it++)
    {
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      glDrawArrays(_render_mode,l,*(it));

      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      glDrawArrays(_render_mode,l,*(it));

      l+=*(it);

    }          

   
}
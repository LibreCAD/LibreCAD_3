#include "gradient_entity.h"
#include <cmath>
#include <QtDebug>
using namespace lc::viewer::opengl;

Gradient_Entity :: Gradient_Entity()
{
    _fill_mode=GL_LINE;
    _render_mode=GL_LINES;
    _linewidth=1.0f;
    _model=glm::mat4(1.0f);
}

Gradient_Entity::~Gradient_Entity()
{
	
}

void Gradient_Entity::LoadData(float* vertices,int size,unsigned int* indices,int count)
{
  
  ApplyGradient(vertices,count);
  float* colored_vertices=&color_vertex_data[0];
  int new_size=color_vertex_data.size()*sizeof(float); 

 //--------VAO-----------
  VAO.Gen();

 //--------VBO ------	
  VBO.Gen(colored_vertices , new_size);

 //---------IB---------
  IBO.Gen(indices, count );
  
 //--------layout--------                 
  VertexBufferLayout layout;              // To be flexible with color,
  layout.Push<float>(3);                  // (x,y,z)
  layout.Push<float>(4);                  // (r,g,b,a)

 //--------attaching VB and (its)layout to VA---  
  VAO.AddBuffer(VBO,layout);

}

void Gradient_Entity::Bind()
{
	//---------bind--------------
	  VBO.Bind();
    VAO.Bind();
    IBO.Bind();
   
}

void Gradient_Entity::UnBind()
{
	//---------unbind--------------
    VBO.UnBind();
    VAO.UnBind();
    IBO.UnBind();
}

void Gradient_Entity::ClearData()
{

}

void Gradient_Entity::SetShader(Shader* shader)
{
   _gradient_shader=shader;
}

void Gradient_Entity::SetModelMatrix(glm::mat4 model)
{
   _model=model;
}

void Gradient_Entity::SetRenderMode(GLenum rendermode)
{
   _render_mode=rendermode;
}

void Gradient_Entity::SetFillMode(GLenum fillmode)
{
   _fill_mode=fillmode;
}

void Gradient_Entity::SetLineWidth(float width)
{
    _linewidth=width;
}


void Gradient_Entity::SetColor(float R,float G,float B,float A)
{
  // NOTHING
}

void Gradient_Entity::AddLinearGradient(float x0,float y0,float x1,float y1)
{
   pattern = new Linear_Pattern();
   (pattern->begin) = glm::vec2(x0,y0);
   (pattern->end) = glm::vec2(x1,y1);

}

void Gradient_Entity::AddGradientColorPoint(float R,float G,float B,float A)
{
   (pattern->color_points).push_back( glm::vec4(R,G,B,A) );
}

void Gradient_Entity::ApplyGradient(float* vertices,int count)
{
     glm::vec2 Vg= (pattern->end) - (pattern->begin);   // gradient line vector

     float len = glm::length(Vg);                       // length of gradient line vector
     float ratio_R = (((pattern->color_points)[1]).r - ((pattern->color_points)[0]).r)/len;
     float ratio_G = (((pattern->color_points)[1]).g - ((pattern->color_points)[0]).g)/len;
     float ratio_B = (((pattern->color_points)[1]).b - ((pattern->color_points)[0]).b)/len;
     float ratio_A = (((pattern->color_points)[1]).a - ((pattern->color_points)[0]).a)/len;

     float d;           // length of scalar projection of point vector to Vg vector

     glm::vec2 Vp;     // vector for each current point

      for(int i=0;i<count;i++)
     {  
        color_vertex_data.push_back( vertices[3*i]);    // X
        color_vertex_data.push_back( vertices[3*i+1]);  // Y
        color_vertex_data.push_back( vertices[3*i+2]);  // Z

        Vp=glm::vec2( vertices[3*i] - (pattern->begin).x , vertices[3*i+1] - (pattern->begin).y);  //  Current point vector (P-Begin)
        
        float dot_prod=glm::dot( Vg , Vp );              // dot product

        d=abs(dot_prod/len);                                  // final distance of projection of point on Vg

      
        color_vertex_data.push_back( ((pattern->color_points)[0]).r + d * ratio_R );    // R
        color_vertex_data.push_back( ((pattern->color_points)[0]).g + d * ratio_G );    // G
        color_vertex_data.push_back( ((pattern->color_points)[0]).b + d * ratio_B );    // B
        color_vertex_data.push_back( ((pattern->color_points)[0]).a + d * ratio_A );    // A
     }

}


void Gradient_Entity::FreeGPU()
{
  
  VBO.FreeGPU();
  IBO.FreeGPU();
  VAO.FreeGPU();
}


void Gradient_Entity::Draw(glm::mat4 _proj,glm::mat4 _view)
{
    //Set the Fill Mode
    glPolygonMode(GL_FRONT_AND_BACK, _fill_mode);

    //Set the Width
   // glLineWidth(_linewidth);
    
    //compute the MVP matrix ( received -V ,-P  .. already have -M)
    glm::mat4 mvp=_proj * _view * _model;
    _gradient_shader->Bind();
    _gradient_shader->SetUniformMat4f("u_MVP",mvp);
  
    // Bind this Entity
    this->Bind();

    //finally draw
     glDrawElements( _render_mode,
                    IBO.GetCount(),
                    GL_UNSIGNED_INT,
                    0 );
   
   
}
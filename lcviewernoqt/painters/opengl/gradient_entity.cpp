#include "gradient_entity.h"
using namespace lc::viewer::opengl;

Gradient_Entity :: Gradient_Entity()
{
  _fill_mode=GL_FILL;
  _render_mode=GL_TRIANGLE_FAN;
  _linewidth=1.0f;
  _model=glm::mat4(1.0f);
}

Gradient_Entity::~Gradient_Entity()
{
}

void Gradient_Entity::loadVertexData(float* vertices,int size,std::vector<int> &jumps)
{
  int count=size/(4*sizeof(float));
  applyGradient(vertices,count);
  float* colored_vertices=&_color_vertex_data[0];
  int new_size=_color_vertex_data.size()*sizeof(float); 
  _jumps=jumps;

  //--------VAO-----------
  _vao.gen();

  //--------VBO ------  
  _vbo.gen(colored_vertices , new_size);

  //--------layout--------                 
  VertexBufferLayout layout;              // To be flexible with color,
  layout.push<float>(3);                  // (x,y,z)
  layout.push<float>(1);                  // (d)
  layout.push<float>(4);                  // (r,g,b,a)

  //--------attaching VB and (its)layout to VA---  
  _vao.addBuffer(_vbo,layout);
}

void Gradient_Entity::bind()
{
  //---------bind--------------
  _vbo.bind();
  _vao.bind();
}

void Gradient_Entity::unbind()
{
  //---------unbind--------------
  _vbo.unbind();
  _vao.unbind();
}

void Gradient_Entity::setType(Shaders_book& shaders)
{
  _gradient_shader=shaders.gradient_shader;
}

void Gradient_Entity::setModelMatrix(glm::mat4 model)
{
  _model=model;
}

void Gradient_Entity::setFillMode(bool fill)
{
  //No need (gradient entity is always filled till now) ( can be changed later ;) )
}

void Gradient_Entity::setLineWidth(float width)
{
  _linewidth=width;
}

void Gradient_Entity::setDashes(std::vector<float> &dashes, int num_dashes,float sum_dashes)
{
  // NOTHING
}

void Gradient_Entity::setColor(float R,float G,float B,float A)
{
  // NOTHING
}

void Gradient_Entity::addLinearGradient(float x0,float y0,float x1,float y1)
{
  _pattern = new Linear_Pattern();
  (_pattern->begin) = glm::vec2(x0,y0);
  (_pattern->end) = glm::vec2(x1,y1);
}

void Gradient_Entity::addGradientColorPoint(float R,float G,float B,float A)
{
  (_pattern->color_points).push_back( glm::vec4(R,G,B,A) );
}

void Gradient_Entity::applyGradient(float* vertices,int count)
{
  glm::vec2 Vg= (_pattern->end) - (_pattern->begin);   // gradient line vector

  float len = glm::length(Vg);                       // length of gradient line vector
  float ratio_R = (((_pattern->color_points)[1]).r - ((_pattern->color_points)[0]).r)/len;
  float ratio_G = (((_pattern->color_points)[1]).g - ((_pattern->color_points)[0]).g)/len;
  float ratio_B = (((_pattern->color_points)[1]).b - ((_pattern->color_points)[0]).b)/len;
  float ratio_A = (((_pattern->color_points)[1]).a - ((_pattern->color_points)[0]).a)/len;

  float d;           // length of scalar projection of point vector to Vg vector

  glm::vec2 Vp;     // vector for each current point

  for(int i=0;i<count;i++)
  {  
    _color_vertex_data.push_back( vertices[4*i]);    // X
    _color_vertex_data.push_back( vertices[4*i+1]);  // Y
    _color_vertex_data.push_back( vertices[4*i+2]);  // Z
    _color_vertex_data.push_back( vertices[4*i+3]);  // d

    Vp=glm::vec2( vertices[4*i] - (_pattern->begin).x , vertices[4*i+1] - (_pattern->begin).y);  //  Current point vector (P-Begin)
        
    float dot_prod=glm::dot( Vg , Vp );              // dot product

    d=abs(dot_prod/len);                                  // final distance of projection of point on Vg
  
    _color_vertex_data.push_back( ((_pattern->color_points)[0]).r + d * ratio_R );    // R
    _color_vertex_data.push_back( ((_pattern->color_points)[0]).g + d * ratio_G );    // G
    _color_vertex_data.push_back( ((_pattern->color_points)[0]).b + d * ratio_B );    // B
    _color_vertex_data.push_back( ((_pattern->color_points)[0]).a + d * ratio_A );    // A
  }
}

void Gradient_Entity::setFont(Font_Book& fonts,const std::string& style)
{
   // NO Need (Used by Tex_Entity)
}

void Gradient_Entity::addTextData(glm::vec4 pos, std::string text_val , float font_size, bool retain)
{
   // NO Need (Used by Text_Entity)
}

void Gradient_Entity::freeGPU()
{
  _vbo.freeGPU();
  _vao.freeGPU();
}

void Gradient_Entity::draw(glm::mat4 _proj,glm::mat4 projB,glm::mat4 _view)
{
  //Set the Fill Mode
  glPolygonMode(GL_FRONT_AND_BACK, _fill_mode);

  //compute the MVP matrix ( received -V ,-P  .. already have -M)
  glm::mat4 mvp=_proj * _view * _model;
  _gradient_shader->bind();
  _gradient_shader->setUniformMat4f("u_MVP",mvp);
  
  // Bind this Entity
  this->bind();

  //finally draw
 
  std::vector<int> :: iterator it;
  int l=0;
  for(it=_jumps.begin();it!=_jumps.end();it++)
  {
    glDrawArrays(_render_mode,l,*(it));
    l+=*(it);
  }          
}
#include "shape_entity.h"

using namespace lc::viewer::opengl;

Shape_Entity :: Shape_Entity()
{
  _fill_mode=GL_LINE;
  _render_mode=GL_LINE_STRIP_ADJACENCY;
    
  _model=glm::mat4(1.0f);

  _linewidth=1.0f;
                     
  _dashes_sum=0.0f;  
  _dashes_size=0;              
    
  _type=Entity_Type::BASIC;  
  _shader=NULL;              
}

Shape_Entity::~Shape_Entity()
{
  //destructor
}

void Shape_Entity::loadVertexData(float* vertices,int size,std::vector<int> &jumps)
{
  //--------VAO-----------
  _vao.gen();

  //--------VBO ------  
  _vbo.gen(vertices , size);

  _jumps=jumps;
  
  //--------layout--------                 
  VertexBufferLayout layout;           
  layout.push<float>(3);      // (x,y,z)         
  layout.push<float>(1);      // (d)

  //--------attaching VB and (its)layout to VA---  
  _vao.addBuffer(_vbo,layout);
}

void Shape_Entity::bind()
{
  //---------bind--------------
  _vbo.bind();
  _vao.bind(); 
}

void Shape_Entity::unbind()
{
  //---------unbind--------------
  _vbo.unbind();
  _vao.unbind();
}

void Shape_Entity::setType(Shaders_book& shaders)
{
  if( _type == Entity_Type::BASIC )         // By default
  {
    _shader=shaders.basic_shader;
    _fill_mode=GL_LINE;
    _render_mode=GL_LINE_STRIP_ADJACENCY;
  }

  else if( _type == Entity_Type::FILL )         // Filled shape
  {
    _shader=shaders.basic_shader;
    _fill_mode=GL_FILL;
    _render_mode=GL_TRIANGLE_FAN;
  }

  else if( _type == Entity_Type::THICK )         // Thickline
  {
    _shader=shaders.thickline_shader;
    _fill_mode=GL_FILL;
    _render_mode=GL_LINE_STRIP_ADJACENCY;
  }

  else if( _type == Entity_Type::PATTERN )         // Pattern
  {
    _shader=shaders.linepattern_shader;
    _fill_mode=GL_FILL;
    _render_mode=GL_LINE_STRIP_ADJACENCY;
  }
}

void Shape_Entity::setModelMatrix(glm::mat4 model)
{
  _model=model;
}

void Shape_Entity::setFillMode(bool fill)
{
  if(fill==true)
    _type = Entity_Type::FILL;      // Will set the type to FILL
}

void Shape_Entity::setLineWidth(float width)
{
  _linewidth=width;
    
  if(width>1.0f)
  {
    _type = Entity_Type::THICK;   // if width>1 change type to THICK
  } 
}

void Shape_Entity::setDashes(std::vector<float> &dashes, int num_dashes,float sum_dashes)
{
  if(num_dashes>0)
  {  
    _dashes_size=num_dashes;
    _dashes_sum=sum_dashes;
    _dashes=dashes;
     
    _type = Entity_Type::PATTERN; 
  }
}


void Shape_Entity::setColor(float R,float G,float B,float A)
{
  // NO Need
}

void Shape_Entity::addLinearGradient(float x0,float y0,float x1,float y1)
{
  // NO Need (Used by Gradient_Entity)
}

void Shape_Entity::addGradientColorPoint(float R,float G,float B,float A)
{
  // NO Need (Used by Gradient_Entity)
}

void Shape_Entity::applyGradient(float* vertices,int size)
{
  // NO Need (Used by Gradient_Entity)
}

void Shape_Entity::setFont(Font_Book& fonts,const std::string& style)
{
  // NO Need (Used by Tex_Entity)
}

void Shape_Entity::addTextData(glm::vec4 pos, std::string text_val , float font_size, bool retain)
{
  // NO Need (Used by Text_Entity)
}

void Shape_Entity::freeGPU()
{
  _vbo.freeGPU();
  _vao.freeGPU();
}

void Shape_Entity::draw(glm::mat4 _proj,glm::mat4 projB,glm::mat4 _view)
{
  //Set the Fill Mode
  glPolygonMode(GL_FRONT_AND_BACK, _fill_mode);
  
  _shader->bind();
   
  //compute the MVP matrix ( received -V ,-P  .. already have -M)
  glm::mat4 mvp=_proj * _view * _model;
   
  _shader->setUniformMat4f("u_MVP",mvp);  // Set MVP

  //----------------------------------------------------------

  if( _type == Entity_Type::THICK )      //Set the Width (if it is THICK)
  {
    _shader->setUniform1f("u_W",_linewidth);      
    // _shader->setUniform1f("u_W",std::max(_linewidth,_linewidth*_view[2][2] ) );  // If width is magnified.. 
  }

  if(_type == Entity_Type::PATTERN )      // Set Width,projB,Dashes (If it is PATTERN)
  {
    _shader->setUniform1f("u_W",_linewidth);
    //_shader->setUniform1f("u_W",std::max(_linewidth,_linewidth*_view[2][2] ) );  // If width is magnified.. 
    
    _view[3][0]=0;
    _view[3][1]=0;  //neglect translations

    glm::mat4 scale=glm::scale(glm::mat4(1.0f),glm::vec3(_view[2][2],_view[2][2],_view[2][2]));

    _shader->setUniformMat4f("u_X", (projB*scale) );  // Set u_X

    // SET the dashes
    _shader->setUniform1fv("dashes",_dashes_size,&_dashes[0]);
    _shader->setUniform1i("dashes_size",_dashes_size);
    _shader->setUniform1f("dashes_sum",_dashes_sum);
    
  }
  
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
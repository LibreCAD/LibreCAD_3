#include "text_entity.h"

using namespace lc::viewer::opengl;

Text_Entity :: Text_Entity()
{
  _shader=NULL;  
  _no_magnify=false; 
  _model=glm::mat4(1.0f); 
}

Text_Entity::~Text_Entity()
{
}

void Text_Entity::loadVertexData(float* vertices,int size,std::vector<int> &jumps)
{
  //---Nothing Text_Entity has no vertex data of own
}

void Text_Entity::bind()
{
  // text Entity have no own BOs
}

void Text_Entity::unbind()
{
  // text Entity have no own BOs
}

void Text_Entity::setType(Shaders_book& shaders)
{
  _shader=shaders.text_shader;   
}

void Text_Entity::setModelMatrix(glm::mat4 model)
{
  _model=model;
}

void Text_Entity::setFillMode(bool fill)
{
  // NO Need (Used by Shape_Entity)
}

void Text_Entity::setLineWidth(float width)
{
  // NO Need (Used by Shape_Entity)
}

void Text_Entity::setDashes(std::vector<float> &dashes, int num_dashes,float sum_dashes)
{
  // NO Need (Used by Shape_Entity)
}

void Text_Entity::setColor(float R,float G,float B,float A)
{
  //No Need
}

void Text_Entity::addLinearGradient(float x0,float y0,float x1,float y1)
{
  // NO Need (Used by Gradient_Entity)
}

void Text_Entity::addGradientColorPoint(float R,float G,float B,float A)
{
  // NO Need (Used by Gradient_Entity)
}

void Text_Entity::applyGradient(float* vertices,int size)
{
  // NO Need (Used by Gradient_Entity)
}

void Text_Entity::setFont(Font_Book& fonts,const std::string& style)
{
  _font= fonts.pickFont(style);  //default
}

void Text_Entity::addTextData(glm::vec4 pos , std::string textval , float font_size, bool retain)
{   
  _text=textval;
  _no_magnify=retain;

  _model=glm::translate( _model,glm::vec3(pos.x,pos.y,pos.z));  // First Translate at pos

  _model=glm::scale( _model,glm::vec3(font_size/64,font_size/64,font_size/64) );   // Scale according to font height
}

void Text_Entity::freeGPU()
{
  // text Entity have no own BOs
}

void Text_Entity::draw(glm::mat4 _proj,glm::mat4 projB,glm::mat4 _view)
{
  //Set the Fill Mode
  // glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    
  //making a temp model matrix 
  glm::mat4 temp_model=_model;

  // If not magnified
  if(_no_magnify)
    temp_model=glm::scale(temp_model,glm::vec3(1.0f/_view[2][2],1.0f/_view[2][2],1.0f/_view[2][2]) );

  //Finally Render Text
  _font->renderText( _text, _proj, _view, temp_model,_shader);
}
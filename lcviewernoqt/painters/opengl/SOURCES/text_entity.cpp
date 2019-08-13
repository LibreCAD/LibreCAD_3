#include "text_entity.h"
#include <QtDebug>
using namespace lc::viewer::opengl;

Text_Entity :: Text_Entity()
{
    _shader=NULL;   
    _model=glm::mat4(1.0f); 
    _text="LibreCAD 3.0";      
}

Text_Entity::~Text_Entity()
{
	
}

void Text_Entity::LoadVertexData(float* vertices,int size,std::vector<int> &jumps)
{
  
 //---Nothing

}

void Text_Entity::Bind()
{
	
}

void Text_Entity::UnBind()
{
	
}

void Text_Entity::SetType(Shaders_book& shaders)
{
  _shader=shaders.text_shader;   
}

void Text_Entity::SetModelMatrix(glm::mat4 model)
{
   _model=model;
}


void Text_Entity::SetFillMode(bool fill)
{
   // NO Need (Used by Shape_Entity)
}

void Text_Entity::SetLineWidth(float width)
{
   // NO Need (Used by Shape_Entity)
}

void Text_Entity::SetDashes(std::vector<float> &dashes, int num_dashes,float sum_dashes)
{
   // NO Need (Used by Shape_Entity)
}


void Text_Entity::SetColor(float R,float G,float B,float A)
{
 
}

void Text_Entity::AddLinearGradient(float x0,float y0,float x1,float y1)
{
   // NO Need (Used by Gradient_Entity)
}

void Text_Entity::AddGradientColorPoint(float R,float G,float B,float A)
{
   // NO Need (Used by Gradient_Entity)
}

void Text_Entity::ApplyGradient(float* vertices,int size)
{
   // NO Need (Used by Gradient_Entity)
}

void Text_Entity::SetFont(Font_Book& fonts,const std::string& style)
{
   _font= fonts.Pick_Font(style);  //default
}

void Text_Entity::AddTextData(float pen_x,float pen_y, const char* text_val , float font_size, bool is_magnified)
{
   // NO Need (Used by Text_Entity)
}


void Text_Entity::FreeGPU()
{
  
}


void Text_Entity::Draw(glm::mat4 _proj,glm::mat4 projB,glm::mat4 _view)
{
    //Set the Fill Mode
   // glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  
    //if not magnified
   // _view=glm::scale(_view,glm::vec3(1.0f/_view[2][2],1.0f/_view[2][2],1.0f/_view[2][2]) );

   //Finally Render Text
    _font->RenderText( _text, _proj, _view, _model,_shader);
   
}
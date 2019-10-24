#include "cacher.h"
using namespace lc::viewer::opengl;

Cacher::Cacher()
{
  _model=glm::mat4(1.0f);
  readyFreshPack();
}

Cacher::~Cacher()
{
}

void Cacher::setShaderBook(struct Shaders_book& book)
{
  _shaders=book;
}

void Cacher::setFontBook(Font_Book& book)
{
  _fonts=book;
}

//---------------------------For Matrix/ Vectors/ Coordinate-----------
void Cacher::updateScale(float scale)
{
  _model=glm::scale(_model,glm::vec3(scale,scale,scale));
}

void Cacher::updateTranslate(float x,float y)
{
  _model=glm::translate(_model,glm::vec3(x,y,0.0));
}

void Cacher::updateRotate(float angle)
{
  _model=glm::rotate(_model, angle, glm::vec3(0.0f ,0.0f ,1.0f) );
}

void Cacher::resetTransformations()
{
  _model=glm::mat4(1.0f);
}

double Cacher::getScale()
{
  return _model[2][2];
}

double Cacher::getTranslateX()
{
  return _model[3][0];
}

double Cacher::getTranslateY()
{
  return _model[3][1];
}   

//---------------------------------------------------------------
void Cacher::readyCurrentEntity()
{
  getCurrentEntity()->setModelMatrix(_model); 
  addDataToCurrentEntity();
  getCurrentEntity()->setType(_shaders);
  getCurrentEntity()->setFont(_fonts,_font_style);
}

void Cacher::selectColor(float R,float G,float B,float A)
{
  // Till now dont need to be cached
}

GL_Text_Extend Cacher::getTextExtend(const char* text_val)
{
  GL_Text_Extend TE=(_fonts.pickFont(_font_style))->getTextExtend(std::string(text_val),_text_height);
  return TE;
}

//--------------------------gl_entity / gl_pack / reset manipulations------------
void Cacher::setNewPack()
{
  _current_gl_pack = new GL_Pack();
}

void Cacher::pushEntityInPack()
{
  _current_gl_pack->pushEntityInPack( getCurrentEntity() );
}

void Cacher::readyForNextEntity()
{   
  readyCurrentEntity();
  pushEntityInPack();
  _model=glm::mat4(1.0f);
  setDefault();
  setNewShapeEntity();
}

void Cacher::readyFreshPack()
{
  _model=glm::mat4(1.0f);
  setDefault();
  setNewPack();
  setNewShapeEntity();
}

//--------------------------------cache entity pack-----------------
void Cacher::savePack(unsigned long id)
{
  _gl_pack_map.insert(std::make_pair( id, _current_gl_pack ) );
  readyFreshPack();
}

//--------------------------------caching query/insert/delete------------

bool Cacher::isPackCached(unsigned long id)
{
  std::map< unsigned long, GL_Pack* >::iterator it;
        
  it = _gl_pack_map.find(id);
        
  if (it != _gl_pack_map.end())
    return true;
  else
    return false; 
}

GL_Pack* Cacher::getCachedPack(unsigned long id)
{
  std::map< unsigned long, GL_Pack* >::iterator it;
        
  it = _gl_pack_map.find(id);
      
  if (it != _gl_pack_map.end())
    return (it->second);
  else
    return NULL;  
}

void Cacher::erasePack(unsigned long id)
{
  std::map< unsigned long, GL_Pack* >::iterator it;
        
  it = _gl_pack_map.find(id);
        
  if (it != _gl_pack_map.end())
  { 
    (it->second)->freePackGPU();
    _gl_pack_map.erase(it);
  }
}

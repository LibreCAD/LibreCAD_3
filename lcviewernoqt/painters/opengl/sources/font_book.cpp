#include "font_book.h"

using namespace lc::viewer::opengl;

Font_Book::Font_Book()
{
   
}

Font_Book::~Font_Book()
{
  
}

bool Font_Book::createDefaultFont(const std::string& name,const std::string& ttf_path)
{
  _default_font=new GL_Font();

  if(_default_font->readyTTF(ttf_path))
  {
    _font_map.insert(std::make_pair( name , _default_font ) );
    return true;
  }

  return false;
}

bool Font_Book::createFont(const std::string& name,const std::string& ttf_path)
{
  GL_Font* ttf_font=new GL_Font();

  if(ttf_font->readyTTF(ttf_path))
  {
    _font_map.insert(std::make_pair( name , ttf_font ) );
    return true;
  }

  return false;
}

GL_Font* Font_Book::pickFont(const std::string& font_name)
{
  std::map<const std::string, GL_Font* >::iterator it;
    
    it = _font_map.find(font_name);
    
    if (it != _font_map.end())
      return (it->second);
    else
      return _default_font;  
}
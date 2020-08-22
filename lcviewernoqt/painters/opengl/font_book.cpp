#include "font_book.h"

using namespace lc::viewer::opengl;

Font_Book::Font_Book()
{
}

Font_Book::~Font_Book()
{
}

bool Font_Book::createDefaultFont(const std::string& name,const std::string& file_path)
{
    _default_font=new GL_Font();

    if(_default_font->readyFont(file_path))
    {
        _font_map.insert(std::make_pair( name, _default_font ) );
        return true;
    }

    return false;
}

bool Font_Book::createFont(const std::string& name,const std::string& file_path)
{
    GL_Font* font=new GL_Font();

    if(font->readyFont(file_path))
    {
        _font_map.insert(std::make_pair( name, font ) );
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

std::vector<std::string> Font_Book::getFontList() const {
    std::vector<std::string> result;
    for (const auto& font : _font_map) {
        result.push_back(font.first);
    }
    return result;
}

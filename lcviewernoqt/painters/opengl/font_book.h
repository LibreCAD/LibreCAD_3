#ifndef GL_FONT_BOOK_H
#define GL_FONT_BOOK_H
#include "gl_font.h"
#include <map>
#include <vector>
#include <boost/filesystem.hpp>

namespace lc
{
namespace viewer
{
namespace opengl
{
class Font_Book
{
private:
    std::map<const std::string, GL_Font* > _font_map;
    GL_Font* _default_font=NULL;

public:
    Font_Book();
    ~Font_Book();
    bool createDefaultFont(const std::string& name,const std::string& file_path);
    bool createFont(const std::string& name,const std::string& file_path);
    bool createFontsFromDir(const std::string& directoryPath);
    GL_Font* pickFont(const std::string& font_style);
    std::vector<std::string> getFontList() const;

private:
    void createFontFromEntry(boost::filesystem::directory_entry& entry, const std::string& directoryPath);
};
}
}
}
#endif // FONT_BOOK_H

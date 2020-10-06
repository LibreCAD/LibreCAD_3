#ifndef GL_FONT_H
#define GL_FONT_H

#define GL_GLEXT_PROTOTYPES
#include <GL/glew.h>
#include <GL/gl.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "shader.h"
#include <map>

namespace lc
{
namespace viewer
{
namespace opengl
{
struct GL_Text_Extend   // Info of cobined "string" glyph(s)
{
    int x_bearing;
    int y_bearing;
    int width;
    int height;
    int x_advance;
    int y_advance;
};

struct Character     // Info of a single character glyph
{
    GLuint     textureID;  // ID handle of the glyph texture
    GLuint     vbo;        //VertexBuffer
    GLuint     vao;        //Vertex Array

    int x_bearing;
    int y_bearing;
    int width;
    int height;
    int x_advance;   // Offset to advance to next glyph
    int y_advance;
};

class GL_Font
{
private:
    std::map<unsigned int, Character> _characters;

public:
    GL_Font();
    ~GL_Font();

    bool readyFont(const std::string& path, std::string& fontFamily, std::string& fontStyle);
    void renderText(std::string text,
                    glm::mat4 proj,
                    glm::mat4 view,
                    glm::mat4 model,
                    Shader* text_shader);
    GL_Text_Extend getTextExtend(std::string text, int font_size);
};
}
}
}

#endif // GL_FONT_H

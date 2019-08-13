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


  struct Character 
  {
    GLuint     TextureID;  // ID handle of the glyph texture
    GLuint     VBO;        //VertexBuffer
    GLuint     VAO;        //Vertex Array
    int        Advance_x;  // Offset to advance to next glyph
  };

class GL_Font
{
   private:
  
  std::map<GLchar, Character> _characters;

  
 public:
   	GL_Font();
   ~GL_Font();
   
   bool ReadyTTF(const std::string& path);
   void RenderText(std::string text, 
                   glm::mat4 proj,
                   glm::mat4 view,
                   glm::mat4 model,
                   Shader* text_shader);
};

   }

  }

 } 

#endif // GL_FONT_H
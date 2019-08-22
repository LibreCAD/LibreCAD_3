#include "gl_font.h"
#include <QString>
using namespace lc::viewer::opengl;

GL_Font::GL_Font()
{
   
}

GL_Font::~GL_Font()
{
  
}

bool GL_Font::readyTTF(const std::string& path)
{
  const char* font_path= path.c_str();

  FT_Library ft;
  FT_Face face;

  if (FT_Init_FreeType(&ft))
  {
      //("ERROR::FREETYPE: Could not init FreeType Library");
      
  }

  if (FT_New_Face(ft, font_path , 0, &face))
  {
      //("ERROR::FREETYPE: Failed to load font");  
  }

   FT_Set_Pixel_Sizes(face, 64,64);  


  float xpos,ypos,w,h;
  
  //=========================================
   glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // Disable byte-alignment restriction
  
   for (unsigned int c = 0; c < 256; c++)
  {

      // Load character glyph 
      if (FT_Load_Char(face, c, FT_LOAD_RENDER))
      {
          //("ERROR::FREETYTPE: Failed to load Glyph");
          continue;
      }
     // Generate texture
      GLuint texture;
      glGenTextures(1, &texture);
      glBindTexture(GL_TEXTURE_2D, texture);
    
     // Set texture options   
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);  
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
   
     // make texture of bitmap data
     glTexImage2D(
                  GL_TEXTURE_2D,
                  0,
                  GL_RED,
                  face->glyph->bitmap.width,
                  face->glyph->bitmap.rows,
                  0,
                  GL_RED,
                  GL_UNSIGNED_BYTE,
                  face->glyph->bitmap.buffer
                );

     glGenerateMipmap(GL_TEXTURE_2D);

      //Now saving coordinate data for the glypph

       xpos =    (float)(face->glyph->bitmap_left);
       ypos =    (float)(face->glyph->bitmap_top ) - (float)(face->glyph->bitmap.rows);
       

       w = face->glyph->bitmap.width;
       h = face->glyph->bitmap.rows;

       float _data[6*5] = {
             xpos,     -(ypos + h), 0.0f,   0.0, 0.0 ,            
             xpos,     -(ypos),     0.0f,   0.0, 1.0 ,
             xpos + w, -(ypos),     0.0f,   1.0, 1.0 ,

             xpos,     -(ypos + h), 0.0f,   0.0, 0.0 ,
             xpos + w, -(ypos),     0.0f,   1.0, 1.0 ,
             xpos + w, -(ypos + h), 0.0f,   1.0, 0.0            
        };
        


    //Now Pass the glyphs coordinate data to GPU

    GLuint vbo;
    GLuint vao;    

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    
    glBufferData(GL_ARRAY_BUFFER, (sizeof(GLfloat) * 6 * 5) , &_data[0] , GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), 0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (const void*)( 3 * sizeof(GLfloat))) ;
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);   

    //Finally Save the Character Data

    Character ch = { texture, vbo ,vao , face->glyph->advance.x };
      
    _characters.insert(std::pair<unsigned int, Character>(c, ch));   
     

   }

   //Finished working with Freetype
   FT_Done_Face(face);
   FT_Done_FreeType(ft);

   return true;
   
}


void GL_Font::renderText(std::string text,glm::mat4 proj,glm::mat4 view,glm::mat4 model,Shader* text_shader)
{
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    std::map<unsigned int, Character>::iterator it;
    
    text_shader->bind();
    text_shader->setUniform1i("u_Texture",0);  // same slot of texture (optional)

    glActiveTexture(GL_TEXTURE0);
    
    // converting to std::wstring
    QString qs=QString::fromUtf8(text.c_str());
    std::wstring w_text=qs.toStdWString();
    
    // Iterate through all characters
    std::wstring::const_iterator c;
    for (c = w_text.begin(); c != w_text.end(); c++)
    {
        
        it=_characters.find(*c);

        if(it==_characters.end())
          it=_characters.find(' ');

        //compute the MVP matrix ( received -V ,-P  .. already have -M)
        glm::mat4 mvp=proj * view * model;
   
       text_shader->setUniformMat4f("u_MVP",mvp);  // Set MVP

        // Render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ((it->second)).textureID);
        
        // Bind the VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, ((it->second)).vbo);
        
        // Bind the VAO memory
        glBindVertexArray(((it->second)).vao);

        // Finally Render
        glDrawArrays(GL_TRIANGLES, 0, 6);

        //Unbind
        glBindBuffer(GL_ARRAY_BUFFER,0);
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
        
        // Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        model=glm::translate(model,glm::vec3( ( (it->second).advance_x >> 6 ),0.0,0.0));
        
    }

    text_shader->unbind();
   
    
}

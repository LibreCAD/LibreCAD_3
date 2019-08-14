#include "gl_font.h"
#include <QtDebug>
using namespace lc::viewer::opengl;

GL_Font::GL_Font()
{
   qDebug("GL_Font constructor");
}

GL_Font::~GL_Font()
{
  
}

bool GL_Font::ReadyTTF(const std::string& path)
{
  const char* font_path= path.c_str();

  qDebug("GL_Font ReadyCharacters");

  FT_Library ft;
  FT_Face face;

  if (FT_Init_FreeType(&ft))
  {
      qDebug("ERROR::FREETYPE: Could not init FreeType Library");
      
  }

  if (FT_New_Face(ft, font_path , 0, &face))
  {
      qDebug("ERROR::FREETYPE: Failed to load font");  
  }

   FT_Set_Pixel_Sizes(face, 64,64);  


  float xpos,ypos,w,h;
  
  //=========================================
   glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // Disable byte-alignment restriction
  
   for (GLubyte c = 0; c < 128; c++)
  {

      // Load character glyph 
      if (FT_Load_Char(face, c, FT_LOAD_RENDER))
     {
          qDebug("ERROR::FREETYTPE: Failed to load Glyph");
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

    /* qDebug("%d [ %c ] = %d , %d , %d , %d , %d , %d",c,c,
                                                  face->glyph->bitmap.width,
                                                  face->glyph->bitmap.rows,
                                                  face->glyph->bitmap_left, 
                                                  face->glyph->bitmap_top,
                                                  face->glyph->advance.x,
                                                  face->glyph->advance.y);
      */
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
      
    _characters.insert(std::pair<GLchar, Character>(c, ch));   
     

   }

   //Finished working with Freetype
   FT_Done_Face(face);
   FT_Done_FreeType(ft);

   return true;
   
}


void GL_Font::RenderText(std::string text,glm::mat4 proj,glm::mat4 view,glm::mat4 model,Shader* text_shader)
{
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    std::map<GLchar, Character>::iterator it;
    
    text_shader->Bind();
    text_shader->SetUniform1i("u_Texture",0);  // same slot of texture (optional)

    glActiveTexture(GL_TEXTURE0);
   
    // Iterate through all characters
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++)
    {
      
        it=_characters.find(*c);

        if(it==_characters.end())
          it=_characters.find(' ');

        //compute the MVP matrix ( received -V ,-P  .. already have -M)
        glm::mat4 mvp=proj * view * model;
   
       text_shader->SetUniformMat4f("u_MVP",mvp);  // Set MVP

        // Render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ((it->second)).TextureID);
        
        // Bind the VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, ((it->second)).VBO);
        
        // Bind the VAO memory
        glBindVertexArray(((it->second)).VAO);

        // Finally Render
        glDrawArrays(GL_TRIANGLES, 0, 6);

        //Unbind
        glBindBuffer(GL_ARRAY_BUFFER,0);
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
        
        // Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        model=glm::translate(model,glm::vec3( ( (it->second).Advance_x >> 6 ),0.0,0.0));
        
    }

    text_shader->UnBind();
   
    
}

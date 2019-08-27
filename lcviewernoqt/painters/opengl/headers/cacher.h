#ifndef CACHER_H
#define CACHER_H

#include "gl_pack.h"
#include "shader.h"
#include "font_book.h"
#include "gl_font.h"
#include "manager.h"
namespace lc
{
  namespace viewer
  {
    namespace opengl
    {
      #define PI 3.14159265
      class Cacher: public Manager
      {
      private:   
       
        glm::mat4 _model;                     //model matrix
   
        GL_Pack* _current_gl_pack;

        Shaders_book _shaders;
        Font_Book _fonts;

        std::map < unsigned long, GL_Pack* > _gl_pack_map;

      public:
        Cacher();
        ~Cacher();

        void setShaderBook(struct Shaders_book& book);
        void setFontBook(Font_Book& book);
  
        //---------------------------For Matrix/ Vectors/ Coordinate-----------
        void updateScale(float scale);
        void updateTranslate(float x,float y);
        void updateRotate(float angle);
        void resetTransformations();
        double getScale();
        double getTranslateX();
        double getTranslateY();    

        //------------------------------------for properties ( painter calls)------------------------------------
        void readyCurrentEntity();                      
        void selectColor(float R,float G,float B,float A);

        GL_Text_Extend getTextExtend(const char* text_val);

        //--------------------------gl_entity / gl_pack / reset manipulations------------
        void setNewPack();
        void pushEntityInPack();
        void readyForNextEntity();
        void readyFreshPack();

        //--------------------------------caching query/insert/delete------------
        void savePack(unsigned long id);
        bool isPackCached(unsigned long id);
        GL_Pack* getCachedPack(unsigned long id);
        void erasePack(unsigned long id);
      };
    }
  }
}
#endif // CACHER_H
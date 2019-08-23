#ifndef CACHER_H
#define CACHER_H
#include <GL/glew.h>  
#include <GL/gl.h>
#include <signal.h> 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <utility>
#include <iterator> 
#include <map> 
#include <cmath>

#include "shape_entity.h"
#include "gradient_entity.h"
#include "text_entity.h"
#include "gl_entity.h"
#include "gl_pack.h"
#include "shader.h"
#include "font_book.h"
#include "gl_font.h"

namespace lc
{
  namespace viewer
  {
    namespace opengl
    {
      #define PI 3.14159265

      class Cacher
      {
      private:   
        //----------------------------------FOR VERTEX DATA------------
        std::vector< glm::vec4 > _vertex_data;       // for vertex data
        std::vector<int> _jumps;
        std::vector< glm::vec4 > _current_vertices;  // for current shape(continous)
  
        float _path_distance;                  //distance(scalar) from start point to current
        bool _closed=false;
  
        //------------------------------------FOR DASH DATA--------------
        float _line_width;
        std::vector<float> _dashes_data; float _dashes_sum=0; int _dashes_size=0;
        bool _fill=false;
  
        //--------------------------------------FOR TEXT DATA-----------------
        std::string _text_value;
        std::string _font_style;
        float _text_height;
        bool _no_text_magnify;

        //--------------------------------------------------------------
        glm::mat4 _model;                     //model matrix
   
        GL_Entity* _current_gl_entity;
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
        void updateModel();

        //-----------
        void updateScale(float scale);
        void updateTranslate(float x,float y);
        void updateRotate(float angle);
        void resetTransformations();
        double getScale();
        double getTranslateX();
        double getTranslateY();    

        //---------------Functions manipulating vertex data(raw)----------------------
        void addVertex(float x,float y,float z=0.0f);
        void appendVertexData();
        void jump();
        void clearData();
        void closeLoop();

        //------------------------------------for properties ( painter calls)------------------------------------
        void addDataToCurrentEntity();                      
        void selectFill();
        void selectColor(float R,float G,float B,float A);
        void selectLineWidth(float width);
        void selectDashes(const double* dashes, const int num_dashes, double offset, bool scaled);
        void selectFontSize(float size, bool deviceCoords);
        void selectFontFace(const char* text_style);
        void selectFontValue(const char* text_val);
        GL_Text_Extend getTextExtend(const char* text_val);

        //----------------------------gradient------------------------------
        void addLinearGradient(float x0,float y0,float x1,float y1);
        void addGradientColorPoint(float R,float G,float B,float A);

        //--------------------------gl_entity / gl_pack / reset manipulations------------
        void setNewPack();
        void setNewShapeEntity();
        void setNewGradientEntity();
        void setNewTextEntity();
        void pushEntityInPack();
        void setDefault();
        void readyForNextEntity();
        void readyFreshPack();

        //--------------------------------caching query/insert/delete------------
        void savePack(unsigned long id);
        bool isPackCached(unsigned long id);
        GL_Pack* getCachedPack(unsigned long id);
        void erasePack(unsigned long id);

        //-------debug------
        void logCachedPacks(); 

      };


    }

  }

}
#endif // CACHER_H
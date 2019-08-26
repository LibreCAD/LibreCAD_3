#ifndef RENDERER_H
#define RENDERER_H
#include <GL/glew.h>  
#include <GL/gl.h>
#include <signal.h> 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <stack>
#include <vector>
#include <cmath>

#include "gl_entity.h"
#include "shape_entity.h"
#include "gradient_entity.h"
#include "text_entity.h"
#include "shader.h"
#include "gl_pack.h"
#include "cacher.h"
#include "gl_font.h"
#include "font_book.h"
#include "viewersettings.h"
#include "manager.h"
namespace lc
{
  namespace viewer
  {
    namespace opengl
    {
      struct context_att
      {
        glm::mat4 ctm;                        //CTM
      };

      #define PI 3.14159265

      class Renderer: public Manager
      {
      private: 
        //---------------------------------------------------------------------------
        glm::mat4 _proj;                      //projection matrix
        glm::mat4 _projB;                     //projectionB for seamless line pattern
        glm::mat4 _view;                      //view matrix for pan
        glm::mat4 _ctm;          

        std::stack < context_att > _context_stack;
  
        //--------------------------------------------------------
        std::string _shader_path;
        std::string _font_path;
        Shaders_book _shaders;

        Font_Book _fonts;

        Cacher* _cacherPtr;
  
      public:
        Renderer();
        ~Renderer();
        void createResources();
        void setCacherRef(Cacher* ch);

        //---------------------------For Matrix/ Vectors/ Coordinate-----------
        void updateProjection(float l,float r,float b,float t);
        void updateView();

        //-----------For Updaing Matrix
        void updateScale(float scale);
        void updateTranslate(float x,float y);
        void updateRotate(float angle);
        void resetTransformations();
        double getScale();
        double getTranslateX();
        double getTranslateY();    

        //----------
        void deviceToUser(double* x, double* y);
        void userToDevice(double* x, double* y);
        void deviceToUserDistance(double* x, double* y);
        void userToDeviceDistance(double* x, double* y);

        //------------context handling-------------------------------------
        void save();
        void restore();

        void readyCurrentEntity();
  
        void selectColor(float R,float G,float B,float A);
        
        GL_Text_Extend getTextExtend(const char* text_val);

        //----------------------------render direct entity------------------
        void render();
        
        //-----------------------------rendering cached entities---------------
        void renderCachedEntity(GL_Entity* entity);
        void renderCachedPack(GL_Pack* pack);
      };
    }
  }
}
#endif // RENDERER_H
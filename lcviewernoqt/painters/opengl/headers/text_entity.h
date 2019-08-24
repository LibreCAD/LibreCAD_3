#ifndef TEXT_ENTITY_H
#define TEXT_ENTITY_H

#include "gl_entity.h"

namespace lc
{
  namespace viewer
  {
    namespace opengl
    {
      class Text_Entity : public GL_Entity
      {
      private:   
        std::string _text;
        Shader* _shader;                       //Shader to be used
        glm::mat4 _model;                      // model matrix
        bool _no_magnify;
        GL_Font* _font=NULL;
  
      public:
        Text_Entity();
        ~Text_Entity();
  
        void setType(Shaders_book& shaders) override;
        void setModelMatrix(glm::mat4 model) override;
  
        void setFont(Font_Book& fonts,const std::string& style) override;
        void addTextData(glm::vec4 pos, std::string textval , float font_size, bool retain) override;

        void freeGPU() override;
        void draw(glm::mat4 proj,glm::mat4 projB,glm::mat4 view) override;

        //----------------No need----------------------------
        void loadVertexData(float* vertices,int size,std::vector<int> &jumps) override;
        void bind() override;
        void unbind() override;
        void setFillMode(bool fill) override;
        void setLineWidth(float width) override;
        void setDashes(std::vector<float> &dashes, int num_dashes,float sum_dashes) override;
        void setColor(float R,float G,float B,float A) override;
        void addLinearGradient(float x0,float y0,float x1,float y1) override;
        void addGradientColorPoint(float R,float G,float B,float A) override;
        void applyGradient(float* vertices,int size) override;
  
      };
    }
  }
}

#endif // TEXT_ENTITY_H
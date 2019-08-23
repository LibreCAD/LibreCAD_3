#ifndef SHAPE_ENTITY_H
#define SHAPE_ENTITY_H

#include "gl_entity.h"

namespace lc
{
  namespace viewer
  {
    namespace opengl
    {
      class Shape_Entity : public GL_Entity
      {
      private:   
        VertexArray _vao;                       //GPU Buffers Objects
        VertexBuffer _vbo;

        std::vector<int> _jumps;               //vector to store jumps

        glm::mat4 _model;                      // model matrix

        GLenum _render_mode;                   //mode for render
        GLenum _fill_mode;                     //mode for filling

        float _linewidth;                      //linewidth
  
        std::vector<float> _dashes;             //to store dash gap
        int _dashes_size=0;
        float _dashes_sum;                     //total sum of dash-gap
  
        Shader* _shader;                 //Shader to be used

        Entity_Type _type;                     // Type of entity ( basic/filled/thick/pattern) 

      public:
  
        Shape_Entity();
        ~Shape_Entity();
  
        void loadVertexData(float* vertices,int size,std::vector<int> &jumps) override;
  
        void bind() override;
        void unbind() override;
  
        void setType(Shaders_book& shaders) override;
        void setModelMatrix(glm::mat4 model) override;
  
        void setFillMode(bool fill) override;
        void setLineWidth(float width) override;
        void setDashes(std::vector<float> &dashes, int num_dashes,float sum_dashes) override;
        void setColor(float R,float G,float B,float A) override;

        void addLinearGradient(float x0,float y0,float x1,float y1) override;
        void addGradientColorPoint(float R,float G,float B,float A) override;
        void applyGradient(float* vertices,int size) override;
  
        void setFont(Font_Book& fonts,const std::string& style) override;
        void addTextData(glm::vec4 pos, std::string text_val , float font_size, bool retain) override;

        void freeGPU() override;

        void draw(glm::mat4 proj,glm::mat4 projB,glm::mat4 view) override;
      };
    }
  }
}

#endif // SHAPE_ENTITY_H
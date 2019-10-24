#ifndef GRADIENT_ENTITY_H
#define GRADIENT_ENTITY_H

#include "gl_entity.h"


namespace lc
{
  namespace viewer
  {
    namespace opengl
    {
      struct Linear_Pattern
      {
        glm::vec2 begin;
        glm::vec2 end;
        std::vector < glm::vec4 > color_points;
      };

      class Gradient_Entity : public GL_Entity
      {
      private:   
        VertexArray _vao;
        VertexBuffer _vbo;                       // GPU Buffer Objects (vertex data) 
          
        glm::mat4 _model;                      // model matrix

        GLenum _render_mode;                   //mode for render
        GLenum _fill_mode;                     //mode for filling
        float _linewidth;                      //linewidth
  
        std::vector<int> _jumps;
  
        Linear_Pattern* _pattern;
        std::vector<float> _color_vertex_data;
        Shader* _gradient_shader;
  
      public:
        Gradient_Entity();
        ~Gradient_Entity();
        
        void loadVertexData(float* vertices,int size,std::vector<int> &jumps) override;
        void bind() override;
        void unbind() override;
        void setType(Shaders_book& shaders) override;
        void setColor(float R,float G,float B,float A) override;
        void addLinearGradient(float x0,float y0,float x1,float y1) override;
        void addGradientColorPoint(float R,float G,float B,float A) override;
        void applyGradient(float* vertices,int size) override;
        void freeGPU() override;
        void draw(glm::mat4 proj,glm::mat4 projB,glm::mat4 view) override;

        //--------No need--------------------------------------
        void setModelMatrix(glm::mat4 model) override;
        void setFillMode(bool fill) override;
        void setLineWidth(float width) override;
        void setDashes(std::vector<float> &dashes, int num_dashes,float sum_dashes) override;
        void setFont(Font_Book& fonts,const std::string& style) override;
        void addTextData(glm::vec4 pos, std::string text_val , float font_size, bool retain) override;
      };
    }
  }
}

#endif // GRADIENT_ENTITY_H
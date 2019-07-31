#ifndef GRADIENT_ENTITY_H
#define GRADIENT_ENTITY_H
#include <GL/glew.h>  
#include <GL/gl.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include "vertexbuffer.h"
#include "indexbuffer.h"
#include "vertexarray.h"
#include "vertexbufferlayout.h"
#include "shader.h"
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
	VertexArray VAO;
	VertexBuffer VBO;                       // GPU Buffer Objects (vertex data)
                    

	glm::mat4 _model;                      // model matrix

	GLenum _render_mode;                   //mode for render
    GLenum _fill_mode;                     //mode for filling
    float _linewidth;                      //linewidth
    
    std::vector<int> _jumps;
    
    Linear_Pattern* pattern;
    std::vector<float> color_vertex_data;
    Shader* _gradient_shader;
public:
	Gradient_Entity();
	~Gradient_Entity();
	void LoadVertexData(float* vertices,int size,std::vector<int> &jumps);
	
	void Bind();
	void UnBind();
    
    void SetType(Shaders_book& shaders);
	void SetModelMatrix(glm::mat4 model);
	
	void SetFillMode(bool fill);
	void SetLineWidth(float width);
	void SetDashes(const double* dashes, const int num_dashes);
	void SetColor(float R,float G,float B,float A);

	void AddLinearGradient(float x0,float y0,float x1,float y1);
	void AddGradientColorPoint(float R,float G,float B,float A);
	void ApplyGradient(float* vertices,int size);

	void FreeGPU();

	void Draw(glm::mat4 proj,glm::mat4 projB,glm::mat4 view);
};



    }
  }

}

#endif // SHAPE_ENTITY_H
#ifndef SHAPE_ENTITY_H
#define SHAPE_ENTITY_H
#include <GL/glew.h>  
#include <GL/gl.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

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


class Shape_Entity : public GL_Entity
{
private:   
	VertexArray VAO;
	VertexBuffer VBO;
	
	glm::mat4 _model;                      // model matrix

	GLenum _render_mode;                   //mode for render
    GLenum _fill_mode;                     //mode for filling
    float _linewidth;                      //linewidth

    std::vector<int> _jumps;
    
    Shader* _basic_shader;
public:
	Shape_Entity();
	~Shape_Entity();
	void LoadData(float* vertices,int size,std::vector<int> &jumps);
	void ClearData();
    
	void Bind();
	void UnBind();
    
    void SetShader(Shader* shader);
	void SetModelMatrix(glm::mat4 model);
	void SetRenderMode(GLenum rendermode);
	void SetFillMode(GLenum fillmode);
	void SetLineWidth(float width);
	void SetColor(float R,float G,float B,float A);

    void AddLinearGradient(float x0,float y0,float x1,float y1);
	void AddGradientColorPoint(float R,float G,float B,float A);
	void ApplyGradient(float* vertices,int size);

	
	void FreeGPU();

	void Draw(glm::mat4 proj,glm::mat4 view);
};



    }
  }

}

#endif // SHAPE_ENTITY_H
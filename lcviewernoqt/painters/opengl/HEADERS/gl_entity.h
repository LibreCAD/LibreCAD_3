#ifndef GL_ENTITY_H
#define GL_ENTITY_H
#include <GL/glew.h>  
#include <GL/gl.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "vertexbuffer.h"
#include "indexbuffer.h"
#include "vertexarray.h"
#include "vertexbufferlayout.h"


namespace lc
{
	namespace viewer
	{
		namespace opengl
		{


class GL_Entity
{
private:   
	VertexArray VAO;
	VertexBuffer VBO;
	IndexBuffer IBO;                       // GPU Buffer Objects (vertex data)

	glm::mat4 _model;                      // model matrix

	GLenum _render_mode;                   //mode for render
    GLenum _fill_mode;                     //mode for filling
    

public:
	GL_Entity();
	~GL_Entity();
	void LoadData(float* vertices,int size,unsigned int* indices,int count);
	void ClearData();
    
    unsigned int GetIndices();
	void Bind();
	void UnBind();

	void SetModelMatrix(glm::mat4 model);
	void SetRenderMode(GLenum rendermode);
	void SetFillMode(GLenum fillmode);

	glm::mat4 GetModelMatrix();
	GLenum GetRenderMode();
	GLenum GetFillMode();

	void Delete();
};



    }
  }

}

#endif // GL_ENTITY_H
#ifndef GL_ENTITY_H
#define GL_ENTITY_H
#include <GL/glew.h>  
#include <GL/gl.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


#include "shader.h"


namespace lc
{
	namespace viewer
	{
		namespace opengl
		{


class GL_Entity
{

public:
	virtual ~GL_Entity() = default;
	virtual void LoadData(float* vertices,int size,unsigned int* indices,int count) = 0;
	virtual void ClearData() = 0;
    
	virtual void Bind() = 0;
	virtual void UnBind() = 0;
    
    virtual void SetShader(Shader* shader) = 0;
	virtual void SetModelMatrix(glm::mat4 model) = 0;
	virtual void SetRenderMode(GLenum rendermode) = 0;
	virtual void SetFillMode(GLenum fillmode) = 0;
	virtual void SetLineWidth(float width) = 0;
	virtual void SetColor(float R,float G,float B,float A) = 0;

	
	virtual void Delete() = 0;

	virtual void Draw(glm::mat4 proj,glm::mat4 view) = 0;
};



    }
  }

}

#endif // GL_ENTITY_H
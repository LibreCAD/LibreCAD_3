#ifndef GL_ENTITY_H
#define GL_ENTITY_H
#include <GL/glew.h>  
#include <GL/gl.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>

#include "shader.h"


namespace lc
{
	namespace viewer
	{
		namespace opengl
		{

enum Entity_Type{
        BASIC=0,FILL=1,THICK=2,PATTERN=3,GRADIENT=4,TEXT=5
    };

class GL_Entity
{

public:
	virtual ~GL_Entity() = default;
	virtual void LoadVertexData(float* vertices,int size,std::vector<int> &jumps) = 0;
	
	virtual void Bind() = 0;
	virtual void UnBind() = 0;
    
    virtual void SetType(Shaders_book& shaders) = 0;
	virtual void SetModelMatrix(glm::mat4 model) = 0;
	
	virtual void SetFillMode(bool fill) = 0;
	virtual void SetLineWidth(float width) = 0;
	virtual void SetDashes(const double* dashes, const int num_dashes) = 0;
	virtual void SetColor(float R,float G,float B,float A) = 0;

    virtual void AddLinearGradient(float x0,float y0,float x1,float y1) = 0;
	virtual void AddGradientColorPoint(float R,float G,float B,float A) = 0;
	virtual void ApplyGradient(float* vertices,int size) = 0;

	
	virtual void FreeGPU() = 0;

	virtual void Draw(glm::mat4 proj,glm::mat4 projB,glm::mat4 view) = 0;
};



    }
  }

}

#endif // GL_ENTITY_H
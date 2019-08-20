#ifndef GL_ENTITY_H
#define GL_ENTITY_H
#include <GL/glew.h>  
#include <GL/gl.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cmath>
#include <vector>

#include "vertexbuffer.h"
#include "indexbuffer.h"
#include "vertexarray.h"
#include "vertexbufferlayout.h"
#include "shader.h"
#include "gl_font.h"
#include "font_book.h"


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
	virtual void loadVertexData(float* vertices,int size,std::vector<int> &jumps) = 0;
	
	virtual void bind() = 0;
	virtual void unbind() = 0;
    
    virtual void setType(Shaders_book& shaders) = 0;
	virtual void setModelMatrix(glm::mat4 model) = 0;
	
	virtual void setFillMode(bool fill) = 0;
	virtual void setLineWidth(float width) = 0;
	virtual void setDashes(std::vector<float> &dashes, int num_dashes,float sum_dashes) = 0;
	virtual void setColor(float R,float G,float B,float A) = 0;

    virtual void addLinearGradient(float x0,float y0,float x1,float y1) = 0;
	virtual void addGradientColorPoint(float R,float G,float B,float A) = 0;
	virtual void applyGradient(float* vertices,int size) = 0;
    

    virtual void setFont(Font_Book& fonts,const std::string& style) = 0;
	virtual void addTextData(glm::vec4 pos, std::string text_val , float font_size, bool retain) = 0;

	
	virtual void freeGPU() = 0;

	virtual void draw(glm::mat4 proj,glm::mat4 projB,glm::mat4 view) = 0;
};



    }
  }

}

#endif // GL_ENTITY_H
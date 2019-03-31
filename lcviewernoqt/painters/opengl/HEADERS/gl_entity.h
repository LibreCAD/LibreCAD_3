#ifndef GL_ENTITY_H
#define GL_ENTITY_H
#include <GL/glew.h>  
#include <GL/gl.h>

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
	IndexBuffer IBO;
public:
	GL_Entity();
	~GL_Entity();
	void LoadData(float* vertices,int size,unsigned int* indices,int count);
	void ClearData();
    
    unsigned int GetIndices();
	void Bind();
	void UnBind();

	void Delete();
};



    }
  }

}

#endif // GL_ENTITY_H
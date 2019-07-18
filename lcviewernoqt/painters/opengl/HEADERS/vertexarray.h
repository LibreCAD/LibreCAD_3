#ifndef VERTEXARRAY_H
#define VERTEXARRAY_H
#define GL_GLEXT_PROTOTYPES

#include <GL/glew.h>  
#include <GL/gl.h>
#include "vertexbuffer.h"
#include "vertexbufferlayout.h"


namespace lc
{
	namespace viewer
	{
		namespace opengl
		{

class VertexArray
{
private:
    unsigned int m_RendererID;
public:		
	VertexArray();
	~VertexArray();

    void Gen();
	void Bind() const;
	void UnBind() const;
    void FreeGPU() const;
	void AddBuffer(const VertexBuffer& vb,const VertexBufferLayout& layout);

};


}

}

}

#endif // VERTEXARRAY_H
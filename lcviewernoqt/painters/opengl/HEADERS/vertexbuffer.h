#ifndef VERTEXBUFFER_H
#define VERTEXBUFFER_H
#define GL_GLEXT_PROTOTYPES

#include <GL/glew.h>  
#include <GL/gl.h>


namespace lc
{
	namespace viewer
	{
		namespace opengl
		{

class VertexBuffer
{
private:
	unsigned int m_RendererID;
public:
	VertexBuffer();
	~VertexBuffer();

    void Gen(const void* data,unsigned int size);
	void Bind() const;
	void UnBind() const;
	void FreeGPU() const;

};

}

}

}

#endif // VERTEXBUFFER_H
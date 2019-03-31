#ifndef INDEXBUFFER_H
#define INDEXBUFFER_H
#define GL_GLEXT_PROTOTYPES

#include <GL/glew.h>  
#include <GL/gl.h>

namespace lc
{
	namespace viewer
	{
		namespace opengl
		{

class IndexBuffer
{
private:
	unsigned int m_RendererID;
	unsigned int m_Count;
public:
	IndexBuffer();
	~IndexBuffer();

    void Gen(const unsigned int* data,unsigned int count);
	void Bind() const;
	void UnBind() const;
	inline unsigned int GetCount() const { return m_Count; }

};

  }

 }

} 

#endif // INDEXBUFFER_H
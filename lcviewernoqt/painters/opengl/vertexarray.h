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
    unsigned int _va_id;
public:
    VertexArray();
    ~VertexArray();

    void gen();
    void bind() const;
    void unbind() const;
    void freeGPU() const;
    void addBuffer(const VertexBuffer& vb,const VertexBufferLayout& layout);
};
}
}
}

#endif // VERTEXARRAY_H
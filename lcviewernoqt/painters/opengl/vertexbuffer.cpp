#include "vertexbuffer.h"
using namespace lc::viewer::opengl;

VertexBuffer::VertexBuffer()
{
}

VertexBuffer::~VertexBuffer()
{
}

void VertexBuffer::gen(const void* data,unsigned int size)
{
    glGenBuffers(1,&_vb_id);
    glBindBuffer(GL_ARRAY_BUFFER,_vb_id);
    glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
}

void VertexBuffer::bind() const
{
    glBindBuffer(GL_ARRAY_BUFFER,_vb_id);
}

void VertexBuffer::unbind() const
{
    glBindBuffer(GL_ARRAY_BUFFER,0);
}

void VertexBuffer::freeGPU() const
{
    unbind();
    glDeleteBuffers(1,&_vb_id);
}
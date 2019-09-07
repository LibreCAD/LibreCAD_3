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
        unsigned int _vb_id;
      public:
        VertexBuffer();
        ~VertexBuffer();

        void gen(const void* data,unsigned int size);
        void bind() const;
        void unbind() const;
        void freeGPU() const;
      };
    }
  }
}

#endif // VERTEXBUFFER_H
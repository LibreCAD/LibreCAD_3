#ifndef VERTEXBUFFERLAYOUT_H
#define VERTEXBUFFERLAYOUT_H
#define GL_GLEXT_PROTOTYPES

#include <GL/glew.h>  
#include <GL/gl.h>
#include <vector>

namespace lc
{
	namespace viewer
	{
		namespace opengl
		{
			struct VertexBufferElement
			{
				unsigned int type;
				unsigned int count;
				unsigned char normalized;

				static unsigned int getSizeOfType(unsigned int type)
				{
					switch(type)
					{
						case GL_FLOAT : return 4;
						case GL_UNSIGNED_INT: return 4;
						case GL_UNSIGNED_BYTE: return 1;
					}

						return 0;
				}
			};

			class VertexBufferLayout
			{
			private:
				unsigned int _stride;
				std::vector<VertexBufferElement> _elements;
			public:
				VertexBufferLayout()
				{
					_stride=0;
				}

				template<typename T>
				void push(unsigned int count)
				{
					//static_assert(false);
				}

				inline const std::vector<VertexBufferElement> getElements() const { return _elements; }
				inline unsigned int getStride() const { return _stride; }

			};

			template<>
			inline void VertexBufferLayout::push<float>(unsigned int count)
			{
				_elements.push_back({GL_FLOAT,count,GL_FALSE});
				_stride+=VertexBufferElement::getSizeOfType(GL_FLOAT) * count;
			}

			template<>
			inline void VertexBufferLayout::push<unsigned int>(unsigned int count)
			{
				_elements.push_back({GL_UNSIGNED_INT,count,GL_FALSE});
				_stride+=VertexBufferElement::getSizeOfType(GL_UNSIGNED_INT) * count;
			}

			template<>
			inline void VertexBufferLayout::push<unsigned char>(unsigned int count)
			{
				_elements.push_back({GL_UNSIGNED_BYTE,count,GL_TRUE});
				_stride+=VertexBufferElement::getSizeOfType(GL_UNSIGNED_BYTE)* count;
			}
		}
	}
}
#endif // VERTEXBUFFERLAYOUT_H
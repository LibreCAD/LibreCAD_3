#ifndef TEXT_ENTITY_H
#define TEXT_ENTITY_H
#include <GL/glew.h>  
#include <GL/gl.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"
#include "gl_entity.h"
#include "gl_font.h"
#include "font_book.h"


namespace lc
{
	namespace viewer
	{
		namespace opengl
		{


class Text_Entity : public GL_Entity
{

private:   
	
    float px=0.0f,py=0.0f;
    std::string _text;
    Shader* _shader;                       //Shader to be used
    glm::mat4 _model;                      // model matrix
    bool _no_magnify;
    GL_Font* _font=NULL;
    
public:
	
	Text_Entity();
	~Text_Entity();
	void LoadVertexData(float* vertices,int size,std::vector<int> &jumps);
	
	void Bind();
	void UnBind();
    
    void SetType(Shaders_book& shaders);
	void SetModelMatrix(glm::mat4 model);
	
	void SetFillMode(bool fill);
	void SetLineWidth(float width);
	void SetDashes(std::vector<float> &dashes, int num_dashes,float sum_dashes);
	void SetColor(float R,float G,float B,float A);
    

    void AddLinearGradient(float x0,float y0,float x1,float y1);
	void AddGradientColorPoint(float R,float G,float B,float A);
	void ApplyGradient(float* vertices,int size);
    

    void SetFont(Font_Book& fonts,const std::string& style);
	void AddTextData(glm::vec4 pos, const char* text_val , float font_size, bool retain);

	
	void FreeGPU();

	void Draw(glm::mat4 proj,glm::mat4 projB,glm::mat4 view);
};



    }
  }

}

#endif // TEXT_ENTITY_H
#ifndef TEXT_ENTITY_H
#define TEXT_ENTITY_H

#include "gl_entity.h"

namespace lc
{
	namespace viewer
	{
		namespace opengl
		{


class Text_Entity : public GL_Entity
{

private:   
	
    std::string _text;
    Shader* _shader;                       //Shader to be used
    glm::mat4 _model;                      // model matrix
    bool _no_magnify;
    GL_Font* _font=NULL;
    
public:
	
	Text_Entity();
	~Text_Entity();
	void loadVertexData(float* vertices,int size,std::vector<int> &jumps);
	
	void bind();
	void unbind();
    
    void setType(Shaders_book& shaders);
	void setModelMatrix(glm::mat4 model);
	
	void setFillMode(bool fill);
	void setLineWidth(float width);
	void setDashes(std::vector<float> &dashes, int num_dashes,float sum_dashes);
	void setColor(float R,float G,float B,float A);
    

    void addLinearGradient(float x0,float y0,float x1,float y1);
	void addGradientColorPoint(float R,float G,float B,float A);
	void applyGradient(float* vertices,int size);
    

    void setFont(Font_Book& fonts,const std::string& style);
	void addTextData(glm::vec4 pos, std::string textval , float font_size, bool retain);

	
	void freeGPU();

	void draw(glm::mat4 proj,glm::mat4 projB,glm::mat4 view);
};



    }
  }

}

#endif // TEXT_ENTITY_H
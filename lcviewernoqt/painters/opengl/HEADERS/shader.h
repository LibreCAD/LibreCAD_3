#ifndef SHADER_H
#define SHADER_H
#define GL_GLEXT_PROTOTYPES

#include <GL/glew.h> 
#include <GL/glut.h>  
#include <GL/gl.h>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <string>


#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace lc
{
	namespace viewer
	{
		namespace opengl
		{


struct ShaderProgramSource
{
	std::string VertexSource;
	std::string GeometrySource;
	std::string FragmentSource;
};

class Shader
{

private:
	std::string m_FilePath;
	unsigned int m_RendererID;
	bool has_geometry=false;
	
public:
	Shader();
	~Shader();
    void Gen(const std::string& filepath);
	void Bind() const;
	void UnBind() const;
	unsigned int GetID() const;

	//set uniforms
	void SetUniform4f(const std::string& name,float v0,float v1,float v2,float v3);
    void SetUniform1i(const std::string& name,int value);
    void SetUniformMat4f(const std::string& name,const glm::mat4& matrix);
    void SetUniform1f(const std::string& name, float value);
    void SetUniform1fv(const std::string& name,int count,const float *value);
    void SetUniform2f(const std::string& name,float v0,float v1);

private:
	int GetUniformLocation(const std::string& name);
    
    unsigned int compileShaders(std::string shader, GLenum type);
    ShaderProgramSource ParseShader(const std::string& filepath);
    unsigned int linkProgram(unsigned int vertexShaderID,unsigned int geometryShaderID, unsigned int fragmentShaderID);
    unsigned int CreateShaderProgram(float R,float G,float B,float A);
};

struct Shaders_book
{
   Shader* basic_shader      = NULL;
   Shader* gradient_shader   = NULL;
   Shader* thickline_shader  = NULL;
   Shader* linepattern_shader= NULL;
   Shader* text_shader       = NULL;

};

   }

 }

}

#endif // SHADER_H
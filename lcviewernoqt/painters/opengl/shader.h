#ifndef SHADER_H
#define SHADER_H
#define GL_GLEXT_PROTOTYPES

#include <GL/glew.h>
#include <GL/gl.h>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <string>


#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <functional>

namespace lc
{
namespace viewer
{
namespace opengl
{
struct ShaderProgramSource
{
    std::string vertexSource;
    std::string geometrySource;
    std::string fragmentSource;
};

class Shader
{
private:
    std::string _file_path;
    unsigned int _shader_id;
    bool _has_geometry=false;

public:
    Shader();
    ~Shader();
    void gen(const std::string& filepath, std::function<void (GLuint)> variableBinder=[](GLuint programId) {});
    void bind() const;
    void unbind() const;
    unsigned int getID() const;

    //set uniforms
    void setUniform4f(const std::string& name,float v0,float v1,float v2,float v3);
    void setUniform1i(const std::string& name,int value);
    void setUniformMat4f(const std::string& name,const glm::mat4& matrix);
    void setUniform1f(const std::string& name, float value);
    void setUniform1fv(const std::string& name,int count,const float *value);
    void setUniform2f(const std::string& name,float v0,float v1);

private:
    int getUniformLocation(const std::string& name);
    unsigned int compileShaders(std::string shader, GLenum type);
    ShaderProgramSource parseShader(const std::string& filepath);
    unsigned int linkProgram(
        unsigned int vertexShaderID,
        unsigned int geometryShaderID,
        unsigned int fragmentShaderID,
        std::function<void (GLuint)> variableBinder
    );
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
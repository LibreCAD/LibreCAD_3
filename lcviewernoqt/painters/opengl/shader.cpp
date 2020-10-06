#include "shader.h"
#include <cad/logger/logger.h>
using namespace lc::viewer::opengl;

Shader::Shader()
{
}

Shader::~Shader()
{
    glDeleteProgram(_shader_id);
}

void Shader::gen(const std::string& filepath, std::function<void (GLuint)> variableBinder)
{
    _shader_id=0;
    _file_path=filepath;

    ShaderProgramSource source=parseShader(_file_path);

    unsigned int vertexshaderID=0;
    unsigned int geometryshaderID=0;
    unsigned int fragmentshaderID=0;


    vertexshaderID=compileShaders(source.vertexSource,GL_VERTEX_SHADER);

    if(_has_geometry)
        geometryshaderID=compileShaders(source.geometrySource,GL_GEOMETRY_SHADER);

    fragmentshaderID=compileShaders(source.fragmentSource,GL_FRAGMENT_SHADER);

    _shader_id= linkProgram(vertexshaderID,geometryshaderID,fragmentshaderID, std::move(variableBinder));

    bind();
}

unsigned int Shader::getID() const
{
    return _shader_id;
}

ShaderProgramSource Shader:: parseShader(const std::string& filepath)
{
    std::ifstream stream(filepath);

    enum ShaderType {
        NONE=-1, VERTEX =0,GEOMETRY=1,FRAGMENT=2
    };

    std::string line;
    std::stringstream ss[3];
    ShaderType type=ShaderType::NONE;

    while(getline(stream,line))
    {
        if(line.find("#shader")!= std::string::npos)
        {
            if(line.find("vertex")!=std::string::npos)
                type=ShaderType::VERTEX;
            else if(line.find("geometry")!=std::string::npos)
            {
                type=ShaderType::GEOMETRY;
                _has_geometry=true;
            }
            else if(line.find("fragment")!=std::string::npos)
                type=ShaderType::FRAGMENT;
        }
        else
        {
            ss[(int)type]<< line <<'\n';
        }
    }

    return { ss[0].str(),ss[1].str(),ss[2].str() };
}

unsigned int Shader:: compileShaders(std::string shader, GLenum type)
{
    const char* shaderCode= shader.c_str();
    GLuint shaderID= glCreateShader(type);

    if(shaderID==0)
    {
        LOG_WARNING << "Shader not created";
        return 0;
    }

    glShaderSource(shaderID,1,&shaderCode,NULL);
    glCompileShader(shaderID);

    GLint compileStatus;

    glGetShaderiv(shaderID,GL_COMPILE_STATUS,&compileStatus);

    if(!compileStatus)
    {
        int length;
        glGetShaderiv(shaderID,GL_INFO_LOG_LENGTH,&length);
        char* error_message=new char[length];

        glGetShaderInfoLog(shaderID,length,&length,error_message);
        LOG_WARNING << "Cannot compile Shader" << error_message;

        delete[] error_message;
        glDeleteShader(shaderID);
        return 0;
    }

    return shaderID;
}


unsigned int Shader::linkProgram(unsigned int vertexShaderID,
                                 unsigned int geometryShaderID,
                                 unsigned int fragmentShaderID,
                                 std::function<void (GLuint)> variableBinder) {
    GLuint programID= glCreateProgram();

    if(programID==0)
    {
        LOG_WARNING << "Program not created";
        return 0;
    }

    if(vertexShaderID!=0)
        glAttachShader(programID,vertexShaderID);

    if(geometryShaderID!=0 && _has_geometry)
        glAttachShader(programID,geometryShaderID);

    if(fragmentShaderID!=0)
        glAttachShader(programID,fragmentShaderID);

    variableBinder(programID);

    glLinkProgram(programID);

    GLint linkstatus;

    glGetProgramiv(programID,GL_LINK_STATUS,&linkstatus);

    if(!linkstatus)
    {
        LOG_WARNING << "Error linking program";
        glDetachShader(programID,vertexShaderID);
        glDetachShader(programID,geometryShaderID);
        glDetachShader(programID,fragmentShaderID);
        glDeleteProgram(programID);

        return 0;
    }
    return programID;
}


void Shader::bind() const
{
    glUseProgram(_shader_id);
}

void Shader::unbind() const
{
    glUseProgram(0);
}

void Shader::setUniform4f(const std::string& name,float v0,float v1,float v2,float v3)
{
    glUniform4f( getUniformLocation(name), v0,v1,v2,v3 );
}

void Shader::setUniform2f(const std::string& name,float v0,float v1)
{
    glUniform2f( getUniformLocation(name), v0,v1);
}

void Shader::setUniform1i(const std::string& name,int value)
{
    glUniform1i( getUniformLocation(name), value );
}

void Shader::setUniformMat4f(const std::string& name,const glm::mat4& matrix)
{
    glUniformMatrix4fv( getUniformLocation(name),1,GL_FALSE,&matrix[0][0]);
}

void Shader::setUniform1f(const std::string& name, float value)
{
    glUniform1f( getUniformLocation(name), value );
}

void Shader::setUniform1fv(const std::string& name,int count, const float *value )
{
    glUniform1fv( getUniformLocation(name), count, value );
}

int Shader:: getUniformLocation(const std::string& name)
{
    int location=glGetUniformLocation(_shader_id,name.c_str());
    if(location==-1)
    {
        return -1;
    }
    return location;
}
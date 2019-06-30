#include "shader.h"
#include "debug.h"
#include <QtDebug>
using namespace lc::viewer::opengl;

Shader::Shader()
{
	DebugMessage("Constructor Shader");
}

Shader::~Shader()
{
	DebugMessage("Destructor Shader");
   glDeleteProgram(m_RendererID);
}

void Shader::Gen(const std::string& filepath)
{
	m_RendererID=0;
	m_FilePath=filepath;

	ShaderProgramSource source=ParseShader(m_FilePath);

	unsigned int vertexshaderID=compileShaders(source.VertexSource,GL_VERTEX_SHADER);
	unsigned int fragmentshaderID=compileShaders(source.FragmentSource,GL_FRAGMENT_SHADER);
	  
	m_RendererID= linkProgram(vertexshaderID,fragmentshaderID);

    Bind();
}

unsigned int Shader::GetID() const
{
	return m_RendererID;
}


ShaderProgramSource Shader:: ParseShader(const std::string& filepath)
{
	std::ifstream stream(filepath);

    enum ShaderType{
        NONE=-1, VERTEX =0,FRAGMENT=1
    };

	std::string line;
    std::stringstream ss[2];
    ShaderType type=ShaderType::NONE;

	while(getline(stream,line))
	{
		if(line.find("#shader")!= std::string::npos)
		{
			if(line.find("vertex")!=std::string::npos)
				type=ShaderType::VERTEX;
			else if(line.find("fragment")!=std::string::npos)
	           type=ShaderType::FRAGMENT;
		}

		else
		{
			ss[(int)type]<< line <<'\n';
		}
	}
   
   qDebug("parse shader===> %s \n %s",ss[0].str().c_str(),ss[1].str().c_str());
   
	return { ss[0].str() ,ss[1].str() };
}

unsigned int Shader:: compileShaders(std::string shader, GLenum type)
{
	const char* shaderCode= shader.c_str();
   GLuint shaderID= glCreateShader(type);

	if(shaderID==0)
	{
		std::cout<<type<<" not created"<<std::endl;
		return 0;
	}

    qDebug("compile shader===>  %s \n *** %d",shaderCode,shaderID);
	glShaderSource(shaderID,1,&shaderCode,NULL);
	glCompileShader(shaderID);

    //--------------
	GLint compileStatus;

	glGetShaderiv(shaderID,GL_COMPILE_STATUS,&compileStatus);

	if(!compileStatus)
	{
		int length;
		glGetShaderiv(shaderID,GL_INFO_LOG_LENGTH,&length);
		char* error_message=new char[length];

		glGetShaderInfoLog(shaderID,length,&length,error_message);

		std::cout<<"Cannot compile"<<type<<std::endl;
		std::cout<<error_message<<std::endl;

		delete[] error_message;
		glDeleteShader(shaderID);
		return 0;
	}
    //--------------
	return shaderID;
}


unsigned int Shader:: linkProgram(GLuint vertexShaderID, GLuint fragmentShaderID)
{
	GLuint programID= glCreateProgram();

	if(programID==0)
	{
		std::cout<<"Program not created"<<std::endl;
		return 0;
	}

	glAttachShader(programID,vertexShaderID);
	glAttachShader(programID,fragmentShaderID);

	glLinkProgram(programID);

    //--------------------
	GLint linkstatus;

	glGetProgramiv(programID,GL_LINK_STATUS,&linkstatus);

	if(!linkstatus)
	{
		std::cout<<"Error linking program"<<std::endl;
		glDetachShader(programID,vertexShaderID);
		glDetachShader(programID,fragmentShaderID);
		glDeleteProgram(programID);
         
        return 0;
	}
	//--------------------

	return programID;

}


void Shader::Bind() const
{
   glUseProgram(m_RendererID);
}

void Shader::UnBind() const
{
   glUseProgram(0);
}

void Shader::SetUniform4f(const std::string& name,float v0,float v1,float v2,float v3)
{
   glUniform4f( GetUniformLocation(name) , v0,v1,v2,v3 );
}

void Shader::SetUniformMat4f(const std::string& name,const glm::mat4& matrix)
{
	glUniformMatrix4fv( GetUniformLocation(name) ,1,GL_FALSE,&matrix[0][0]);
}

unsigned int Shader:: GetUniformLocation(const std::string& name)
{
   unsigned int location=glGetUniformLocation(m_RendererID,name.c_str());
   if(location==-1)
   	std::cout<<"not exist"<<std::endl;

   return location;
}
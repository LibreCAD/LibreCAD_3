#include "renderer.h"
#include "debug.h"
#include <QtDebug>
using namespace lc::viewer::opengl;

void GLClearError()
{
	while(glGetError()!=GL_NO_ERROR);
}

bool GLLogCall(const char* function,const char* file,int line)
{
	while(GLenum error =glGetError())
	{
		std::cout<<"[ OpenGL Error ] ("<< error <<")"<<function<<" "<< file<<" : "<<line<<std::endl;
	    return false;
	}
	return true;
}

//---------------------------------------------------
Renderer::Renderer()
{
	DebugMessage("Constructor Renderer");
	
	fill_mode=GL_LINE;
    render_mode=GL_LINES;

    view=glm::mat4(1.0f);
    scaling=glm::mat4(1.0f);
    model=scaling;

    Update_MVP();

   
}

Renderer::~Renderer()
{
   DebugMessage("Destructor Renderer");
}

//-------------------------------------------------


void Renderer::Update_projection(float l,float r,float b,float t)
{
    proj=glm::ortho(l,r,b,t,-1.0f,1.0f);
    Update_MVP();
}

void Renderer::Update_view(float x,float y)
{
	view=translate(glm::mat4(1.0f),glm::vec3(x,y,0.0));
	Update_MVP();
}

void Renderer::Update_scaling(float scale_f)
{
	scaling=scale(glm::mat4(1.0f),glm::vec3(scale_f,scale_f,scale_f));	
	model=scaling;
	Update_MVP();
}

void Renderer::Update_model()
{
	model=scaling;  //TEMP
	//TODO: update model further with gl_entity own translate and rotate also
	// if no such : then model will remain = scaling only

	Update_MVP();
}

void Renderer::Update_MVP()
{
    mvp= proj * view * model;
}

void Renderer::Set_MVP()
{
	Update_model();

	SH.Bind();
    SH.SetUniformMat4f("u_MVP",mvp);
    SH.UnBind();
}

void Renderer::Device_To_User(double* x, double* y)
{
	glm::vec4 temp=glm::vec4(*x,*y,0,1);
	temp=glm::inverse(view*model) * temp;
	*x=temp.x;
	*y=temp.y;
}

void Renderer::User_To_Device(double* x, double* y)
{
	glm::vec4 temp=glm::vec4(*x,*y,0,1);
	temp=glm::mat4(view*model) * temp;
	*x=temp.x;
	*y=temp.y;
}

//------------------------------------------

bool Renderer::Find_GL_Entity(unsigned int id)
{
	//TODO: find the gl_entity in map

}

void Renderer::Use_GL_Entity(unsigned int id)
{
	//current_gl_entity= gl_entity corresponding to id
}

void Renderer::Add_New_GL_Entity()
{
	if(current_gl_entity!=NULL)
	current_gl_entity->Delete();

	current_gl_entity = new GL_Entity();
}

void Renderer::Add_Vertex(float x,float y,float z)
{
	vertices.push_back(x);
	vertices.push_back(y);
	vertices.push_back(z);

    unsigned int l=(unsigned int)(indices.size());
	indices.push_back((l));

}

void Renderer::Clear_Data()
{
	vertices.clear();
	indices.clear();
}

void Renderer::Add_Data_To_GL_Entity()
{
   current_gl_entity->LoadData(&vertices[0] , vertices.size()*sizeof(float) , 
   	                           &indices[0]  , indices.size() );

}

void Renderer::Select_Fill(GLenum fill)
{
	fill_mode=fill;
}

void Renderer::Select_Render_Mode(GLenum mode)
{
   render_mode=mode;
}

void Renderer::Select_Line_Width(float width)
{
	glLineWidth(width);
}

void Renderer::CreateShaderProgram()
{
	DebugMessage("generating Shader");
	//SH.Gen("opengl/RES/SHADERS/basic_shader.shader");
	//SH.Gen("/home/krixz/LC_3/lc3_dev_4/LibreCAD_3/lcviewernoqt/painters/opengl/RES/SHADERS/basic_shader.shader");
	SH.Gen("/home/krixz/LC_3/LC3_forked/LibreCAD_3/lcviewernoqt/painters/opengl/RES/SHADERS/basic_shader.shader");
	SH.UnBind();
}

void Renderer::Select_Color(float R,float G,float B,float A)
{
	SH.Bind();
	SH.SetUniform4f("u_Color",R,G,B,A);
    SH.UnBind();
}
    
void Renderer::Set_Default()
{
	fill_mode=GL_LINE;
    render_mode=GL_LINES;
    Select_Line_Width(1);
    Clear_Data();
}
    
void Renderer::Draw()
{
	glPolygonMode(GL_FRONT_AND_BACK, fill_mode);
    
   

    Set_MVP();
     SH.Bind();
    
    //TODO: check if entity already exist
    //if not the add new
      Add_New_GL_Entity();
      //and load data
      Add_Data_To_GL_Entity();

    //else
     //pick entity from map and use that  

    current_gl_entity->Bind();

    //finally draw
     glDrawElements(render_mode,
     	            current_gl_entity->GetIndices(),
     	            GL_UNSIGNED_INT,
     	            0);

     //again set things to default for next entity
     Set_Default();

}

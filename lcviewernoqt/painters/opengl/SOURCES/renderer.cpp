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

GLenum glCheckError_(const char *file, int line)
{
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR)
    {
        std::string error;
        switch (errorCode)
        {
            case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
            case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
            case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
            case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
            case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
            case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
            case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
        }
        std::cout << error << " | " << file << " (" << line << ")" << std::endl;
    }
    return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__) 


//---------------------------------------------------
Renderer::Renderer()
{
	DebugMessage("Constructor Renderer");
	
	fill_mode=GL_LINE;
    render_mode=GL_LINES;

    ctm=glm::mat4(1.0f);
    
    view=ctm;
    model=glm::mat4(1.0f);

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

void Renderer::Update_view()
{
	view=ctm;
	Update_MVP();
}

void Renderer::Update_model(glm::mat4 _model)
{	  
	//TODO: update model further with gl_entity own translate and rotate also
	// if no such : then model will remain = Identity only
    model=_model;

	Update_MVP();
}

void Renderer::Update_MVP()
{
    mvp= proj * view * model;
}

void Renderer::Set_MVP()
{
	Update_MVP();

	SH.Bind();
    SH.SetUniformMat4f("u_MVP",mvp);
    SH.UnBind();
}
//------------------------

void Renderer::Update_scale(float scale_f)
{
	ctm=glm::scale(ctm,glm::vec3(scale_f,scale_f,scale_f));	
	view=ctm;
	Update_MVP();
}

void Renderer::Update_translate(float x,float y)
{
	ctm=glm::translate(ctm,glm::vec3(x,y,0.0));
	view=ctm;
	Update_MVP();
}

void Renderer::Update_rotate(float angle)
{
	ctm=glm::rotate(ctm, angle, glm::vec3(0.0f ,0.0f ,1.0f) );
	view=ctm;
	qDebug("Renderer rotation update= %f",angle);
	Update_MVP();
}
void Renderer::Reset_Transformations()
{
    ctm=glm::mat4(1.0f);
    model=glm::mat4(1.0f);
    view=ctm;
    Update_MVP();
}

double Renderer::Get_Scale()
{
	return ctm[2][2];
}

double Renderer::Get_Translate_X()
{
	return ctm[3][0];
}

double Renderer::Get_Translate_Y()
{
	return ctm[3][1];
}    

//-----------------------

void Renderer::Device_To_User(double* x, double* y)
{
	glm::vec4 temp=glm::vec4(*x,*y,0,1);
	temp=glm::inverse(ctm) * temp;
	*x=temp.x;
	*y=temp.y;
}

void Renderer::Device_To_User_Distance(double* x, double* y)
{
	glm::vec4 temp_vec=glm::vec4(*x,*y,0,1);
	glm::mat4 temp_mat=ctm;
	temp_mat[3][0]=0;
	temp_mat[3][1]=0;
	temp_vec=glm::inverse( temp_mat ) * temp_vec;
	*x=temp_vec.x;
	*y=temp_vec.y;
}

void Renderer::User_To_Device(double* x, double* y)
{
	glm::vec4 temp=glm::vec4(*x,*y,0,1);
	temp=glm::mat4(ctm) * temp;
	*x=temp.x;
	*y=temp.y;
}

void Renderer::User_To_Device_Distance(double* x, double* y)
{
	glm::vec4 temp_vec=glm::vec4(*x,*y,0,1);
	glm::mat4 temp_mat=ctm;
	temp_mat[3][0]=0;
	temp_mat[3][1]=0;
	temp_vec=glm::inverse( temp_mat ) * temp_vec;
	*x=temp_vec.x;
	*y=temp_vec.y;
}

//------------------------------------------

void Renderer::Save()
{
	context_att current_context;
	current_context.ctm = this->ctm;
	context_stack.push(current_context);
}

void Renderer::Restore()
{
	context_att prev_context = context_stack.top();
	this->ctm = prev_context.ctm;
	context_stack.pop();
    view=ctm;
    model=glm::mat4(1.0f);
    Update_MVP();
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

     qDebug("+++++++++++++++++ RENDERER DIRECT DRAW +++++++++++++++++");

     //again set things to default for next entity
     Set_Default();

}

void Renderer::Render_Cached_Entity(GL_Entity* cached_entity)
{
   current_gl_entity->UnBind();
    
   Save();
	 Set_Default();
	 
   
   Select_Render_Mode( cached_entity->GetRenderMode() );
   Select_Fill( cached_entity->GetFillMode() );
   Update_model( cached_entity->GetModelMatrix() );

   glPolygonMode(GL_FRONT_AND_BACK, cached_entity->GetFillMode());

   Set_MVP();
   SH.Bind();

   cached_entity->Bind();
  
   qDebug("****************** RENDERER CACHED DRAW ******************");

   qDebug("Render cached entity draw--- %u render_mode=%d  fill_mode=%d",cached_entity,
                                                                         cached_entity->GetRenderMode(),
                                                                         cached_entity->GetFillMode());
  

    //finally draw
     glDrawElements(cached_entity->GetRenderMode(),
     	            cached_entity->GetIndices(),
     	            GL_UNSIGNED_INT,
     	            0);

    Restore();
   
}

void Renderer::Render_Cached_Pack(GL_Pack* pack)
{
   qDebug("---In RENDERER------ render pack %u",pack);

   int l=pack->Pack_Size();

   GL_Entity* gl_entity_in_pack;

   for(int i=0;i<l;i++)
   {
      gl_entity_in_pack=pack->Get_GL_Entity_At(i);
      qDebug("----To render entity -- %u",gl_entity_in_pack);
      Render_Cached_Entity(gl_entity_in_pack);
   }

}
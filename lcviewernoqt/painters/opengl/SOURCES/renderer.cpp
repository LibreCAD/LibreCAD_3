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
   
   ctm=glm::mat4(1.0f);   
    view=ctm;
      
}

Renderer::~Renderer()
{
   DebugMessage("Destructor Renderer");
}

void Renderer::CreateShaderProgram()
{
  DebugMessage("generating Shader");
  
  basic_shader.Gen("/home/krixz/LC_PURE/GSoC/LibreCAD_3/lcviewernoqt/painters/opengl/RES/SHADERS/basic_shader.shader");
  basic_shader.UnBind();

  gradient_shader.Gen("/home/krixz/LC_PURE/GSoC/LibreCAD_3/lcviewernoqt/painters/opengl/RES/SHADERS/color_vertex_shader.shader");
  gradient_shader.UnBind();


  CH_Ptr->Set_Shader_Ref(&basic_shader , &gradient_shader , NULL);
}

void Renderer::Set_Cacher_Ref(Cacher* ch)
{
   CH_Ptr=ch;
}

//-------------------------------------------------


void Renderer::Update_projection(float l,float r,float b,float t)
{
    proj=glm::ortho(l,r,b,t,-1.0f,1.0f);
}

void Renderer::Update_view()
{
	  view=ctm;
}

//------------------------

void Renderer::Update_scale(float scale_f)
{
	ctm=glm::scale(ctm,glm::vec3(scale_f,scale_f,scale_f));	
	view=ctm;
}

void Renderer::Update_translate(float x,float y)
{
	ctm=glm::translate(ctm,glm::vec3(x,y,0.0));
	view=ctm;
}

void Renderer::Update_rotate(float angle)
{
	ctm=glm::rotate(ctm, angle, glm::vec3(0.0f ,0.0f ,1.0f) );
	view=ctm;
}

void Renderer::Reset_Transformations()
{
    ctm=glm::mat4(1.0f);
    view=ctm;
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
   
}

//------------------------------------------

void Renderer::Add_New_Shape_Entity()
{
  //Clear data in buffer(CPU)
      Clear_Data();

	if(current_gl_entity!=NULL)
	delete current_gl_entity;

	current_gl_entity = new Shape_Entity();
  current_gl_entity->SetShader(&basic_shader);
}

void Renderer::Add_New_Gradient_Entity()
{
  //Clear data in buffer(CPU)
      Clear_Data();

  if(current_gl_entity!=NULL)
  delete current_gl_entity;

  current_gl_entity = new Gradient_Entity();
  current_gl_entity->SetShader(&gradient_shader);
}

//-------------------------------------------

void Renderer::Add_Vertex(float x,float y,float z)
{
	vertices.push_back(x);
	vertices.push_back(-y);   //  !!! BEWARE !!! ( negation on y , to have coherent with cairo)
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
	 current_gl_entity->SetFillMode(fill);
}

void Renderer::Select_Render_Mode(GLenum mode)
{
   current_gl_entity->SetRenderMode(mode);
}

void Renderer::Select_Line_Width(float width)
{
	 current_gl_entity->SetLineWidth(width);
}

void Renderer::Select_Color(float R,float G,float B,float A)
{
    current_gl_entity->SetColor(R,G,B,A);
}

//---------------------gradient------------------------

void Renderer::Add_Linear_Gradient(float x0,float y0,float x1,float y1)
{
    current_gl_entity->AddLinearGradient(x0,-y0,x1,-y1);   // !!! BEWARE !!!
}

void Renderer::Add_Gradient_Color_Point(float R,float G,float B,float A)
{
    current_gl_entity->AddGradientColorPoint(R,G,B,A);
}

//----------------------------------------------------------
      
void Renderer::Render()
{
      //load data to current entity
      Add_Data_To_GL_Entity();
      
      // Send the proj & view matrix needed to draw
      current_gl_entity->Draw(proj,view);

      //Free the GPU memory
      current_gl_entity->FreeGPU();
      
      //Adding a new entity( Shape_entity )
      Add_New_Shape_Entity();
    
}

void Renderer::Render_Cached_Entity(GL_Entity* cached_entity)
{
  
     current_gl_entity->UnBind();
    
     Save();
	 
       cached_entity->Draw(proj,view);
 
     Restore();
   
}

void Renderer::Render_Cached_Pack(GL_Pack* pack)
{
 
   int l=pack->Pack_Size();

   GL_Entity* gl_entity_in_pack;

   for(int i=0;i<l;i++)
   {
      gl_entity_in_pack=pack->Get_GL_Entity_At(i);
      Render_Cached_Entity(gl_entity_in_pack);
   }

}
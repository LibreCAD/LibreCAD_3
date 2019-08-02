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
   dashes_sum=0; 
   dashes_size=0;
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


  shaders.basic_shader = new Shader();
  shaders.basic_shader->Gen("/home/krixz/LC_PURE/GSoC/LibreCAD_3/lcviewernoqt/painters/opengl/RES/SHADERS/basic_shader.shader");
  shaders.basic_shader->UnBind();

  shaders.gradient_shader = new Shader();
  shaders.gradient_shader->Gen("/home/krixz/LC_PURE/GSoC/LibreCAD_3/lcviewernoqt/painters/opengl/RES/SHADERS/color_vertex_shader.shader");
  shaders.gradient_shader->UnBind();

  shaders.thickline_shader = new Shader();
  shaders.thickline_shader->Gen("/home/krixz/LC_PURE/GSoC/LibreCAD_3/lcviewernoqt/painters/opengl/RES/SHADERS/thickline_shader.shader");
  shaders.thickline_shader->UnBind();

  shaders.linepattern_shader = new Shader();
  shaders.linepattern_shader->Gen("/home/krixz/LC_PURE/GSoC/LibreCAD_3/lcviewernoqt/painters/opengl/RES/SHADERS/dash_pattern_shader.shader");
  shaders.linepattern_shader->UnBind();

  CH_Ptr->Set_Shader_Book(shaders);

    glEnable(GL_BLEND);  
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  
}

void Renderer::Set_Cacher_Ref(Cacher* ch)
{
   CH_Ptr=ch;
}

//-------------------------------------------------


void Renderer::Update_projection(float l,float r,float b,float t)
{
    projB=glm::ortho(-r/2,r/2,b/2,-b/2,-1.0f,1.0f);
    proj=glm::ortho(l,r,b,t,-1.0f,1.0f);

    if(shaders.thickline_shader!=NULL)
    {
      shaders.thickline_shader->Bind();
      shaders.thickline_shader->SetUniform2f("WIN_SCALE",r,b);
      shaders.thickline_shader->UnBind();
    }

    if(shaders.linepattern_shader!=NULL)
    {
      shaders.linepattern_shader->Bind();
      shaders.linepattern_shader->SetUniform2f("WIN_SCALE",r,b);
      shaders.linepattern_shader->UnBind();
    }
    
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
  current_gl_entity->SetType(shaders);
}

void Renderer::Add_New_Gradient_Entity()
{
  //Clear data in buffer(CPU)
      Clear_Data();

  if(current_gl_entity!=NULL)
  delete current_gl_entity;

  current_gl_entity = new Gradient_Entity();
  current_gl_entity->SetType(shaders);
}

//-------------------------------------------
//###########################################################################
void Renderer::Add_Vertex(float x,float y,float z)
{
  //Compute D.. (D=0 if current_vertices=empty) 
  //              else D= distance(this.xy-current_vertices.xy)
  if(current_vertices.size()==0)
    path_distance=0.0f;
  else
  {
     glm::vec2 P=glm::vec2( (*(current_vertices.rbegin())) );
     glm::vec2 Q=glm::vec2( x , -y);
     float d=glm::length(P-Q);
     path_distance+=d;
  }
  //qDebug("i=%d ( X=%f Y=%f ) dis=%f",current_vertices.size(),x,-y,path_distance);
  current_vertices.push_back( glm::vec4(x,-y,z,path_distance) );
}

void Renderer::Append_Vertex_Data()
{
 if(current_vertices.size()>1)
 {
  if(fill==true)
  {
     vertex_data.insert( vertex_data.end() , current_vertices.begin() , current_vertices.end() );
     jumps.push_back(current_vertices.size());
  }

  else
  {
    if(closed==false)
    {
     vertex_data.push_back( *(current_vertices.begin()+1)  );  // 2nd
     vertex_data.insert( vertex_data.end() , current_vertices.begin() , current_vertices.end() );
     vertex_data.push_back( *(current_vertices.rbegin()+1) );    // 2nd Last

     jumps.push_back(current_vertices.size()+2);
    }

    else
    {
     vertex_data.push_back( *(current_vertices.rbegin()) );  // last
     vertex_data.insert( vertex_data.end() , current_vertices.begin() , current_vertices.end() );
     vertex_data.push_back( *(current_vertices.begin())  );    // 1st
     vertex_data.push_back( *(current_vertices.begin()+1)  );  // 2nd

     jumps.push_back(current_vertices.size()+3);
    }
   }
 }
   current_vertices.clear();
}

void Renderer::Jump()
{
    Append_Vertex_Data();
    closed=false;
    path_distance=0.0f;
}


void Renderer::Close_Loop()
{
  closed=true;
}

void Renderer::Clear_Data()
{
  closed=false;
  fill=false;
  path_distance=0.0f;
  vertex_data.clear();
	current_vertices.clear();
	jumps.clear();
}

//----------------------------------------------------

void Renderer::Add_Data_To_GL_Entity()
{
    Append_Vertex_Data();

   current_gl_entity->LoadVertexData(&vertex_data[0].x , vertex_data.size()*(4*sizeof(float)) , jumps );
   
    current_gl_entity->SetLineWidth(line_width);                        // ALERT:
    current_gl_entity->SetDashes(dashes_data,dashes_size,dashes_sum);   // THIS Order
    current_gl_entity->SetFillMode(fill);                               // Is Fixed!!!
    current_gl_entity->SetType(shaders);

}

void Renderer::Select_Fill()
{
  fill=true;
}

void Renderer::Select_Line_Width(float width)
{
   line_width=width;
	
}

void Renderer::Select_Dashes(const double* dashes, const int num_dashes, double offset, bool scaled)
{
  
   if(num_dashes==0)
   { 
     dashes_size=0;
     dashes_sum=0;
     dashes_data.clear();
   }

   else
   { 
     int r;  float d;
       if(num_dashes%2==0)
       r=1;
       else
       r=2;
       
        while(r--)
        { 
            dashes_size+=num_dashes;   
            for(int i=0;i<num_dashes;i++)
            {
              d=(float)(floor(dashes[i]+1));
              dashes_sum+=d;
              dashes_data.push_back(d);
            }
        }

    }

}

void Renderer::Select_Color(float R,float G,float B,float A)
{

  shaders.basic_shader->Bind();
  shaders.basic_shader->SetUniform4f("u_Color",R,G,B,A);
  shaders.basic_shader->UnBind();

  shaders.thickline_shader->Bind();
  shaders.thickline_shader->SetUniform4f("u_Color",R,G,B,A);
  shaders.thickline_shader->UnBind();

  shaders.linepattern_shader->Bind();
  shaders.linepattern_shader->SetUniform4f("u_Color",R,G,B,A);
  shaders.linepattern_shader->UnBind();
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
      current_gl_entity->Draw(proj,projB,view);

      //Free the GPU memory
      current_gl_entity->FreeGPU();
      
      //Adding a new entity( Shape_entity )
      Add_New_Shape_Entity();
    
}

void Renderer::Render_Cached_Entity(GL_Entity* cached_entity)
{
  
     current_gl_entity->UnBind();
    
     Save();
	 
       cached_entity->Draw(proj,projB,view);
 
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
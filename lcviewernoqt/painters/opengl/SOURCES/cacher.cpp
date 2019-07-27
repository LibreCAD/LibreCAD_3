#include "cacher.h"
#include "debug.h"
#include <QtDebug>
using namespace lc::viewer::opengl;

     Cacher::Cacher()
     {
     	fill_mode=GL_LINE;
        render_mode=GL_LINE_STRIP_ADJACENCY;

        model=glm::mat4(1.0f);

        Ready_Fresh_GL_Pack();

     	DebugMessage("Constructor cacher");
     }

	Cacher::~Cacher()
	{

	}

    void Cacher::Set_Shader_Book(struct Shaders_book book)
    {
        shaders=book;
    }

	//---------------------------For Matrix/ Vectors/ Coordinate-----------

	void Cacher::Update_model()
	{

	}

	//-----------

	void Cacher::Update_scale(float scale)
	{
        model=glm::scale(model,glm::vec3(scale,scale,scale));
	}

	void Cacher::Update_translate(float x,float y)
	{
        model=glm::translate(model,glm::vec3(x,y,0.0));
	}

    void Cacher::Update_rotate(float angle)
    {
        model=glm::rotate(model, angle, glm::vec3(0.0f ,0.0f ,1.0f) );
    }

    void Cacher::Reset_Transformations()
    {
        model=glm::mat4(1.0f);
    }

    double Cacher::Get_Scale()
    {
        return model[2][2];
    }

    double Cacher::Get_Translate_X()
    {
        return model[3][0];
    }

    double Cacher::Get_Translate_Y()
    {
        return model[3][1];
    }   

	//------------------------------------for properties ( painter calls)------------------------------------
  
	//###########################################################################
void Cacher::Add_Vertex(float x,float y,float z)
{
  //Compute D.. (D=0 if current_vertices=empty) 
  //              else D= distance(this.xy-current_vertices.xy)
  current_vertices.push_back( glm::vec4(x,-y,z,0.0f) );
}

void Cacher::Append_Vertex_Data()
{
 if(current_vertices.size()>1)
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
   current_vertices.clear();
}

void Cacher::Jump()
{
    Append_Vertex_Data();
    closed=false;
}


void Cacher::Close_Loop()
{
  closed=true;
}

void Cacher::Clear_Data()
{
  closed=false;
  vertex_data.clear();
    current_vertices.clear();
    jumps.clear();
}


	//---------------------------------------------------------------
	void Cacher::Add_Data_To_GL_Entity()
	{
       Append_Vertex_Data();

        current_gl_entity->LoadData(&vertex_data[0].x , vertex_data.size()*(4*sizeof(float)) , jumps );
        current_gl_entity->SetModelMatrix(model);

         current_gl_entity->SetFillMode(fill_mode);
         current_gl_entity->SetRenderMode(render_mode);
         
        
	}

	void Cacher::Select_Fill(GLenum fill)
	{
        fill_mode=fill;
      
	}

	void Cacher::Select_Render_Mode(GLenum mode)
	{
        render_mode=mode;
       
	}

    void Cacher::Select_Color(float R,float G,float B,float A)
    {
       // Till now dont need to be cached
    }

    void Cacher::Select_Line_Width(float width)
    {
       // Till now dont need to be cached
    }

    //--------------------------gradient--------------------------------

    void Cacher::Add_Linear_Gradient(float x0,float y0,float x1,float y1)
    {
        current_gl_entity->AddLinearGradient(x0,-y0,x1,-y1);   //  !!! BEWARE !!! ( negation on y , to have coherent with cairo)
    }

    void Cacher::Add_Gradient_Color_Point(float R,float G,float B,float A)
    {
        current_gl_entity->AddGradientColorPoint(R,G,B,A);
    }


    //--------------------------gl_entity / gl_pack / reset manipulations------------
    
    void Cacher::Set_New_GL_Pack()
    {
        current_gl_pack = new GL_Pack();
    }

    void Cacher::Set_New_Shape_Entity()
    {
        current_gl_entity = new Shape_Entity();
        current_gl_entity->SetShader(shaders.basic_shader);
    }

    void Cacher::Set_New_Gradient_Entity()
    {
        current_gl_entity = new Shape_Entity();
        current_gl_entity->SetShader(shaders.gradient_shader);
    }

    void Cacher::Push_Entity_In_Pack()
    {
        current_gl_pack->Push_Entity_In_Pack( current_gl_entity );
    }

    void Cacher::Set_Default()
    {
    	Clear_Data();
        fill_mode=GL_LINE;
        render_mode=GL_LINE_STRIP_ADJACENCY;

        model=glm::mat4(1.0f);
    }

    void Cacher::Ready_For_Next_GL_Entity()
    {   
    	Add_Data_To_GL_Entity();
    	Push_Entity_In_Pack();
    	Set_Default();
    	Set_New_Shape_Entity();
    }

    void Cacher::Ready_Fresh_GL_Pack()
    {
    	Set_Default();
    	Set_New_GL_Pack();
    	Set_New_Shape_Entity();
    }


    //--------------------------------cache entity pack-----------------

    void Cacher::Save_Entity_Pack(unsigned long id)
    {
    	gl_pack_map.insert(std::make_pair( id, current_gl_pack ) );

    	Ready_Fresh_GL_Pack();
    }

    //--------------------------------caching query/insert/delete------------

    bool Cacher::Is_Entity_Cached_Pack(unsigned long id)
    {
        std::map< unsigned long, GL_Pack* >::iterator it;
        
        it = gl_pack_map.find(id);
        
        if (it != gl_pack_map.end())
          return true;
        else
          return false;	
    }

    GL_Pack* Cacher::Get_Entity_Cached_Pack(unsigned long id)
    {
        std::map< unsigned long, GL_Pack* >::iterator it;
        
        it = gl_pack_map.find(id);
      
        if (it != gl_pack_map.end())
          return (it->second);
        else
          return NULL;	
    }

    void Cacher::Erase_Entity_Pack(unsigned long id)
    {
       std::map< unsigned long, GL_Pack* >::iterator it;
        
        it = gl_pack_map.find(id);
        
        if (it != gl_pack_map.end())
         { 
            (it->second)->Free_GPU_Pack();
            gl_pack_map.erase(it);
         }

      }

    void Cacher::Log_Cached_Packs()   //Temporary to debug
    {
    	std::map< unsigned long, GL_Pack* >::iterator itr;
        qDebug("     |MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM|");

        for(itr=gl_pack_map.begin();itr!=gl_pack_map.end();itr++)
        {
                qDebug("     | pack cached ID =  %u  size=%d  add=%u",itr->first,(itr->second)->Pack_Size(),itr->second);
               
        };
        qDebug("     |WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW|");

    }
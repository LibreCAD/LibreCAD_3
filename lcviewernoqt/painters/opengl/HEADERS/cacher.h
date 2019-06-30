#ifndef CACHER_H
#define CACHER_H
#include <GL/glew.h>  
#include <GL/gl.h>
#include <signal.h> 


#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <utility>
#include <iterator> 
#include <map> 

#include "gl_entity.h"
#include "gl_pack.h"
#include "shader.h"

namespace lc
{
	namespace viewer
	{
		namespace opengl
		{


#define PI 3.14159265

class Cacher
{
private:   
    std::vector<float> vertices;          // for vertex data
    std::vector<unsigned int> indices;    //for indices data

	GLenum render_mode;                   //mode for render
    GLenum fill_mode;                     //mode for filling
    
    glm::mat4 model;                     //model matrix
   
   
	GL_Entity* current_gl_entity;
	GL_Pack* current_gl_pack;

    Shader* basic_shader;
    Shader* gradient_shader;
    Shader* text_shader;

	std::map < unsigned long, GL_Pack* > gl_pack_map;

public:

	Cacher();

	~Cacher();

    void Set_Shader_Ref(Shader* basic, Shader* gradient, Shader* text);

	//---------------------------For Matrix/ Vectors/ Coordinate-----------

	void Update_model();

	//-----------

	void Update_scale(float scale);

	void Update_translate(float x,float y);

    void Update_rotate(float angle);

    void Reset_Transformations();

    double Get_Scale();

    double Get_Translate_X();

    double Get_Translate_Y();    

	//------------------------------------for properties ( painter calls)------------------------------------
  
	void Add_Vertex(float x,float y,float z=0.0f);
	
	void Add_Data_To_GL_Entity();

	void Select_Fill(GLenum fill);

	void Select_Render_Mode(GLenum mode);

    void Select_Color(float R,float G,float B,float A);

    void Select_Line_Width(float width);

    //--------------------------gl_entity / gl_pack / reset manipulations------------
    
    void Set_New_GL_Pack();

    void Set_New_GL_Entity();

    void Push_Entity_In_Pack();

    void Clear_Data();

    void Set_Default();

    void Ready_For_Next_GL_Entity();

    void Ready_Fresh_GL_Pack();

    //--------------------------------caching query/insert/delete------------

    void Save_Entity_Pack(unsigned long id);

    bool Is_Entity_Cached_Pack(unsigned long id);

    GL_Pack* Get_Entity_Cached_Pack(unsigned long id);

    void Erase_Entity_Pack(unsigned long id);

    //-------debug------

    void Log_Cached_Packs(); 

};


}

}

}
#endif // CACHER_H
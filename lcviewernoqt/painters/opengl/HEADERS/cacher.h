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
#include <cmath>

#include "shape_entity.h"
#include "gradient_entity.h"
#include "text_entity.h"
#include "gl_entity.h"
#include "gl_pack.h"
#include "shader.h"
#include "font_book.h"
#include "gl_font.h"

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
    std::vector< glm::vec4 > vertex_data;       // for vertex data
    std::vector<int> jumps;
    std::vector< glm::vec4 > current_vertices;  // for current shape(continous)
    
    float path_distance;                  //distance(scalar) from start point to current
    bool closed=false;
    //--------------------------------------------
	 
    glm::mat4 model;                     //model matrix

    float line_width;
    std::vector<float> dashes_data; float dashes_sum=0; int dashes_size=0;
    bool fill=false;
    //--------------------------------------------
   
	GL_Entity* current_gl_entity;
	GL_Pack* current_gl_pack;

    Shaders_book shaders;
    Font_Book fonts;

	std::map < unsigned long, GL_Pack* > gl_pack_map;

public:

	Cacher();

	~Cacher();

    void Set_Shader_Book(struct Shaders_book& book);

    void Set_Font_Book(Font_Book& book);

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

    //---------------Functions manipulating vertex data(raw)----------------------

    void Add_Vertex(float x,float y,float z=0.0f);
    
    void Append_Vertex_Data();

    void Jump();

    void Clear_Data();

    void Close_Loop();

	//------------------------------------for properties ( painter calls)------------------------------------
  
	void Add_Data_To_GL_Entity();                      

	void Select_Fill();

    void Select_Color(float R,float G,float B,float A);

    void Select_Line_Width(float width);

    void Select_Dashes(const double* dashes, const int num_dashes, double offset, bool scaled);


    //----------------------------gradient------------------------------

    void Add_Linear_Gradient(float x0,float y0,float x1,float y1);

    void Add_Gradient_Color_Point(float R,float G,float B,float A);

    //--------------------------gl_entity / gl_pack / reset manipulations------------
    
    void Set_New_GL_Pack();

    void Set_New_Shape_Entity();

    void Set_New_Gradient_Entity();

    void Set_New_Text_Entity();

    void Push_Entity_In_Pack();

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
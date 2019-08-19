#ifndef RENDERER_H
#define RENDERER_H
#include <GL/glew.h>  
#include <GL/gl.h>
#include <signal.h> 


#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <stack>
#include <vector>
#include <cmath>

#include "gl_entity.h"
#include "shape_entity.h"
#include "gradient_entity.h"
#include "text_entity.h"
#include "shader.h"
#include "gl_pack.h"
#include "cacher.h"
#include "gl_font.h"
#include "font_book.h"
#include "res.h"

namespace lc
{
	namespace viewer
	{
		namespace opengl
		{
//---------------Error handling(if needed)----------------------

 #define ASSERT(x) if (!(x)) raise(SIGINT);
 
 #define GLCall(x) GLClearError();\
   x;\
   ASSERT(GLLogCall(#x,__FILE__,__LINE__))



void GLClearError();
bool GLLogCall(const char* function,const char* file,int line);

//--------------------------------------------------------------

struct context_att
{
     glm::mat4 ctm;                        //CTM
};



//--------------------------------------------------------------

#define PI 3.14159265

class Renderer
{
private: 
    //-------------------------------------FOR VERTEX DATA-----------------  
    std::vector< glm::vec4 > vertex_data;       // for vertex data
    std::vector<int> jumps;
    std::vector< glm::vec4 > current_vertices;  // for current shape(continous)
    
    float path_distance;                  //distance(scalar) from start point to current
    bool closed=false;
    bool fill=false;

    //--------------------------------------FOR DASH DATA---------------
    float line_width;
    std::vector<float> dashes_data; float dashes_sum=0; int dashes_size=0;
   
    //--------------------------------------FOR TEXT DATA-----------------

    std::string text_value;
    std::string font_style;
    float text_height;
    bool no_text_magnify;

    //---------------------------------------------------------------------------
    
    
     glm::mat4 proj;                      //projection matrix
     glm::mat4 projB;
     glm::mat4 view;                      //view matrix for pan
     glm::mat4 ctm;          

	
    std::stack < context_att > context_stack;
    
    //--------------------------------------------------------

    std::string shader_path;
    std::string font_path;
    Shaders_book shaders;

    Font_Book fonts;
    
	GL_Entity* current_gl_entity = NULL;

	Cacher* CH_Ptr;
	
public:

	Renderer();

	~Renderer();

	void CreateShaderProgram();

	void Set_Cacher_Ref(Cacher* ch);

	//---------------------------For Matrix/ Vectors/ Coordinate-----------

	void Update_projection(float l,float r,float b,float t);

	void Update_view();

	//-----------

	void Update_scale(float scale);

	void Update_translate(float x,float y);

    void Update_rotate(float angle);

    void Reset_Transformations();

    double Get_Scale();

    double Get_Translate_X();

    double Get_Translate_Y();    

    //----------
	void Device_To_User(double* x, double* y);

	void User_To_Device(double* x, double* y);

	void Device_To_User_Distance(double* x, double* y);

	void User_To_Device_Distance(double* x, double* y);

	//------------context handling-------------------------------------

	void Save();

	void Restore();

	//---------------Functions manipulating vertex data(raw)----------------------

	void Add_Vertex(float x,float y,float z=0.0f);
	
	void Append_Vertex_Data();

	void Jump();

	void Clear_Data();

	void Close_Loop();

	//---------------Functions manipulating entities---------------------------------------------------------
  
    bool Find_GL_Entity(unsigned int id);
	
	void Use_GL_Entity(unsigned int id);

    void Add_New_Shape_Entity();

    void Add_New_Gradient_Entity();
    
    void Add_New_Text_Entity();

	void Add_Data_To_GL_Entity();
    
    //---------------Functions to select attributes of entities------------
	void Select_Fill();

    void Select_Color(float R,float G,float B,float A);

    void Select_Line_Width(float width);

    void Select_Dashes(const double* dashes, const int num_dashes, double offset, bool scaled);

    void Select_Font_Size(float size, bool deviceCoords);

    void Select_Font_Face(const char* text_style);

    void Select_Font_Value(const char* text_val);

    //--------------------------for gradient entity----------------------------

    void Add_Linear_Gradient(float x0,float y0,float x1,float y1);

    void Add_Gradient_Color_Point(float R,float G,float B,float A);

    //----------------------------render direct entity------------------

	void Render();

	void Set_Default();

	//-----------------------------rendering cached entities---------------

    void Render_Cached_Entity(GL_Entity* entity);

	void Render_Cached_Pack(GL_Pack* pack);


};


}

}

}
#endif // RENDERER_H
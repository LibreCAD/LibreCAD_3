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

#include "gl_entity.h"
#include "shape_entity.h"
#include "shader.h"
#include "gl_pack.h"
#include "cacher.h"

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
    std::vector<float> vertices;          // for vertex data
    std::vector<unsigned int> indices;    //for indices data

    
     Shader basic_shader;                           //Shader
     Shader gradient_shader;                        //Shader color vertex (gradient)

     glm::mat4 proj;                      //projection matrix
     glm::mat4 view;                      //view matrix for pan
     glm::mat4 ctm;          

	
    std::stack < context_att > context_stack;
    
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

	//------------context handling

	void Save();

	void Restore();

	//------------------------------------------------------------------------
  
    bool Find_GL_Entity(unsigned int id);
	
	void Use_GL_Entity(unsigned int id);

    void Add_New_Shape_Entity();

	void Add_Vertex(float x,float y,float z=0.0f);
	
	void Clear_Data();

	void Add_Data_To_GL_Entity();

	void Select_Fill(GLenum fill);

	void Select_Render_Mode(GLenum mode);

    void Select_Color(float R,float G,float B,float A);

    void Select_Line_Width(float width);

    //----------------------------render direct entity------------------

	void Render();

	//-----------------------------rendering cached entities---------------

    void Render_Cached_Entity(GL_Entity* entity);

	void Render_Cached_Pack(GL_Pack* pack);


};


}

}

}
#endif // RENDERER_H
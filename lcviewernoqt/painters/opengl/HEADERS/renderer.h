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
#include "viewersettings.h"

namespace lc
{
	namespace viewer
	{
		namespace opengl
		{

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
    std::vector< glm::vec4 > _vertex_data;       // for vertex data
    std::vector<int> _jumps;
    std::vector< glm::vec4 > _current_vertices;  // for current shape(continous)
    
    float _path_distance;                  //distance(scalar) from start point to current
    bool _closed=false;
    bool _fill=false;

    //--------------------------------------FOR DASH DATA---------------
    float _line_width;
    std::vector<float> _dashes_data; float _dashes_sum=0; int _dashes_size=0;
   
    //--------------------------------------FOR TEXT DATA-----------------

    std::string _text_value;
    std::string _font_style;
    float _text_height;
    bool _no_text_magnify;

    //---------------------------------------------------------------------------
    
    
     glm::mat4 _proj;                      //projection matrix
     glm::mat4 _projB;                     //projectionB for seamless line pattern
     glm::mat4 _view;                      //view matrix for pan
     glm::mat4 _ctm;          

	
    std::stack < context_att > _context_stack;
    
    //--------------------------------------------------------

    std::string _shader_path;
    std::string _font_path;
    Shaders_book _shaders;

    Font_Book _fonts;
    
	GL_Entity* _current_gl_entity = NULL;

	Cacher* _cacherPtr;
	
public:

	Renderer();

	~Renderer();

	void createResources();

	void setCacherRef(Cacher* ch);

	//---------------------------For Matrix/ Vectors/ Coordinate-----------

	void updateProjection(float l,float r,float b,float t);

	void updateView();

	//-----------

	void updateScale(float scale);

	void updateTranslate(float x,float y);

    void updateRotate(float angle);

    void resetTransformations();

    double getScale();

    double getTranslateX();

    double getTranslateY();    

    //----------
	void deviceToUser(double* x, double* y);

	void userToDevice(double* x, double* y);

	void deviceToUserDistance(double* x, double* y);

	void userToDeviceDistance(double* x, double* y);

	//------------context handling-------------------------------------

	void save();

	void restore();

	//---------------Functions manipulating vertex data(raw)----------------------

	void addVertex(float x,float y,float z=0.0f);
	
	void appendVertexData();

	void jump();

	void clearData();

	void closeLoop();

	//---------------Functions manipulating entities---------------------------------------------------------
  
    bool findEntity(unsigned int id);
	
	void useEntity(unsigned int id);

    void addNewShapeEntity();

    void addNewGradientEntity();
    
    void addNewTextEntity();

	void addDataToCurrentEntity();
    
    //---------------Functions to select attributes of entities------------
	void selectFill();

    void selectColor(float R,float G,float B,float A);

    void selectLineWidth(float width);

    void selectDashes(const double* dashes, const int num_dashes, double offset, bool scaled);

    void selectFontSize(float size, bool deviceCoords);

    void selectFontFace(const char* text_style);

    void selectFontValue(const char* text_val);

    //--------------------------for gradient entity----------------------------

    void addLinearGradient(float x0,float y0,float x1,float y1);

    void addGradientColorPoint(float R,float G,float B,float A);

    //----------------------------render direct entity------------------

	void render();

	void setDefault();

	//-----------------------------rendering cached entities---------------

    void renderCachedEntity(GL_Entity* entity);

	void renderCachedPack(GL_Pack* pack);


};


}

}

}
#endif // RENDERER_H
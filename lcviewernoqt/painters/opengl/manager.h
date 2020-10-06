#ifndef MANAGER_H
#define MANAGER_H
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
#include <stack>
#include <cmath>

#include "shape_entity.h"
#include "gradient_entity.h"
#include "text_entity.h"
#include "gl_entity.h"

namespace lc
{
namespace viewer
{
namespace opengl
{
#define PI 3.14159265
class Manager
{
private:
    //----------------------------------FOR VERTEX DATA------------
    std::vector< glm::vec4 > _vertex_data; // for vertex data
    std::vector<int> _jumps;
    std::vector< glm::vec4 > _current_vertices;  // for current shape(continuous)

    float _path_distance;                  //distance(scalar) from start point to current
    bool _closed=false;
    bool _fill=false;
    float _line_width;
    //------------------------------------FOR DASH DATA--------------

    std::vector<float> _dashes_data;
    float _dashes_sum=0;
    int _dashes_size=0;

    //--------------------------------------FOR TEXT DATA-----------------
    std::string _text_value;
    bool _no_text_magnify;

    //--------------------------------------CURRENT ENTITY-----------
    GL_Entity* _current_gl_entity=NULL;

public:
    float _text_height;
    std::string _font_style;
    Font_Book::FontType _font_type = Font_Book::FontType::REGULAR;
    Manager();
    ~Manager();
    //-----------------Functions manipulating entities-----------------------------
    void deleteCurrentEntity();
    void setNewShapeEntity();
    void setNewGradientEntity();
    void setNewTextEntity();
    GL_Entity* getCurrentEntity();

    //---------------Functions manipulating vertex data(raw)----------------------
    void addVertex(float x,float y,float z=0.0f);
    void appendVertexData();
    void jump();
    void clearData();
    void closeLoop();
    void selectFill();

    //----------------Functions manipulating gradient data---------------------------
    void addLinearGradient(float x0,float y0,float x1,float y1);
    void addGradientColorPoint(float R,float G,float B,float A);

    //----------------Functions manipulating dash-gap data---------------------------
    void selectLineWidth(float width);
    void selectDashes(const double* dashes, const int num_dashes, double offset, bool scaled);

    //----------------Functions manipulating Font data-----------------------------
    void selectFontFace(const char* text_style, Font_Book::FontType fontType = Font_Book::FontType::REGULAR);
    void selectFontValue(const char* text_val);
    void selectFontSize(float size, bool deviceCoords);
    //----------------Functions adding data to entity--------------------------------
    void setDefault();
    void addDataToCurrentEntity();
    inline bool isNew() {
        return _current_vertices.size()==0;
    };
};
}
}
}
#endif // MANAGER_H

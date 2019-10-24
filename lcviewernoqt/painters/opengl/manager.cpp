#include "manager.h"
#include <cad/logger/logger.h>

using namespace lc::viewer::opengl;

Manager::Manager()
{
  _dashes_sum=0; 
  _dashes_size=0;
}

Manager::~Manager()
{
}

//-----------------Functions manipulating entities-----------------------------
void Manager::deleteCurrentEntity()
{
  if(_current_gl_entity!=NULL)
  delete _current_gl_entity;
}
void Manager::setNewShapeEntity()
{
  _current_gl_entity = new Shape_Entity();
}

void Manager::setNewGradientEntity()
{
  _current_gl_entity = new Gradient_Entity();
}

void Manager::setNewTextEntity()
{
  _current_gl_entity = new Text_Entity();
}

GL_Entity* Manager::getCurrentEntity()
{
  return _current_gl_entity;
}

//---------------Functions manipulating vertex data(raw)----------------------
void Manager::addVertex(float x,float y,float z)
{
  if(_current_vertices.size()==0)
    _path_distance=0.0f;
  else
  {
     glm::vec2 P=glm::vec2( (*(_current_vertices.rbegin())) );
     glm::vec2 Q=glm::vec2( x , -y);
     float d=glm::length(P-Q);
     _path_distance+=d;
  }
  _current_vertices.push_back( glm::vec4(x,-y,z,_path_distance) );
}

void Manager::appendVertexData()
{
  if(_current_vertices.size()>1)
  {
    if(_fill==true)
    {
      _vertex_data.insert( _vertex_data.end() , _current_vertices.begin() , _current_vertices.end() );
      _jumps.push_back(_current_vertices.size());
    }

    else
    {
      if(_closed==false)
      {
        _vertex_data.push_back( *(_current_vertices.begin()+1)  );  // 2nd
        _vertex_data.insert( _vertex_data.end() , _current_vertices.begin() , _current_vertices.end() );
        _vertex_data.push_back( *(_current_vertices.rbegin()+1) );    // 2nd Last

        _jumps.push_back(_current_vertices.size()+2);
      }

      else
      {
        _vertex_data.push_back( *(_current_vertices.rbegin()) );  // last
        _vertex_data.insert( _vertex_data.end() , _current_vertices.begin() , _current_vertices.end() );
        _vertex_data.push_back( *(_current_vertices.begin())  );    // 1st
        _vertex_data.push_back( *(_current_vertices.begin()+1)  );  // 2nd

        _jumps.push_back(_current_vertices.size()+3);
      }
    }
  }

 else if(_current_vertices.size()==1)
 {
    _vertex_data.insert( _vertex_data.end() , _current_vertices.begin() , _current_vertices.end() );
    _jumps.push_back(_current_vertices.size());
 }

  _current_vertices.clear();
}

void Manager::jump()
{
  appendVertexData();
  _closed=false;
  _path_distance=0.0f;
}

void Manager::closeLoop()
{
  _closed=true;
}

void Manager::clearData()
{
  _closed=false;
  _fill=false;
  _path_distance=0.0f;
  _vertex_data.clear();
  _current_vertices.clear();
  _jumps.clear();
}

void Manager::selectFill()
{
  _fill=true;
}

//----------------Functions manipulating gradient data---------------------------
void Manager::addLinearGradient(float x0,float y0,float x1,float y1)
{
  _current_gl_entity->addLinearGradient(x0,-y0,x1,-y1);   // !!! BEWARE !!!
}

void Manager::addGradientColorPoint(float R,float G,float B,float A)
{
  _current_gl_entity->addGradientColorPoint(R,G,B,A);
}

//----------------Functions manipulating dash-gap data---------------------------
void Manager::selectLineWidth(float width)
{
  _line_width=width;
}

void Manager::selectDashes(const double* dashes, const int num_dashes, double offset, bool scaled)
{
  if(num_dashes==0)
  { 
    _dashes_size=0;
    _dashes_sum=0;
    _dashes_data.clear();
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
      _dashes_size+=num_dashes;   
      for(int i=0;i<num_dashes;i++)
      {
        d=(float)(floor(dashes[i]+1));
        _dashes_sum+=d;
        _dashes_data.push_back(d);
      }
    }
  }
} 

//----------------Functions manipulating Font data-----------------------------    
void Manager::selectFontSize(float size, bool deviceCoords)
{
  _text_height=size;
  _no_text_magnify=deviceCoords;
}

void Manager::selectFontFace(const char* text_style)
{
  _font_style=text_style;
}

void Manager::selectFontValue(const char* text_val)
{
  _text_value=text_val;
}

//------------------Functions adding data to entity----------------------------------------
void Manager::setDefault()
{
  clearData();   
  _font_style="ABeeZee-Regular";
  _text_value=" ";
  _text_height=12;
  _no_text_magnify=false;
}

void Manager::addDataToCurrentEntity()
{
	appendVertexData();
	if (_vertex_data.empty()) {
		LOG_WARNING << "Vertex data is empty. Ignoring." << std::endl;
	}
	else {
		_current_gl_entity->loadVertexData(&_vertex_data[0].x, _vertex_data.size() * (4 * sizeof(float)), _jumps);
		_current_gl_entity->setLineWidth(_line_width);                        // ALERT:
		_current_gl_entity->setDashes(_dashes_data, _dashes_size, _dashes_sum);   // THIS Order
		_current_gl_entity->setFillMode(_fill);                               // Is Fixed!!!
		_current_gl_entity->addTextData(_vertex_data[0], _text_value, _text_height, _no_text_magnify);
	}
}


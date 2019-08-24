#include "renderer.h"
using namespace lc::viewer::opengl;

Renderer::Renderer()
{
  _dashes_sum=0; 
  _dashes_size=0;
  _ctm=glm::mat4(1.0f);   
  _view=_ctm;
  _shader_path=(lc::viewer::viewerSettings.get(SETTINGS_GL_SHADER_PATH)->getString());
  _font_path=(lc::viewer::viewerSettings.get(SETTINGS_GL_FONT_PATH)->getString());
}

Renderer::~Renderer()
{
   //destructor
}

void Renderer::createResources()
{
  _shaders.basic_shader = new Shader();
  _shaders.basic_shader->gen(_shader_path+"basic_shader.shader");
  _shaders.basic_shader->unbind();

  _shaders.gradient_shader = new Shader();
  _shaders.gradient_shader->gen(_shader_path+"color_vertex_shader.shader");
  _shaders.gradient_shader->unbind();

  _shaders.thickline_shader = new Shader();
  _shaders.thickline_shader->gen(_shader_path+"thickline_shader.shader");
  _shaders.thickline_shader->unbind();

  _shaders.linepattern_shader = new Shader();
  _shaders.linepattern_shader->gen(_shader_path+"dash_pattern_shader.shader");
  _shaders.linepattern_shader->unbind();

  _shaders.text_shader = new Shader();
  _shaders.text_shader->gen(_shader_path+"text_shader.shader");
  _shaders.text_shader->unbind();

  _cacherPtr->setShaderBook(_shaders);

  _fonts.createDefaultFont("arial",_font_path+"arial.ttf");
  _fonts.createFont("cac_champagne",_font_path+"cac_champagne.ttf");
   
  _cacherPtr->setFontBook(_fonts); 

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void Renderer::setCacherRef(Cacher* ch)
{
  _cacherPtr=ch;
}
//-------------------------------------------------
void Renderer::updateProjection(float l,float r,float b,float t)
{
  _projB=glm::ortho(-r/2,r/2,b/2,-b/2,-1.0f,1.0f);
  _proj=glm::ortho(l,r,b,t,-1.0f,1.0f);

  if(_shaders.thickline_shader!=NULL)
  {
    _shaders.thickline_shader->bind();
    _shaders.thickline_shader->setUniform2f("WIN_SCALE",r,b);
    _shaders.thickline_shader->unbind();
  }

  if(_shaders.linepattern_shader!=NULL)
  {
    _shaders.linepattern_shader->bind();
    _shaders.linepattern_shader->setUniform2f("WIN_SCALE",r,b);
    _shaders.linepattern_shader->unbind();
  }
}

void Renderer::updateView()
{
  _view=_ctm;
}
//------------------------
void Renderer::updateScale(float scale_f)
{
  _ctm=glm::scale(_ctm,glm::vec3(scale_f,scale_f,scale_f)); 
  _view=_ctm;
}

void Renderer::updateTranslate(float x,float y)
{
  _ctm=glm::translate(_ctm,glm::vec3(x,y,0.0));
  _view=_ctm;
}

void Renderer::updateRotate(float angle)
{
  _ctm=glm::rotate(_ctm, angle, glm::vec3(0.0f ,0.0f ,1.0f) );
  _view=_ctm;
}

void Renderer::resetTransformations()
{
  _ctm=glm::mat4(1.0f);
  _view=_ctm;
}

double Renderer::getScale()
{
  return _ctm[2][2];
}

double Renderer::getTranslateX()
{
  return _ctm[3][0];
}

double Renderer::getTranslateY()
{
  return _ctm[3][1];
}    

//-----------------------

void Renderer::deviceToUser(double* x, double* y)
{
  glm::vec4 temp=glm::vec4(*x,*y,0,1);
  temp=glm::inverse(_ctm) * temp;
  *x=temp.x;
  *y=temp.y;
}

void Renderer::deviceToUserDistance(double* x, double* y)
{
  glm::vec4 temp_vec=glm::vec4(*x,*y,0,1);
  glm::mat4 temp_mat=_ctm;
  temp_mat[3][0]=0;
  temp_mat[3][1]=0;
  temp_vec=glm::inverse( temp_mat ) * temp_vec;
  *x=temp_vec.x;
  *y=temp_vec.y;
}

void Renderer::userToDevice(double* x, double* y)
{
  glm::vec4 temp=glm::vec4(*x,*y,0,1);
  temp=glm::mat4(_ctm) * temp;
  *x=temp.x;
  *y=temp.y;
}

void Renderer::userToDeviceDistance(double* x, double* y)
{
  glm::vec4 temp_vec=glm::vec4(*x,*y,0,1);
  glm::mat4 temp_mat=_ctm;
  temp_mat[3][0]=0;
  temp_mat[3][1]=0;
  temp_vec=glm::inverse( temp_mat ) * temp_vec;
  *x=temp_vec.x;
  *y=temp_vec.y;
}

//------------------------------------------

void Renderer::save()
{
  context_att current_context;
  current_context.ctm = this->_ctm;
  _context_stack.push(current_context);
}

void Renderer::restore()
{
  context_att prev_context = _context_stack.top();
  this->_ctm = prev_context.ctm;
  _context_stack.pop();
  _view=_ctm;
}
//------------------------------------------
void Renderer::addNewShapeEntity()
{
  if(_current_gl_entity!=NULL)
  delete _current_gl_entity;

  _current_gl_entity = new Shape_Entity();
}

void Renderer::addNewGradientEntity()
{
  if(_current_gl_entity!=NULL)
  delete _current_gl_entity;

  _current_gl_entity = new Gradient_Entity();
}

void Renderer::addNewTextEntity()
{
  if(_current_gl_entity!=NULL)
  delete _current_gl_entity;

  _current_gl_entity = new Text_Entity();
}

//-------------------------------------------
void Renderer::addVertex(float x,float y,float z)
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

void Renderer::appendVertexData()
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

void Renderer::jump()
{
  appendVertexData();
  _closed=false;
  _path_distance=0.0f;
}

void Renderer::closeLoop()
{
  _closed=true;
}

void Renderer::clearData()
{
  _closed=false;
  _fill=false;
  _path_distance=0.0f;
  _vertex_data.clear();
  _current_vertices.clear();
  _jumps.clear();
}
//----------------------------------------------------
void Renderer::addDataToCurrentEntity()
{
  appendVertexData();

  _current_gl_entity->loadVertexData(&_vertex_data[0].x , _vertex_data.size()*(4*sizeof(float)) , _jumps );
   
  _current_gl_entity->setLineWidth(_line_width);                        // ALERT:
  _current_gl_entity->setDashes(_dashes_data,_dashes_size,_dashes_sum);   // THIS Order
  _current_gl_entity->setFillMode(_fill);                               // Is Fixed!!!
  _current_gl_entity->setType(_shaders);
  _current_gl_entity->setFont(_fonts,_font_style);
  _current_gl_entity->addTextData(_vertex_data[0], _text_value, _text_height, _no_text_magnify);
}

void Renderer::selectFill()
{
  _fill=true;
}

void Renderer::selectLineWidth(float width)
{
  _line_width=width;
}

void Renderer::selectDashes(const double* dashes, const int num_dashes, double offset, bool scaled)
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

void Renderer::selectColor(float R,float G,float B,float A)
{
  _shaders.basic_shader->bind();
  _shaders.basic_shader->setUniform4f("u_Color",R,G,B,A);
  _shaders.basic_shader->unbind();

  _shaders.thickline_shader->bind();
  _shaders.thickline_shader->setUniform4f("u_Color",R,G,B,A);
  _shaders.thickline_shader->unbind();

  _shaders.linepattern_shader->bind();
  _shaders.linepattern_shader->setUniform4f("u_Color",R,G,B,A);
  _shaders.linepattern_shader->unbind();

  _shaders.text_shader->bind();
  _shaders.text_shader->setUniform4f("u_Color",R,G,B,A);
  _shaders.text_shader->unbind();
}


void Renderer::selectFontSize(float size, bool deviceCoords)
{
  _text_height=size;
  _no_text_magnify=deviceCoords;
}

void Renderer::selectFontFace(const char* text_style)
{
  _font_style=text_style;
}

void Renderer::selectFontValue(const char* text_val)
{
  _text_value=text_val;
}

GL_Text_Extend Renderer::getTextExtend(const char* text_val)
{
  GL_Text_Extend TE=(_fonts.pickFont(_font_style))->getTextExtend(std::string(text_val),_text_height);
  return TE;
}

//---------------------gradient------------------------

void Renderer::addLinearGradient(float x0,float y0,float x1,float y1)
{
  _current_gl_entity->addLinearGradient(x0,-y0,x1,-y1);   // !!! BEWARE !!!
}

void Renderer::addGradientColorPoint(float R,float G,float B,float A)
{
  _current_gl_entity->addGradientColorPoint(R,G,B,A);
}

//----------------------------------------------------------
void Renderer::setDefault()
{
  clearData();   
  _font_style="arial";
  _text_value=" ";
  _text_height=12;
  _no_text_magnify=false;
}

void Renderer::render()
{
  //load data to current entity
  addDataToCurrentEntity();   
  // Send the _proj & _view matrix needed to draw
  _current_gl_entity->draw(_proj,_projB,_view);
  //Free the GPU memory
  _current_gl_entity->freeGPU();     
  //Clear data in buffer(CPU)
  setDefault();
  //Adding a new entity( Shape_entity )
  addNewShapeEntity();    
}

void Renderer::renderCachedEntity(GL_Entity* cached_entity)
{
  _current_gl_entity->unbind();   
  save();
  cached_entity->draw(_proj,_projB,_view);
  restore();
}

void Renderer::renderCachedPack(GL_Pack* pack)
{
  int l=pack->packSize();
  GL_Entity* gl_entity_in_pack;

  for(int i=0;i<l;i++)
  {
    gl_entity_in_pack=pack->getEntityAt(i);
    renderCachedEntity(gl_entity_in_pack);
  }
}
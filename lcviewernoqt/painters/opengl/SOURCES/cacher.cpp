#include "cacher.h"

using namespace lc::viewer::opengl;

  Cacher::Cacher()
  {
     	  _dashes_sum=0; 
        _dashes_size=0;
        _model=glm::mat4(1.0f);

        readyFreshPack();

  }

	Cacher::~Cacher()
	{

	}

  void Cacher::setShaderBook(struct Shaders_book& book)
  {
        _shaders=book;
  }

  void Cacher::setFontBook(Font_Book& book)
  {
        _fonts=book;
  }

	//---------------------------For Matrix/ Vectors/ Coordinate-----------

	void Cacher::updateModel()
	{

	}

	//-----------

	void Cacher::updateScale(float scale)
	{
        _model=glm::scale(_model,glm::vec3(scale,scale,scale));
	}

	void Cacher::updateTranslate(float x,float y)
	{
        _model=glm::translate(_model,glm::vec3(x,y,0.0));
	}

    void Cacher::updateRotate(float angle)
    {
        _model=glm::rotate(_model, angle, glm::vec3(0.0f ,0.0f ,1.0f) );
    }

    void Cacher::resetTransformations()
    {
        _model=glm::mat4(1.0f);
    }

    double Cacher::getScale()
    {
        return _model[2][2];
    }

    double Cacher::getTranslateX()
    {
        return _model[3][0];
    }

    double Cacher::getTranslateY()
    {
        return _model[3][1];
    }   

	//------------------------------------for properties ( painter calls)------------------------------------
  
	//###########################################################################
void Cacher::addVertex(float x,float y,float z)
{
  //Compute D.. (D=0 if _current_vertices=empty) 
  //              else D= distance(this.xy-_current_vertices.xy)
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

void Cacher::appendVertexData()
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

void Cacher::jump()
{
    appendVertexData();
    _closed=false;
    _path_distance=0.0f;
}


void Cacher::closeLoop()
{
  _closed=true;
}

void Cacher::clearData()
{
  _closed=false;
  _fill=false;
  _path_distance=0.0f;
  _vertex_data.clear();
  _current_vertices.clear();
  _jumps.clear();

}


	//---------------------------------------------------------------
	void Cacher::addDataToCurrentEntity()
	{
       appendVertexData();

        _current_gl_entity->loadVertexData(&_vertex_data[0].x , _vertex_data.size()*(4*sizeof(float)) , _jumps );
        _current_gl_entity->setModelMatrix(_model);          
        
        _current_gl_entity->setLineWidth(_line_width);                        // ALERT:
        _current_gl_entity->setDashes(_dashes_data,_dashes_size,_dashes_sum);   // THIS Order
        _current_gl_entity->setFillMode(_fill);                               // Is Fixed!!!
        _current_gl_entity->setType(_shaders);
        _current_gl_entity->setFont(_fonts,_font_style);
        _current_gl_entity->addTextData(_vertex_data[0], _text_value, _text_height, _no_text_magnify);
        
	}

	void Cacher::selectFill()
	{
        _fill=true;  
	}

    void Cacher::selectColor(float R,float G,float B,float A)
    {
       // Till now dont need to be cached
    }

    void Cacher::selectLineWidth(float width)
    {
       _line_width=width;
       
    }

    void Cacher::selectDashes(const double* dashes, const int num_dashes, double offset, bool scaled)
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

    void Cacher::selectFontSize(float size, bool deviceCoords)
    {
        _text_height=size;
        _no_text_magnify=deviceCoords;
    }

    void Cacher::selectFontFace(const char* text_style)
    {
        _font_style=text_style;
    }

    void Cacher::selectFontValue(const char* text_val)
    {
        _text_value=text_val;
    }

    GL_Text_Extend Cacher::getTextExtend(const char* text_val)
    {
         GL_Text_Extend TE=(_fonts.pickFont(_font_style))->getTextExtend(std::string(text_val),_text_height);
         return TE;
    }


    //--------------------------gradient--------------------------------

    void Cacher::addLinearGradient(float x0,float y0,float x1,float y1)
    {
        _current_gl_entity->addLinearGradient(x0,-y0,x1,-y1);   //  !!! BEWARE !!! ( negation on y , to have coherent with cairo)
    }

    void Cacher::addGradientColorPoint(float R,float G,float B,float A)
    {
        _current_gl_entity->addGradientColorPoint(R,G,B,A);
    }


    //--------------------------gl_entity / gl_pack / reset manipulations------------
    
    void Cacher::setNewPack()
    {
        _current_gl_pack = new GL_Pack();
    }

    void Cacher::setNewShapeEntity()
    {
        _current_gl_entity = new Shape_Entity();
    }

    void Cacher::setNewGradientEntity()
    {
        _current_gl_entity = new Shape_Entity();
    }

    void Cacher::setNewTextEntity()
    {
        _current_gl_entity = new Text_Entity();
    }

    void Cacher::pushEntityInPack()
    {
        _current_gl_pack->pushEntityInPack( _current_gl_entity );
    }

    void Cacher::setDefault()
    {
    	clearData();
       
        _model=glm::mat4(1.0f);

         _font_style="arial";
         _text_value=" ";
         _text_height=12;
         _no_text_magnify=false;
    }

    void Cacher::readyForNextEntity()
    {   
    	addDataToCurrentEntity();
    	pushEntityInPack();
    	setDefault();
    	setNewShapeEntity();
    }

    void Cacher::readyFreshPack()
    {
    	setDefault();
    	setNewPack();
    	setNewShapeEntity();
    }


    //--------------------------------cache entity pack-----------------

    void Cacher::savePack(unsigned long id)
    {
    	_gl_pack_map.insert(std::make_pair( id, _current_gl_pack ) );

    	readyFreshPack();
    }

    //--------------------------------caching query/insert/delete------------

    bool Cacher::isPackCached(unsigned long id)
    {
        std::map< unsigned long, GL_Pack* >::iterator it;
        
        it = _gl_pack_map.find(id);
        
        if (it != _gl_pack_map.end())
          return true;
        else
          return false;	
    }

    GL_Pack* Cacher::getCachedPack(unsigned long id)
    {
        std::map< unsigned long, GL_Pack* >::iterator it;
        
        it = _gl_pack_map.find(id);
      
        if (it != _gl_pack_map.end())
          return (it->second);
        else
          return NULL;	
    }

    void Cacher::erasePack(unsigned long id)
    {
       std::map< unsigned long, GL_Pack* >::iterator it;
        
        it = _gl_pack_map.find(id);
        
        if (it != _gl_pack_map.end())
         { 
            (it->second)->freePackGPU();
            _gl_pack_map.erase(it);
         }

      }

    void Cacher::logCachedPacks()   //Temporary to debug
    {
    	

    }
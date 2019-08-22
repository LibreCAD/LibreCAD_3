
#include "openglpainter.h"
#include <cmath>
#include <QOpenGLContext>


LcOpenGLPainter::LcOpenGLPainter(unsigned int width, unsigned int height) 
{      
   
  new_device_size(width,height);

   _cacher_painter=new LcCacherPainter();

    QOpenGLContext *CC= QOpenGLContext::currentContext();
     if(CC!=0)
    {
       create_resources();
      _renderer.addNewShapeEntity();
    }

   
}

void LcOpenGLPainter::create_resources()
{
     LcCacherPainter* cp= dynamic_cast<LcCacherPainter*>(_cacher_painter);
     _renderer.setCacherRef(&(((*cp)._cacher)));

     _renderer.createResources();
    
}

 void LcOpenGLPainter::new_device_size(unsigned int width, unsigned int height)
 {
     if (height == 0) 
        height = 1;     
                   
   GLfloat aspect = (GLfloat)width / (GLfloat)height;
 
   glViewport(0, 0, width, height);

   _device_width=(float)width;
   _device_height=(float)height;

   _renderer.updateProjection(0, _device_width, _device_height,0);
   
   
 }



                 void LcOpenGLPainter::new_path()
                 {

                 }

                 void LcOpenGLPainter::close_path()
                 {
                       _renderer.closeLoop();
                 }

                 void LcOpenGLPainter::new_sub_path()
                 {

                 }

                 void LcOpenGLPainter::clear(double r, double g, double b)
                 {
                         glClearColor(r,g,b,1);
                         glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                 }

                 void LcOpenGLPainter::clear(double r, double g, double b, double a)
                 {
                         glClearColor(r,g,b,a);
                         glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                 }

                 void LcOpenGLPainter::move_to(double x, double y)
                 {

                        _renderer.jump();
                        _renderer.addVertex(x,y);
                        _pen_x=x;  _pen_y=y;
                 }

                 void LcOpenGLPainter::line_to(double x, double y)
                 {
                        _renderer.addVertex(x,y);
                        _pen_x=x;  _pen_y=y;
                 }

                 void LcOpenGLPainter::lineWidthCompensation(double lwc) // When set, we add this to the current linewidth, perhaps we should find a better method
                 {

                 }
                 void LcOpenGLPainter::line_width(double lineWidth)
                 {
                      _renderer.selectLineWidth(lineWidth);
                 }

                 double LcOpenGLPainter::scale()
                 {
                      return _renderer.getScale();
                 }

                 void LcOpenGLPainter::scale(double s)
                 {
                     _renderer.updateScale(s);
                 }

                 void LcOpenGLPainter::rotate(double r)
                 {
                     _renderer.updateRotate(r);
                 }

                 void LcOpenGLPainter::arc(double x, double y, double r, double start, double end)
                 {  
                       if(start>end)
                        end+=2*PI;

                    _renderer.jump();
                    
                      float delta=(std::abs(end-start));
                      float angle=0;
                      long points=CURVE_POINTS;
                        
                        for(int i=0;i<points;i++)
                        {
                            angle=( ((float)i)/points)*(delta) + (start);
                            
                            _renderer.addVertex( (x+r*cos(angle)) , (y+r*sin(angle)) );
                        }

                 }

                 void LcOpenGLPainter::arcNegative(double x, double y, double r, double start, double end)
                 {      
                        if(start<end)
                        end-=2*PI;

                        _renderer.jump();

                        float delta=(std::abs(end-start));
                        float angle=0;
                        long points=CURVE_POINTS;

                        for(int i=0;i<points;i++)
                        {
                            angle=start - ( ((float)i)/points)*(delta) ;
                            
                            _renderer.addVertex( (x+r*cos(angle)) , (y+r*sin(angle)) );
                        }
                        
                 }

                 void LcOpenGLPainter::circle(double x, double y, double r)
                 {
                        _renderer.jump();

                        float angle=0;
                        for(int i=0;i<CURVE_POINTS;i++)
                        {
                            angle=( ((float)i)/CURVE_POINTS )*(2*PI);
                            _renderer.addVertex( (x+r*cos(angle)) , (y+r*sin(angle)) );
                        }

                        _renderer.closeLoop();
                       
                 }

                 void LcOpenGLPainter::ellipse(double cx, double cy, double rx, double ry, double sa, double ea, double ra)
                 {
                       _renderer.jump();

                      
                        float SA= atan( (rx/ry)* tan(sa) );   //Finding eccentric angles
                        float EA= atan( (rx/ry)* tan(ea) );

                        if(sa>(PI/2) && sa<=(3*(PI/2)) )      //2nd-3rd Quadrant
                         SA+=PI;

                        else if(sa>(3*(PI/2)) && sa<=(2*PI) )  // 4th Quadrant
                         SA+=2*PI;   

                         if(ea>(PI/2) && ea<=(3*(PI/2)) )       //2nd-3rd Quadrant
                         EA+=PI;

                        else if(ea>(3*(PI/2)) && ea<=(2*PI) )   // 4th Quadrant
                         EA+=2*PI;   


                        float delta=(std::abs(EA-SA));
                           if(ea==sa)                           // 360 full
                            delta=2*PI;

                        float A=0;
                        long points=CURVE_POINTS;
                        float tx,ty,Tx,Ty;
                        
                        for(int i=0;i<=points;i++)
                        {
                            A=( ((float)i)/points)*(delta) + (SA);
                           
                            tx=rx*cos(A);                       // parametric equation
                            ty=ry*sin(A);
                            
                            Tx=(tx*cos(ra) - ty*sin(ra)) + cx;  // first rotate then shift origin
                            Ty=(tx*sin(ra) + ty*cos(ra)) + cy;

                            _renderer.addVertex( Tx , Ty );
                            
                        }



                 }

                 void LcOpenGLPainter::rectangle(double x1, double y1, double w, double h)
                 { 
                        _renderer.jump();
                        
                        _renderer.addVertex(x1,y1);
                        _renderer.addVertex(x1+w,y1);
                        _renderer.addVertex(x1+w,y1+h);
                        _renderer.addVertex(x1,y1+h);
                        
                        _renderer.closeLoop();
                 }

                 void LcOpenGLPainter::stroke()
                 {
                      _renderer.render();
                 }

                 void LcOpenGLPainter::source_rgb(double r, double g, double b)
                 {
                      _renderer.selectColor(r,g,b,1.0f);
                 }

                 void LcOpenGLPainter::source_rgba(double r, double g, double b, double a)
                 {
                      _renderer.selectColor(r,g,b,a);
                 }

                 void LcOpenGLPainter::translate(double x, double y)
                 {
                      _renderer.updateTranslate(x,y);
                 }

                 void LcOpenGLPainter::user_to_device(double* x, double* y)
                 {
                    *y=-*y;
                    _renderer.userToDevice(x,y);
                 }

                 void LcOpenGLPainter::device_to_user(double* x, double* y)
                 {
                    _renderer.deviceToUser(x,y);
                    *y=-*y;
                 }

                 void LcOpenGLPainter::user_to_device_distance(double* dx, double* dy)
                 {
                    *dy = -*dy;
                    _renderer.userToDeviceDistance(dx,dy);
                 }

                 void LcOpenGLPainter::device_to_user_distance(double* dx, double* dy)
                 {
                    _renderer.deviceToUserDistance(dx,dy);
                    *dy = -*dy;
                 }

                 void LcOpenGLPainter::font_size(double size, bool deviceCoords)
                 {
                      _renderer.selectFontSize(size,deviceCoords);
                 }

                 void LcOpenGLPainter::select_font_face(const char* text_val)
                 {
                      _renderer.selectFontFace(text_val);
                 }

                 void LcOpenGLPainter::text(const char* text_val)
                 {
                     _renderer.addNewTextEntity();
                     _renderer.selectFontValue(text_val);
                 }

                 TextExtends LcOpenGLPainter::text_extends(const char* text_val)
                 {
                    TextExtends te;

                    GL_Text_Extend extents=_renderer.getTextExtend(text_val);
        

                    te.height = extents.height;
                    te.width = extents.width;
                    te.x_advance = extents.x_advance;
                    te.x_bearing = extents.x_bearing;
                    te.y_advance = extents.y_advance;
                    te.y_bearing = extents.y_bearing;

                    return te;
                }

                 void LcOpenGLPainter::quadratic_curve_to(double x1, double y1, double x2, double y2)
                 {
                     double x0=_pen_x; double y0=_pen_y;

                     long points=CURVE_POINTS;
                     float Px,Py,t; 

                    for(int i=0;i<=points;i++)
                    {
                        t=((float)(i)/(float)(CURVE_POINTS));
                        Px=(1-t)*(1-t)*x0 + 2*t*(1-t)*x1 + t*t*x2;
                        Py=(1-t)*(1-t)*y0 + 2*t*(1-t)*y1 + t*t*y2;
                        _renderer.addVertex(Px,Py);
                    }

                    _pen_x=Px; _pen_y=Py;
                 }

                 void LcOpenGLPainter::curve_to(double x1, double y1, double x2, double y2, double x3, double y3)
                 {
                    double x0=_pen_x; double y0=_pen_y;

                     long points=CURVE_POINTS;
                     float Px,Py,t; 

                    for(int i=0;i<=points;i++)
                    {
                        t=((float)(i)/(float)(CURVE_POINTS));
                        Px=(1-t)*(1-t)*(1-t)*x0 + 3*t*(1-t)*(1-t)*x1  + 3*t*t*(1-t)*x2 + t*t*t*x3;
                        Py=(1-t)*(1-t)*(1-t)*y0 + 3*t*(1-t)*(1-t)*y1  + 3*t*t*(1-t)*y2 + t*t*t*y3;
                        _renderer.addVertex(Px,Py);
                    }

                     _pen_x=Px; _pen_y=Py;
                 }

                 void LcOpenGLPainter::save()
                 {
                      _renderer.save();
                 }

                 void LcOpenGLPainter::restore()
                 {
                      _renderer.restore();

                 }

                 long LcOpenGLPainter::pattern_create_linear(double x1, double y1, double x2, double y2)
                 {
                      _renderer.addNewGradientEntity();
                      _renderer.addLinearGradient(x1,y1,x2,y2);
                      return 0;
                 }

                 void LcOpenGLPainter::pattern_add_color_stop_rgba(long pat, double offset, double r, double g, double b, double a)
                 {
                      _renderer.addGradientColorPoint(r,g,b,a);
                 }

                 void LcOpenGLPainter::set_pattern_source(long pat)
                 {

                 }

                 void LcOpenGLPainter::pattern_destroy(long pat)
                 {

                 }

                 void LcOpenGLPainter::fill()
                 {
                      _renderer.selectFill();
                 }

                 void LcOpenGLPainter::point(double x, double y, double size, bool deviceCoords)
                 {

                 }

                 void LcOpenGLPainter::reset_transformations()
                 {

                      _renderer.resetTransformations();

                 }

                 unsigned char* LcOpenGLPainter::data()
                 {

                 }

                 void LcOpenGLPainter::set_dash(const double* dashes, const int num_dashes, double offset, bool scaled)
                 {
                      _renderer.selectDashes(dashes,num_dashes,offset,scaled);
                 }

                 void LcOpenGLPainter::dash_destroy()
                 {
                      _renderer.selectDashes(NULL,0,0,false);
                 }

                 long LcOpenGLPainter::image_create(const std::string& file)
                 {

                 }

                 void LcOpenGLPainter::image_destroy(long image)
                 {

                 }

                 void LcOpenGLPainter::image(long image, double uvx, double vy, double vvx, double vvy, double x, double y)
                 {

                 }

                void LcOpenGLPainter::disable_antialias()
                {

                }

                 void LcOpenGLPainter::enable_antialias()
                 {

                 }

                 void LcOpenGLPainter::getTranslate(double* x, double* y)
                 {

                 }
                 
                 void LcOpenGLPainter::startcaching()
                 {
                     // NOTHING to DO.. (CachePainter Use this)
                 }

                 void LcOpenGLPainter::finishcaching(unsigned long id)
                 {
                     // NOTHING to DO.. (CachePainter Use this)
                 }

                 LcPainter* LcOpenGLPainter::getCacherpainter()
                 {
                    return _cacher_painter;
                 }

                 bool LcOpenGLPainter::isEntityCached(unsigned long id)
                 {
                    return _cacher_painter->isEntityCached(id);
                 }

                 void LcOpenGLPainter::renderEntityCached(unsigned long id)
                 {
                    LcCacherPainter* cp= dynamic_cast<LcCacherPainter*>(_cacher_painter);
                    GL_Pack* _gl_pack=((*cp)._cacher).getCachedPack(id);
                    _renderer.renderCachedPack(_gl_pack);
                 }

                 void LcOpenGLPainter::deleteEntityCached(unsigned long id)
                 {
                    _cacher_painter->deleteEntityCached(id);
                 }
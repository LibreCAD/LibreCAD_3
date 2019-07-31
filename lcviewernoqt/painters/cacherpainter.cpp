
#include "cacherpainter.h"
#include <QtDebug>


LcCacherPainter::LcCacherPainter() 
{      
   qDebug(" cacherpainter-- Constructor");
  
}

void LcCacherPainter::ReadyShaderProgram()
{
    // NO Need ( main painter uses for rendering)
}

void LcCacherPainter::new_device_size(unsigned int width, unsigned int height)
{
    // NO Need ( main painter uses for rendering)
}


                 void LcCacherPainter::new_path()
                 {

                 }

                 void LcCacherPainter::close_path()
                 {
                       _cacher.Close_Loop();
                 }

                 void LcCacherPainter::new_sub_path()
                 {

                 }

                 void LcCacherPainter::clear(double r, double g, double b)
                 {
                        // NO Need ( main painter uses for rendering)
                 }

                 void LcCacherPainter::clear(double r, double g, double b, double a)
                 {
                       // NO Need ( main painter uses for rendering)  
                 }

                 void LcCacherPainter::move_to(double x, double y)
                 {
                      _cacher.Jump();
                      _cacher.Add_Vertex(x,y);
                 }

                 void LcCacherPainter::line_to(double x, double y)
                 {
                      _cacher.Add_Vertex(x,y);
                 }

                 void LcCacherPainter::lineWidthCompensation(double lwc) // When set, we add this to the current linewidth, perhaps we should find a better method
                 {

                 }
                 void LcCacherPainter::line_width(double lineWidth)
                 {
                     _cacher.Select_Line_Width(lineWidth);
                 }

                 double LcCacherPainter::scale()
                 {
                     return _cacher.Get_Scale();
                 }

                 void LcCacherPainter::scale(double s)
                 {
                    _cacher.Update_scale(s);
                 }

                 void LcCacherPainter::rotate(double r)
                 {
                     _cacher.Update_rotate(-r);
                 }

                 void LcCacherPainter::arc(double x, double y, double r, double start, double end)
                 {
                      _cacher.Jump();
                    
                      float delta=(std::abs(end-start));
                      float angle=0;
                      long points=curve_points;
                        
                        for(int i=0;i<points;i++)
                        {
                            angle=( ((float)i)/points)*(delta) + (start);
                            
                            _cacher.Add_Vertex( (x+r*cos(angle)) , (y+r*sin(angle)) );
                        }

                 }

                 void LcCacherPainter::arcNegative(double x, double y, double r, double start, double end)
                 {
                       _cacher.Jump();

                        float delta=(std::abs(end-start));
                        float angle=0;
                        long points=curve_points;

                        for(int i=0;i<points;i++)
                        {
                            angle=start - ( ((float)i)/points)*(delta) ;
                            
                            _cacher.Add_Vertex( (x+r*cos(angle)) , (y+r*sin(angle)) );
                        }
                 }

                 void LcCacherPainter::circle(double x, double y, double r)
                 {
                      _cacher.Jump();

                        float angle=0;
                        for(int i=0;i<curve_points;i++)
                        {
                            angle=( ((float)i)/curve_points )*(2*PI);
                            _cacher.Add_Vertex( (x+r*cos(angle)) , (y+r*sin(angle)) );
                        }

                        _cacher.Close_Loop();

                 }

                 void LcCacherPainter::ellipse(double cx, double cy, double rx, double ry, double sa, double ea, double ra)
                 {

                 }

                 void LcCacherPainter::rectangle(double x1, double y1, double w, double h)
                 { 
                        _cacher.Jump();
                        
                        _cacher.Add_Vertex(x1,y1);
                        _cacher.Add_Vertex(x1+w,y1);
                        _cacher.Add_Vertex(x1+w,y1+h);
                        _cacher.Add_Vertex(x1,y1+h);
                        
                        _cacher.Close_Loop();
                 }

                 void LcCacherPainter::stroke()
                 {
                     _cacher.Ready_For_Next_GL_Entity();
                    
                 }

                 void LcCacherPainter::source_rgb(double r, double g, double b)
                 {
                     // Implement later when required
                 }

                 void LcCacherPainter::source_rgba(double r, double g, double b, double a)
                 {
                     // Implement later when required
                 }

                 void LcCacherPainter::translate(double x, double y)
                 {
                      _cacher.Update_translate(x,-y );
                 }

                 void LcCacherPainter::user_to_device(double* x, double* y)
                 {
                     // NO Need ( main painter uses for rendering)
                 }

                 void LcCacherPainter::device_to_user(double* x, double* y)
                 {
                     // NO Need ( main painter uses for rendering)
                 }

                 void LcCacherPainter::user_to_device_distance(double* dx, double* dy)
                 {
                    // NO Need ( main painter uses for rendering)
                 }

                 void LcCacherPainter::device_to_user_distance(double* dx, double* dy)
                 {
                     // NO Need ( main painter uses for rendering)
                 }

                 void LcCacherPainter::font_size(double size, bool deviceCoords)
                 {
                      
                 }

                 void LcCacherPainter::select_font_face(const char* text_val)
                 {

                 }

                 void LcCacherPainter::text(const char* text_val)
                 {
                    
                 }

                 TextExtends LcCacherPainter::text_extends(const char* text_val)
                 {

                 }

                 void LcCacherPainter::quadratic_curve_to(double x1, double y1, double x2, double y2)
                 {

                 }

                 void LcCacherPainter::curve_to(double x1, double y1, double x2, double y2, double x3, double y3)
                 {

                 }

                 void LcCacherPainter::save()
                 {
                     
                 }

                 void LcCacherPainter::restore()
                 {
                     

                 }

                 long LcCacherPainter::pattern_create_linear(double x1, double y1, double x2, double y2)
                 {
                     _cacher.Set_New_Gradient_Entity();
                     _cacher.Add_Linear_Gradient(x1,y1,x2,y2);
                 }

                 void LcCacherPainter::pattern_add_color_stop_rgba(long pat, double offset, double r, double g, double b, double a)
                 {
                     _cacher.Add_Gradient_Color_Point(r,g,b,a);
                 }

                 void LcCacherPainter::set_pattern_source(long pat)
                 {

                 }

                 void LcCacherPainter::pattern_destroy(long pat)
                 {

                 }

                 void LcCacherPainter::fill()
                 {
                     _cacher.Select_Fill();
                 }

                 void LcCacherPainter::point(double x, double y, double size, bool deviceCoords)
                 {

                 }

                 void LcCacherPainter::reset_transformations()
                 {

                 }

                 unsigned char* LcCacherPainter::data()
                 {

                 }

                 void LcCacherPainter::set_dash(const double* dashes, const int num_dashes, double offset, bool scaled)
                 {
                    _cacher.Select_Dashes(dashes,num_dashes,offset,scaled);
                 }

                 void LcCacherPainter::dash_destroy()
                 {
                    _cacher.Select_Dashes(NULL,0,0,false);
                 }

                 long LcCacherPainter::image_create(const std::string& file)
                 {

                 }

                 void LcCacherPainter::image_destroy(long image)
                 {

                 }

                 void LcCacherPainter::image(long image, double uvx, double vy, double vvx, double vvy, double x, double y)
                 {

                 }

                void LcCacherPainter::disable_antialias()
                {

                }

                 void LcCacherPainter::enable_antialias()
                 {

                 }

                 void LcCacherPainter::getTranslate(double* x, double* y)
                 {

                 }
                 
                 void LcCacherPainter::startcaching()
                 {
                    qDebug("<<<<<<<<<<<<<<<<<<<caching STARTS>>>>>>>>>>>>>>>>");
                     _cacher.Ready_Fresh_GL_Pack();
                 }

                 void LcCacherPainter::finishcaching(unsigned long id)
                 {
                    qDebug("<<<<<<<<<<<<<<<<<<<caching FINISH>>>>>>>>>>>>>>>>> id=%u",id);
                     _cacher.Save_Entity_Pack(id);

                     //_cacher.Log_Cached_Packs();  // DEBUG
                 }

                 LcPainter* LcCacherPainter::getCacherpainter()
                 {
                    // NO Need ( main painter uses)
                 }

                 bool LcCacherPainter::isEntityCached(unsigned long id)
                 {
                      return _cacher.Is_Entity_Cached_Pack(id);
                 }

                 void LcCacherPainter::renderEntityCached(unsigned long id)
                 {
                    // No Need ( cant do rendering here)
                 }

                 void LcCacherPainter::deleteEntityCached(unsigned long id)
                 {
                    _cacher.Erase_Entity_Pack(id);
                    //_cacher.Log_Cached_Packs();  // DEBUG
                 }

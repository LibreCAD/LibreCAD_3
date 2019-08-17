
#include "openglpainter.h"
#include <QtDebug>
#include <cmath>
#include <QOpenGLContext>


LcOpenGLPainter::LcOpenGLPainter(unsigned int width, unsigned int height) 
{      
   
  new_device_size(width,height);

   CHE=new LcCacherPainter();

   qDebug( "=======================Opengl_Painter_Created=============%u w=%d h=%d",this,width,height);
    QOpenGLContext *CC= QOpenGLContext::currentContext();
     if(CC!=0)
    {
      ReadyShaderProgram();
      RND.Add_New_Shape_Entity();
    }

   
}

void LcOpenGLPainter::ReadyShaderProgram()
{
     LcCacherPainter* cp= dynamic_cast<LcCacherPainter*>(CHE);
     RND.Set_Cacher_Ref(&(((*cp)._cacher)));

     RND.CreateShaderProgram();
     qDebug( "==RND.CreatingShader() call");

    
}

 void LcOpenGLPainter::new_device_size(unsigned int width, unsigned int height)
 {
     if (height == 0) 
        height = 1;     
                   
   GLfloat aspect = (GLfloat)width / (GLfloat)height;
 
   glViewport(0, 0, width, height);

   device_width=(float)width;
   device_height=(float)height;

   RND.Update_projection(0, device_width, device_height,0);
   
   
 }



                 void LcOpenGLPainter::new_path()
                 {

                 }

                 void LcOpenGLPainter::close_path()
                 {
                       RND.Close_Loop();
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

                        RND.Jump();
                        RND.Add_Vertex(x,y);
                        pen_x=x;  pen_y=y;
                 }

                 void LcOpenGLPainter::line_to(double x, double y)
                 {
                        RND.Add_Vertex(x,y);
                        pen_x=x;  pen_y=y;
                 }

                 void LcOpenGLPainter::lineWidthCompensation(double lwc) // When set, we add this to the current linewidth, perhaps we should find a better method
                 {

                 }
                 void LcOpenGLPainter::line_width(double lineWidth)
                 {
                      RND.Select_Line_Width(lineWidth);
                 }

                 double LcOpenGLPainter::scale()
                 {
                      return RND.Get_Scale();
                 }

                 void LcOpenGLPainter::scale(double s)
                 {
                     RND.Update_scale(s);
                 }

                 void LcOpenGLPainter::rotate(double r)
                 {
                     RND.Update_rotate(r);
                 }

                 void LcOpenGLPainter::arc(double x, double y, double r, double start, double end)
                 {
                    RND.Jump();
                    
                      float delta=(std::abs(end-start));
                      float angle=0;
                      long points=curve_points;
                        
                        for(int i=0;i<points;i++)
                        {
                            angle=( ((float)i)/points)*(delta) + (start);
                            
                            RND.Add_Vertex( (x+r*cos(angle)) , (y+r*sin(angle)) );
                        }

                 }

                 void LcOpenGLPainter::arcNegative(double x, double y, double r, double start, double end)
                 {
                        RND.Jump();

                        float delta=(std::abs(end-start));
                        float angle=0;
                        long points=curve_points;

                        for(int i=0;i<points;i++)
                        {
                            angle=start - ( ((float)i)/points)*(delta) ;
                            
                            RND.Add_Vertex( (x+r*cos(angle)) , (y+r*sin(angle)) );
                        }
                        
                 }

                 void LcOpenGLPainter::circle(double x, double y, double r)
                 {
                        RND.Jump();

                        float angle=0;
                        for(int i=0;i<curve_points;i++)
                        {
                            angle=( ((float)i)/curve_points )*(2*PI);
                            RND.Add_Vertex( (x+r*cos(angle)) , (y+r*sin(angle)) );
                        }

                        RND.Close_Loop();
                       
                 }

                 void LcOpenGLPainter::ellipse(double cx, double cy, double rx, double ry, double sa, double ea, double ra)
                 {

                 }

                 void LcOpenGLPainter::rectangle(double x1, double y1, double w, double h)
                 { 
                        RND.Jump();
                        
                        RND.Add_Vertex(x1,y1);
                        RND.Add_Vertex(x1+w,y1);
                        RND.Add_Vertex(x1+w,y1+h);
                        RND.Add_Vertex(x1,y1+h);
                        
                        RND.Close_Loop();
                 }

                 void LcOpenGLPainter::stroke()
                 {
                      RND.Render();
                 }

                 void LcOpenGLPainter::source_rgb(double r, double g, double b)
                 {
                      RND.Select_Color(r,g,b,1.0f);
                 }

                 void LcOpenGLPainter::source_rgba(double r, double g, double b, double a)
                 {
                      RND.Select_Color(r,g,b,a);
                 }

                 void LcOpenGLPainter::translate(double x, double y)
                 {
                      RND.Update_translate(x,y);
                 }

                 void LcOpenGLPainter::user_to_device(double* x, double* y)
                 {
                    *y=-*y;
                    RND.User_To_Device(x,y);
                 }

                 void LcOpenGLPainter::device_to_user(double* x, double* y)
                 {
                    RND.Device_To_User(x,y);
                    *y=-*y;
                 }

                 void LcOpenGLPainter::user_to_device_distance(double* dx, double* dy)
                 {
                    *dy = -*dy;
                    RND.User_To_Device_Distance(dx,dy);
                 }

                 void LcOpenGLPainter::device_to_user_distance(double* dx, double* dy)
                 {
                    RND.Device_To_User_Distance(dx,dy);
                    *dy = -*dy;
                 }

                 void LcOpenGLPainter::font_size(double size, bool deviceCoords)
                 {
                      RND.Select_Font_Size(size,deviceCoords);
                 }

                 void LcOpenGLPainter::select_font_face(const char* text_val)
                 {
                      RND.Select_Font_Face(text_val);
                 }

                 void LcOpenGLPainter::text(const char* text_val)
                 {
                     RND.Add_New_Text_Entity();
                     RND.Select_Font_Value(text_val);
                 }

                 TextExtends LcOpenGLPainter::text_extends(const char* text_val)
                 {

                 }

                 void LcOpenGLPainter::quadratic_curve_to(double x1, double y1, double x2, double y2)
                 {
                     double x0=pen_x; double y0=pen_y;

                     long points=curve_points;
                     float Px,Py,t; 

                    for(int i=0;i<=points;i++)
                    {
                        t=((float)(i)/(float)(curve_points));
                        Px=(1-t)*(1-t)*x0 + 2*t*(1-t)*x1 + t*t*x2;
                        Py=(1-t)*(1-t)*y0 + 2*t*(1-t)*y1 + t*t*y2;
                        RND.Add_Vertex(Px,Py);
                    }

                    pen_x=Px; pen_y=Py;
                 }

                 void LcOpenGLPainter::curve_to(double x1, double y1, double x2, double y2, double x3, double y3)
                 {
                    double x0=pen_x; double y0=pen_y;

                     long points=curve_points;
                     float Px,Py,t; 

                    for(int i=0;i<=points;i++)
                    {
                        t=((float)(i)/(float)(curve_points));
                        Px=(1-t)*(1-t)*(1-t)*x0 + 3*t*(1-t)*(1-t)*x1  + 3*t*t*(1-t)*x2 + t*t*t*x3;
                        Py=(1-t)*(1-t)*(1-t)*y0 + 3*t*(1-t)*(1-t)*y1  + 3*t*t*(1-t)*y2 + t*t*t*y3;
                        RND.Add_Vertex(Px,Py);
                    }

                     pen_x=Px; pen_y=Py;
                 }

                 void LcOpenGLPainter::save()
                 {
                      RND.Save();
                 }

                 void LcOpenGLPainter::restore()
                 {
                      RND.Restore();

                 }

                 long LcOpenGLPainter::pattern_create_linear(double x1, double y1, double x2, double y2)
                 {
                      RND.Add_New_Gradient_Entity();
                      RND.Add_Linear_Gradient(x1,y1,x2,y2);
                      return 0;
                 }

                 void LcOpenGLPainter::pattern_add_color_stop_rgba(long pat, double offset, double r, double g, double b, double a)
                 {
                      RND.Add_Gradient_Color_Point(r,g,b,a);
                 }

                 void LcOpenGLPainter::set_pattern_source(long pat)
                 {

                 }

                 void LcOpenGLPainter::pattern_destroy(long pat)
                 {

                 }

                 void LcOpenGLPainter::fill()
                 {
                      RND.Select_Fill();
                 }

                 void LcOpenGLPainter::point(double x, double y, double size, bool deviceCoords)
                 {

                 }

                 void LcOpenGLPainter::reset_transformations()
                 {

                      RND.Reset_Transformations();

                 }

                 unsigned char* LcOpenGLPainter::data()
                 {

                 }

                 void LcOpenGLPainter::set_dash(const double* dashes, const int num_dashes, double offset, bool scaled)
                 {
                      RND.Select_Dashes(dashes,num_dashes,offset,scaled);
                 }

                 void LcOpenGLPainter::dash_destroy()
                 {
                      RND.Select_Dashes(NULL,0,0,false);
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
                    return CHE;
                 }

                 bool LcOpenGLPainter::isEntityCached(unsigned long id)
                 {
                    return CHE->isEntityCached(id);
                 }

                 void LcOpenGLPainter::renderEntityCached(unsigned long id)
                 {
                    LcCacherPainter* cp= dynamic_cast<LcCacherPainter*>(CHE);
                    GL_Pack* _gl_pack=((*cp)._cacher).Get_Entity_Cached_Pack(id);
                   // qDebug(">>>gl_pack>>> %u size>> %d",_gl_pack,_gl_pack->Pack_Size());
                    RND.Render_Cached_Pack(_gl_pack);
                 }

                 void LcOpenGLPainter::deleteEntityCached(unsigned long id)
                 {
                    CHE->deleteEntityCached(id);
                 }
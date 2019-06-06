
#include "openglpainter.h"
#include <QtDebug>

#include <QOpenGLContext>


LcOpenGLPainter::LcOpenGLPainter(unsigned int width, unsigned int height) 
{      
   
   if (height == 0) height = 1;                
   GLfloat aspect = (GLfloat)width / (GLfloat)height;
 
   glViewport(0, 0, width, height);

   device_width=(float)width;
   device_height=(float)height;

   RND.Update_projection(0, width, height, 0);
   

   qDebug( "=======================Opengl_Painter_Created=============%u w=%d h=%d",this,width,height);
    QOpenGLContext *CC= QOpenGLContext::currentContext();
     if(CC!=0)
    {
      ReadyShaderProgram();
    }
}

void LcOpenGLPainter::ReadyShaderProgram()
{
     RND.CreateShaderProgram();
    qDebug( "==RND.CreatingShader() call");
}


                 void LcOpenGLPainter::new_path()
                 {

                 }

                 void LcOpenGLPainter::close_path()
                 {
                       RND.Select_Render_Mode(GL_LINE_LOOP);
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
                         px=x; py=y;
                 }

                 void LcOpenGLPainter::line_to(double x, double y)
                 {
                        RND.Add_Vertex(px,py);   // line from pen(x,y) ----P(x,y)
                        RND.Add_Vertex(x,y);

                        move_to(x,y);            // setting new

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
                      return scale_factor;
                 }

                 void LcOpenGLPainter::scale(double s)
                 {
                     scale_factor*= s;
                     RND.Update_scale_mat(scale_factor);
                     qDebug("Opengl Painter Scale=%f",scale_factor);  
                 }

                 void LcOpenGLPainter::rotate(double r)
                 {
                     RND.Update_rotate_mat(-r);
                 }

                 void LcOpenGLPainter::arc(double x, double y, double r, double start, double end)
                 {

                 }

                 void LcOpenGLPainter::arcNegative(double x, double y, double r, double start, double end)
                 {

                 }

                 void LcOpenGLPainter::circle(double x, double y, double r)
                 {
                        float angle=0;
                        for(int i=0;i<curve_points;i++)
                        {
                            angle=( ((float)i)/curve_points )*(2*PI);
                            RND.Add_Vertex( (x+r*cos(angle)) , (y+r*sin(angle)) );
                        }
                        RND.Select_Render_Mode(GL_LINE_LOOP);

                 }

                 void LcOpenGLPainter::ellipse(double cx, double cy, double rx, double ry, double sa, double ea, double ra)
                 {

                 }

                 void LcOpenGLPainter::rectangle(double x1, double y1, double w, double h)
                 { 
                        move_to(x1,y1);
                        RND.Add_Vertex(px,py);
                        RND.Add_Vertex(px+w,py);
                        RND.Add_Vertex(px+w,py+h);
                        RND.Add_Vertex(px,py+h);

                        RND.Select_Render_Mode(GL_QUADS);

                        // pen coordinates remains same
                 }

                 void LcOpenGLPainter::stroke()
                 {
                      RND.Draw();
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
                      pan_x +=(x* scale_factor);
                      pan_y -=(y* scale_factor);    // TODO: should be pan_y +=(y) temporary
                      RND.Update_translate_mat(pan_x  ,pan_y );
                      qDebug("OpenGL painter PanX=%f PanY=%f",pan_x,pan_y);
                 }

                 void LcOpenGLPainter::user_to_device(double* x, double* y)
                 {
                    //*y=-*y;
                    RND.User_To_Device(x,y);
                 }

                 void LcOpenGLPainter::device_to_user(double* x, double* y)
                 {
                    RND.Device_To_User(x,y);
                   // *y=-*y;
                 }

                 void LcOpenGLPainter::user_to_device_distance(double* dx, double* dy)
                 {
                    RND.User_To_Device_Distance(dx,dy);
                 }

                 void LcOpenGLPainter::device_to_user_distance(double* dx, double* dy)
                 {
                    RND.Device_To_User_Distance(dx,dy);
                 }

                 void LcOpenGLPainter::font_size(double size, bool deviceCoords)
                 {
                      font_size_value=size;
                 }

                 void LcOpenGLPainter::select_font_face(const char* text_val)
                 {

                 }

                 void LcOpenGLPainter::text(const char* text_val)
                 {
                     int c=0;
                     while(text_val[c]!='\0')
                     {
                        rectangle(px,py,font_size_value,font_size_value);
                        //TODO: Temporary rectangles..Later to render the glyphs
                        c++;
                        move_to(font_size_value*c,0.0f);
                     }
                 }

                 TextExtends LcOpenGLPainter::text_extends(const char* text_val)
                 {

                 }

                 void LcOpenGLPainter::quadratic_curve_to(double x1, double y1, double x2, double y2)
                 {

                 }

                 void LcOpenGLPainter::curve_to(double x1, double y1, double x2, double y2, double x3, double y3)
                 {

                 }

                 void LcOpenGLPainter::save()
                 {
                      RND.Save();
                 }

                 void LcOpenGLPainter::restore()
                 {
                      RND.Restore();

                      qDebug("panx=%f pany=%f",RND.Get_Translate_X(),RND.Get_Translate_Y());
                      pan_x=RND.Get_Translate_X();
                      pan_y=RND.Get_Translate_Y();
                      scale_factor=RND.Get_Scale();
                 }

                 long LcOpenGLPainter::pattern_create_linear(double x1, double y1, double x2, double y2)
                 {

                 }

                 void LcOpenGLPainter::pattern_add_color_stop_rgba(long pat, double offset, double r, double g, double b, double a)
                 {

                 }

                 void LcOpenGLPainter::set_pattern_source(long pat)
                 {

                 }

                 void LcOpenGLPainter::pattern_destroy(long pat)
                 {

                 }

                 void LcOpenGLPainter::fill()
                 {
                      RND.Select_Fill(GL_FILL);
                      RND.Select_Render_Mode(GL_POLYGON);
                 }

                 void LcOpenGLPainter::point(double x, double y, double size, bool deviceCoords)
                 {

                 }

                 void LcOpenGLPainter::reset_transformations()
                 {
                      pan_x =0.0f;
                      pan_y =0.0f;
                      scale_factor= 1.0f;

                      RND.Reset_Transformations();

                 }

                 unsigned char* LcOpenGLPainter::data()
                 {

                 }

                 void LcOpenGLPainter::set_dash(const double* dashes, const int num_dashes, double offset, bool scaled)
                 {

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

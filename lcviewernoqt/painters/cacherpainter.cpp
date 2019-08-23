#include "cacherpainter.h"

LcCacherPainter::LcCacherPainter() 
{      
   
}

void LcCacherPainter::create_resources()
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
	_cacher.closeLoop();
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
	_cacher.jump();
	_cacher.addVertex(x,y);
	_pen_x=x;  _pen_y=y;
}

void LcCacherPainter::line_to(double x, double y)
{
	_cacher.addVertex(x,y);
	_pen_x=x;  _pen_y=y;
}

void LcCacherPainter::lineWidthCompensation(double lwc) // When set, we add this to the current linewidth, perhaps we should find a better method
{

}

void LcCacherPainter::line_width(double lineWidth)
{
	_cacher.selectLineWidth(lineWidth);
}

double LcCacherPainter::scale()
{
	return _cacher.getScale();
}

void LcCacherPainter::scale(double s)
{
	_cacher.updateScale(s);
}

void LcCacherPainter::rotate(double r)
{
	_cacher.updateRotate(r);
}

void LcCacherPainter::arc(double x, double y, double r, double start, double end)
{    
	if(start<end)
		end-=2*PI;

	_cacher.jump();

	float delta=(std::abs(end-start));
	float angle=0;
	long points=CURVE_POINTS;

	for(int i=0;i<=points;i++)
	{
		angle=start - ( ((float)i)/points)*(delta) ;
		_cacher.addVertex( (x+r*cos(angle)) , (y+r*sin(angle)) );
	}
}

void LcCacherPainter::arcNegative(double x, double y, double r, double start, double end)
{     
	if(start>end)
		end+=2*PI;

	_cacher.jump();
					
	float delta=(std::abs(end-start));
	float angle=0;
	long points=CURVE_POINTS;
						
	for(int i=0;i<=points;i++)
	{
		angle=( ((float)i)/points)*(delta) + (start);
		_cacher.addVertex( (x+r*cos(angle)) , (y+r*sin(angle)) );
	}
}

void LcCacherPainter::circle(double x, double y, double r)
{
	_cacher.jump();

	float angle=0;
	for(int i=0;i<CURVE_POINTS;i++)
	{
		angle=( ((float)i)/CURVE_POINTS )*(2*PI);
		_cacher.addVertex( (x+r*cos(angle)) , (y+r*sin(angle)) );
	}
	_cacher.closeLoop();
}

void LcCacherPainter::ellipse(double cx, double cy, double rx, double ry, double sa, double ea, double ra)
{
	_cacher.jump();
	float SA= atan( (rx/ry)* tan(sa) );   //Finding eccentric angles
	float EA= atan( (rx/ry)* tan(ea) );

	if(sa>(PI/2) && sa<=(3*(PI/2)) )      //2nd-3rd Quadrant
		SA+=PI;

	if(sa>(3*(PI/2)) && sa<=(2*PI) )  // 4th Quadrant
		SA+=2*PI;   

	if(ea>(PI/2) && ea<=(3*(PI/2)) )       //2nd-3rd Quadrant
		EA+=PI;

	if(ea>(3*(PI/2)) && ea<=(2*PI) )   // 4th Quadrant
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

		_cacher.addVertex( Tx , Ty );
							
	}
}

void LcCacherPainter::rectangle(double x1, double y1, double w, double h)
{ 
	_cacher.jump();
						
	_cacher.addVertex(x1,y1);
	_cacher.addVertex(x1+w,y1);
	_cacher.addVertex(x1+w,y1+h);
	_cacher.addVertex(x1,y1+h);
						
	_cacher.closeLoop();
}

void LcCacherPainter::stroke()
{
	_cacher.readyForNextEntity();
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
	_cacher.updateTranslate(x,y );
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
	_cacher.selectFontSize(size,deviceCoords);
}

void LcCacherPainter::select_font_face(const char* text_val)
{
	_cacher.selectFontFace(text_val);
}

void LcCacherPainter::text(const char* text_val)
{
	_cacher.setNewTextEntity();
	_cacher.selectFontValue(text_val);
}

TextExtends LcCacherPainter::text_extends(const char* text_val)
{
	TextExtends te;
	GL_Text_Extend extents=_cacher.getTextExtend(text_val);
		
	te.height = extents.height;
	te.width = extents.width;
	te.x_advance = extents.x_advance;
	te.x_bearing = extents.x_bearing;
	te.y_advance = extents.y_advance;
	te.y_bearing = extents.y_bearing;

	return te;
}

void LcCacherPainter::quadratic_curve_to(double x1, double y1, double x2, double y2)
{
	double x0=_pen_x; double y0=_pen_y;

	long points=CURVE_POINTS;
	float Px,Py,t; 

	for(int i=0;i<=points;i++)
	{
		t=((float)(i)/(float)(CURVE_POINTS));
		Px=(1-t)*(1-t)*x0 + 2*t*(1-t)*x1 + t*t*x2;
		Py=(1-t)*(1-t)*y0 + 2*t*(1-t)*y1 + t*t*y2;
		_cacher.addVertex(Px,Py);
	}

		_pen_x=Px; _pen_y=Py;
}

void LcCacherPainter::curve_to(double x1, double y1, double x2, double y2, double x3, double y3)
{
	double x0=_pen_x; double y0=_pen_y;

	long points=CURVE_POINTS;
	float Px,Py,t; 

	for(int i=0;i<=points;i++)
	{
		t=((float)(i)/(float)(CURVE_POINTS));
		Px=(1-t)*(1-t)*(1-t)*x0 + 3*t*(1-t)*(1-t)*x1  + 3*t*t*(1-t)*x2 + t*t*t*x3;
		Py=(1-t)*(1-t)*(1-t)*y0 + 3*t*(1-t)*(1-t)*y1  + 3*t*t*(1-t)*y2 + t*t*t*y3;
		_cacher.addVertex(Px,Py);
	}

	_pen_x=Px; _pen_y=Py;
}

void LcCacherPainter::save()
{
					 
}

void LcCacherPainter::restore()
{
					 
}

long LcCacherPainter::pattern_create_linear(double x1, double y1, double x2, double y2)
{
	_cacher.setNewGradientEntity();
	_cacher.addLinearGradient(x1,y1,x2,y2);
	return 0;
}

void LcCacherPainter::pattern_add_color_stop_rgba(long pat, double offset, double r, double g, double b, double a)
{
	_cacher.addGradientColorPoint(r,g,b,a);
}

void LcCacherPainter::set_pattern_source(long pat)
{

}

void LcCacherPainter::pattern_destroy(long pat)
{

}

void LcCacherPainter::fill()
{
	_cacher.selectFill();
}

void LcCacherPainter::point(double x, double y, double size, bool deviceCoords)
{

}

void LcCacherPainter::reset_transformations()
{

}

unsigned char* LcCacherPainter::data()
{
	return NULL;
}

void LcCacherPainter::set_dash(const double* dashes, const int num_dashes, double offset, bool scaled)
{
	_cacher.selectDashes(dashes,num_dashes,offset,scaled);
}

void LcCacherPainter::dash_destroy()
{
	_cacher.selectDashes(NULL,0,0,false);
}

long LcCacherPainter::image_create(const std::string& file)
{
	return 0;
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
	_cacher.readyFreshPack();
}

void LcCacherPainter::finishcaching(unsigned long id)
{
	_cacher.savePack(id);
}

LcPainter* LcCacherPainter::getCacherpainter()
{
	// NO Need ( main painter uses)
	return NULL;
}

bool LcCacherPainter::isEntityCached(unsigned long id)
{
	return _cacher.isPackCached(id);
}

void LcCacherPainter::renderEntityCached(unsigned long id)
{
	// No Need ( cant do rendering here)
}

void LcCacherPainter::deleteEntityCached(unsigned long id)
{
	_cacher.erasePack(id);
}

#include "openglpainter.h"

OpenglPainter::OpenglPainter()
{
}

void OpenglPainter::set_manager(Manager* manager)
{
    _manager=manager;
}

void OpenglPainter::move_to(double x, double y)
{
    if(abs(_pen_x-x)>BBHEURISTIC2 || abs(_pen_y-y)>BBHEURISTIC2 || _manager->isNew()) {
        _manager->jump();
        _manager->addVertex(x,y);
        _pen_x=x;
        _pen_y=y;
    }
}

void OpenglPainter::point(double x, double y, double size, bool deviceCoords)
{
    if (deviceCoords) {
        double x = size;
        double y = size;

        double zeroCornerX = 0.;
        double zeroCornerY = 0.;

        device_to_user(&zeroCornerX, &zeroCornerY);
        device_to_user(&x, &y);

        size = (x - zeroCornerX);
    }
    circle(x,y,size);
}

void OpenglPainter::line_to(double x, double y)
{
    _manager->addVertex(x,y);
    _pen_x=x;
    _pen_y=y;
}

void OpenglPainter::arc(double x, double y, double r, double start, double end)
{
    if(start<end)
        end-=2*PI;

    //_manager->jump();

    float delta=(std::abs(end-start));
    float angle=0;
    long points=CURVE_POINTS;

    for(int i=0; i<=points; i++)
    {
        angle=start - ( ((float)i)/points)*(delta) ;

        _manager->addVertex( (_pen_x=x+r*cos(angle)), (_pen_y=y+r*sin(angle)) );
    }
}

void OpenglPainter::arcNegative(double x, double y, double r, double start, double end)
{
    if(start>end)
        end+=2*PI;

    //_manager->jump();

    float delta=(std::abs(end-start));
    float angle=0;
    long points=CURVE_POINTS;

    for(int i=0; i<=points; i++)
    {
        angle=( ((float)i)/points)*(delta) + (start);
        _manager->addVertex( (_pen_x=x+r*cos(angle)), (_pen_y=y+r*sin(angle)) );
    }
}

void OpenglPainter::circle(double x, double y, double r)
{
    _manager->jump();

    float angle=0;
    for(int i=0; i<CURVE_POINTS; i++)
    {
        angle=( ((float)i)/CURVE_POINTS )*(2*PI);
        _manager->addVertex( (x+r*cos(angle)), (y+r*sin(angle)) );
    }
    _manager->closeLoop();
}

void OpenglPainter::ellipse(double cx, double cy, double rx, double ry, double sa, double ea, double ra)
{
    _manager->jump();

    float Esa= atan( (rx/ry)* tan(sa) );   //Finding eccentric angles
    float Eea= atan( (rx/ry)* tan(ea) );

    //Extending to general term
    Esa += ceil((sa-M_PI/2)/M_PI)*M_PI;
    Eea += ceil((ea-M_PI/2)/M_PI)*M_PI;

    if (Eea<=Esa)
        Eea += 2*M_PI;

    float delta=(std::abs(Eea-Esa));

    float EA=0;
    long points=CURVE_POINTS;
    float tx,ty,TX,TY;

    for(int i=0; i<=points; i++)
    {
        EA=( ((float)i)/points)*(delta) + (Esa);

        tx=rx*cos(EA);                       // parametric equation
        ty=ry*sin(EA);

        TX=(tx*cos(ra) - ty*sin(ra)) + cx;  // first rotate then shift origin
        TY=(tx*sin(ra) + ty*cos(ra)) + cy;

        _manager->addVertex( TX, TY );
    }
}

void OpenglPainter::rectangle(double x1, double y1, double w, double h)
{
    _manager->jump();

    _manager->addVertex(x1,y1);
    _manager->addVertex(x1+w,y1);
    _manager->addVertex(x1+w,y1+h);
    _manager->addVertex(x1,y1+h);

    _manager->closeLoop();
}

void OpenglPainter::quadratic_curve_to(double x1, double y1, double x2, double y2)
{
    double x0=_pen_x;
    double y0=_pen_y;

    long points=CURVE_POINTS;
    float Px,Py,t;

    for(int i=0; i<=points; i++)
    {
        t=((float)(i)/(float)(CURVE_POINTS));
        Px=(1-t)*(1-t)*x0 + 2*t*(1-t)*x1 + t*t*x2;
        Py=(1-t)*(1-t)*y0 + 2*t*(1-t)*y1 + t*t*y2;
        _manager->addVertex(Px,Py);
    }

    _pen_x=Px;
    _pen_y=Py;
}

void OpenglPainter::curve_to(double x1, double y1, double x2, double y2, double x3, double y3)
{
    double x0=_pen_x;
    double y0=_pen_y;

    long points=CURVE_POINTS;
    float Px,Py,t;

    for(int i=0; i<=points; i++)
    {
        t=((float)(i)/(float)(CURVE_POINTS));
        Px=(1-t)*(1-t)*(1-t)*x0 + 3*t*(1-t)*(1-t)*x1  + 3*t*t*(1-t)*x2 + t*t*t*x3;
        Py=(1-t)*(1-t)*(1-t)*y0 + 3*t*(1-t)*(1-t)*y1  + 3*t*t*(1-t)*y2 + t*t*t*y3;
        _manager->addVertex(Px,Py);
    }

    _pen_x=Px;
    _pen_y=Py;
}

void OpenglPainter::close_path()
{
    _manager->closeLoop();
}

void OpenglPainter::fill()
{
    _manager->selectFill();
    stroke();
}

void OpenglPainter::new_path()
{
}

void OpenglPainter::new_sub_path()
{
}

void OpenglPainter::lineWidthCompensation(double lwc) // When set, we add this to the current linewidth, perhaps we should find a better method
{
}

void OpenglPainter::line_width(double lineWidth)
{
    _manager->selectLineWidth(lineWidth);
}

void OpenglPainter::font_size(double size, bool deviceCoords)
{
    _manager->selectFontSize(size,deviceCoords);
}

void OpenglPainter::select_font_face(const char* text_val, const char* font_type)
{
    if ( !strcmp(font_type, "BOLD")) {
        _manager->selectFontFace(text_val, Font_Book::FontType::BOLD);
    }
    else if ( !strcmp(font_type, "ITALIC")) {
        _manager->selectFontFace(text_val, Font_Book::FontType::ITALIC);
    }
    else if ( !strcmp(font_type,"BOLD_ITALIC")) {
        _manager->selectFontFace(text_val, Font_Book::FontType::BOLD_ITALIC);
    }
    else {
        _manager->selectFontFace(text_val, Font_Book::FontType::REGULAR);
    }
}

void OpenglPainter::set_dash(const double* dashes, const int num_dashes, double offset, bool scaled)
{
    _manager->selectDashes(dashes,num_dashes,offset,scaled);
}

void OpenglPainter::dash_destroy()
{
    _manager->selectDashes(NULL,0,0,false);
}

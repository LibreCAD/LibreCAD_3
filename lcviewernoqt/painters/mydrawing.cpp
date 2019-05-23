#include "mydrawing.h"
#include <QtDebug>

using namespace lc::viewer;



void linecheck(LcPainter& painter)
{
   painter.line_width(2);
   painter.source_rgba(1,0,0,1);
   painter.move_to(0,0);
   painter.line_to(40,40);
   painter.line_to(40,0);
   painter.line_to(80,40);
   painter.line_to(80,-10);
  
   painter.stroke();
}

void closedcheck(LcPainter& painter)
{
   painter.line_width(2);
   painter.source_rgba(1,0,0,1);
   painter.move_to(0,-50);
   painter.line_to(40,-20);
   painter.line_to(40,-50);
   painter.line_to(80,-20);
   painter.line_to(80,-70);
   painter.close_path();
   
   painter.stroke();
}

void closedfillcheck(LcPainter& painter)
{
   painter.line_width(2);
   painter.source_rgba(1,0,0,1);
   painter.move_to(0,70);
   painter.line_to(40,90);
   painter.line_to(40,70);
   painter.line_to(80,90);
   painter.line_to(80,60);
   painter.close_path();
   painter.fill();
   painter.stroke();
}

void rectanglecheck(LcPainter& painter)
{
	painter.source_rgba(1,0,1,1);
	painter.rectangle(-80,-80,30,40);
    painter.fill();
	painter.stroke();
}

void circlecheck(LcPainter& painter)
{   
	painter.line_width(3);
	painter.source_rgba(0,0,1,1);
	painter.circle(-50,50,30);
	//painter.fill();
  painter.stroke();
}

void gridcheck(LcPainter& painter)
{
	
	for(float i=-1;i<1;i=i+205)
	{
		painter.move_to(i,1);
		painter.line_to(i,-1);
	}
 painter.source_rgba(1,0,1,1);
	painter.stroke();

	painter.source_rgba(1,1,1,1);
	for(float i=-1;i<1;i=i+205)
	{
		painter.move_to(1,i);
		painter.line_to(-1,i);
	}

	painter.stroke();
}

void gradient_background(LcPainter& painter)
{
   
  //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   //  painter.source_rgba(1,0,1,1);
  //glColor3f(1.0f,1.0f,00f);
  /*     glBegin(GL_POLYGON);
              // glColor3f(0,3092,0);
               glVertex2f(-50f, 50f); // vertex 1
              // glColor3f(0,3092,0);
               glVertex2f(50f, 50f); // vertex 2
               
              //  glColor3f(1033,5045,1033);
               glVertex2f(50f, -50f); // vertex 3
              // glColor3f(1033,5045,1033);
               glVertex2f(-50f, -50f); // vertex 4
       glEnd();
       */
}


void drawingpage(LcPainter& painter)
{  
   qDebug( "drawingpage draw()");
	
    linecheck(painter);

   rectanglecheck(painter);

    circlecheck(painter);

    closedcheck(painter);

     closedfillcheck(painter);
    

}


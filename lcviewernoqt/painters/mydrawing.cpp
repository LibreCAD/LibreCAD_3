#include "mydrawing.h"


using namespace lc::viewer;



void linecheck(LcPainter& painter)
{
   painter.line_width(2);
   painter.source_rgba(1,0,0,1);
   painter.move_to(0,0);
   painter.line_to(0.4,0.4);
   painter.line_to(0.4,0);
   painter.line_to(0.8,0.4);
   painter.line_to(0.8,-0.1);
  
   painter.stroke();
}

void closedcheck(LcPainter& painter)
{
   painter.line_width(2);
   painter.source_rgba(1,0,0,1);
   painter.move_to(0,-0.5);
   painter.line_to(0.4,-0.2);
   painter.line_to(0.4,-0.5);
   painter.line_to(0.8,-0.2);
   painter.line_to(0.8,-0.7);
   painter.close_path();
   
   painter.stroke();
}

void closedfillcheck(LcPainter& painter)
{
   painter.line_width(2);
   painter.source_rgba(1,0,0,1);
   painter.move_to(0,0.7);
   painter.line_to(0.4,0.9);
   painter.line_to(0.4,0.7);
   painter.line_to(0.8,0.9);
   painter.line_to(0.8,0.6);
   painter.close_path();
   painter.fill();
   painter.stroke();
}

void rectanglecheck(LcPainter& painter)
{
	painter.source_rgba(1,0,1,1);
	painter.rectangle(-0.8,-0.8,0.3,0.4);
    painter.fill();
	painter.stroke();
}

void circlecheck(LcPainter& painter)
{   
	painter.line_width(3);
	painter.source_rgba(0,0,1,1);
	painter.circle(-0.5,0.5,0.3);
	//painter.fill();
  painter.stroke();
}

void gridcheck(LcPainter& painter)
{
	painter.source_rgba(1,1,1,1);
	for(float i=-1;i<1;i=i+0.25)
	{
		painter.move_to(i,1);
		painter.line_to(i,-1);
	}

	painter.stroke();

	painter.source_rgba(1,1,1,1);
	for(float i=-1;i<1;i=i+0.25)
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
  //glColor3f(1.0f,1.0f,0.0f);
  /*     glBegin(GL_POLYGON);
              // glColor3f(0,0.392,0);
               glVertex2f(-0.5f, 0.5f); // vertex 1
              // glColor3f(0,0.392,0);
               glVertex2f(0.5f, 0.5f); // vertex 2
               
              //  glColor3f(0.133,0.545,0.133);
               glVertex2f(0.5f, -0.5f); // vertex 3
              // glColor3f(0.133,0.545,0.133);
               glVertex2f(-0.5f, -0.5f); // vertex 4
       glEnd();
       */
}


void drawingpage(LcPainter& painter)
{  
	painter.clear(0,0.52,0);
  //painter.setup();

  //gradient_background();

	 gridcheck(painter);
   
    linecheck(painter);

    rectanglecheck(painter);

    circlecheck(painter);

    closedcheck(painter);

     closedfillcheck(painter);
    

}


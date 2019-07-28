#shader vertex

#version 330 core
layout (location = 0) in vec3 pos; 
layout (location = 1) in float prev_distance; 
uniform mat4 u_MVP;


void main()
{
    gl_Position = u_MVP *vec4(pos, 1.0); 
    
}

//############################################################
//#############################################################

#shader geometry
#version 330 core
layout (lines_adjacency) in;
layout (triangle_strip, max_vertices = 4) out;


uniform float   u_W=1.0f;      
uniform float   MITER_LIMIT=0.75f;    
uniform vec2    WIN_SCALE;  


vec2 screen_space(vec4 vertex)
{
    return vec2( vertex.xy /vertex.w ) * (WIN_SCALE/2.0f);
}


void main(void)
{
 float THICKNESS=(u_W)/2.0f;
   
  // get the four vertices passed to the shader:
  vec2 p0 = screen_space(gl_in[0].gl_Position );   // start of previous segment
  vec2 p1 = screen_space(gl_in[1].gl_Position);   // end of previous segment, start of current segment
  vec2 p2 = screen_space( gl_in[2].gl_Position );   // end of current segment, start of next segment
  vec2 p3 = screen_space( gl_in[3].gl_Position);   // end of next segment

  
  // determine the direction of each of the 3 segments (previous, current, next)
  vec2 v0 = normalize(p1-p0);
  vec2 v1 = normalize(p2-p1);
  vec2 v2 = normalize(p3-p2);

  // determine the normal of each of the 3 segments (previous, current, next)
  vec2 n0 = vec2(-v0.y, v0.x);
  vec2 n1 = vec2(-v1.y, v1.x);
  vec2 n2 = vec2(-v2.y, v2.x);

  

  // determine miter lines by averaging the normals of the 2 segments
  vec2 miter_a = normalize(n0 + n1);    // miter at start of current segment
  vec2 miter_b = normalize(n1 + n2);    // miter at end of current segment

  // determine the length of the miter by projecting it onto normal and then inverse it
  float length_a = THICKNESS / dot(miter_a, n1);
  float length_b = THICKNESS / dot(miter_b, n1);
  
  //--------------------------------

  // prevent excessively long miters at sharp corners
  if(( dot(v0,v1) < -MITER_LIMIT ) )
   {
    miter_a = n1;
    length_a = THICKNESS;  
  }

  if(( dot(v1,v2) < -MITER_LIMIT ) )
  {
    miter_b = n1;
    length_b = THICKNESS;
  }
  //------------------

  
  // generate the triangle strip
 
gl_Position = vec4( 2.0f*(p1 + length_a * miter_a) / WIN_SCALE, 0.0, 1.0 );
  EmitVertex();
  

gl_Position = vec4(  2.0f*(p1 - length_a * miter_a) / WIN_SCALE, 0.0, 1.0 );
  EmitVertex();
  
 
gl_Position = vec4(  2.0f*(p2 + length_b * miter_b) / WIN_SCALE, 0.0, 1.0 );
  EmitVertex();
  
  
gl_Position = vec4(  2.0f*(p2 - length_b * miter_b) / WIN_SCALE, 0.0, 1.0 );
  EmitVertex();

  EndPrimitive();
}
//############################################################
#shader fragment

#version 330 core

out vec4 out_Color;
uniform vec4 u_Color;

void main() 
{ 

   out_Color = u_Color;
 
   
} 

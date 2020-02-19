#shader vertex

#version 140
in vec3 pos;
in float prev_distance;
in vec4 col;

out vec4 eachcol;

uniform mat4 u_MVP;

void main() 
{
  gl_Position = u_MVP * vec4(pos, 1);
  eachcol=col;
} 

#shader fragment

#version 140

out vec4 out_Color;
in vec4 eachcol;

void main() 
{
  out_Color = eachcol;
} 

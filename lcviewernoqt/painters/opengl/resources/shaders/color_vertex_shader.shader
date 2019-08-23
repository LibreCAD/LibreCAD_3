#shader vertex

#version 330 core
layout (location = 0) in vec3 pos;
layout (location = 1) in float prev_distance; 
layout (location = 2) in vec4 col;

out vec4 eachcol;

uniform mat4 u_MVP;

void main() 
{
  gl_Position = u_MVP * vec4(pos, 1);
  eachcol=col;
} 

#shader fragment

#version 330 core

out vec4 out_Color;
in vec4 eachcol;

void main() 
{
  out_Color = eachcol;
} 

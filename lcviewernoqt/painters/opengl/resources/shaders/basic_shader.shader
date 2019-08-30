#shader vertex

#version 330 core
layout (location = 0) in vec3 pos; 
layout (location = 1) in float prev_distance; 
uniform mat4 u_MVP;

void main()
{
  gl_Position = u_MVP *vec4(pos, 1.0); 
}

//-------------------------------------------
#shader fragment

#version 330 core

uniform vec4 u_Color;
out vec4 out_Color;

void main() 
{
  out_Color = u_Color;
} 


#shader vertex

#version 130
in vec3 pos;

uniform mat4 u_MVP;
void main() 
{
   gl_Position = u_MVP *vec4(pos, 1);
} 

#shader fragment

#version 130

uniform vec4 u_Color;

void main() 
{
    gl_FragColor = u_Color;
} 

#shader vertex
#version 140

in vec3 pos;
in float prev_distance;
uniform mat4 u_MVP;

void main()
{
  gl_Position = u_MVP *vec4(pos, 1.0); 
}

//-------------------------------------------
#shader fragment
#version 140

uniform vec4 u_Color;
out vec4 out_Color;

void main() 
{
  out_Color = u_Color;
} 


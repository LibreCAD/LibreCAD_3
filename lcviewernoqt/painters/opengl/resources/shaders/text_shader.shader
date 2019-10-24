#shader vertex

#version 330 core
layout (location = 0) in vec3 pos; 
layout (location = 1) in vec2 texCoord; 

uniform mat4 u_MVP;
out vec2 v_TexCoord;

void main() 
{
  gl_Position = u_MVP * vec4(pos, 1);
  v_TexCoord=texCoord;
} 

//-------------------------------------------
#shader fragment

#version 330 core

in vec2 v_TexCoord;

uniform vec4 u_Color;
uniform sampler2D u_Texture;

out vec4 out_Color;

void main() 
{
  vec4 sampled = vec4(1.0, 1.0, 1.0, texture(u_Texture,v_TexCoord).r);
  out_Color = u_Color * sampled;
}

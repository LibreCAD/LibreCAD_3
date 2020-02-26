#shader vertex

#version 140
in vec3 pos;
in vec2 texCoord;

uniform mat4 u_MVP;
out vec2 v_TexCoord;

void main() 
{
  gl_Position = u_MVP * vec4(pos, 1);
  v_TexCoord=texCoord;
} 

//-------------------------------------------
#shader fragment

#version 140

in vec2 v_TexCoord;

uniform vec4 u_Color;
uniform sampler2D u_Texture;

out vec4 out_Color;

void main() 
{
  vec4 sampled = vec4(1.0, 1.0, 1.0, texture(u_Texture,v_TexCoord).r);
  out_Color = u_Color * sampled;
}

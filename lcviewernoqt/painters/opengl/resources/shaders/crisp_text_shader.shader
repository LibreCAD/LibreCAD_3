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

const float width=0.55;
const float edge=0.01;

void main() 
{
  float distance = 1.0 - texture2D(u_Texture,v_TexCoord).r;

  float alpha= 1.0 - smoothstep(width, width + edge , distance);

  out_Color = vec4(u_Color.rgb ,  alpha);
}

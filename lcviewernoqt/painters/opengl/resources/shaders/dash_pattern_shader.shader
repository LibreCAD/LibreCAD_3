#shader vertex

#version 330 core
layout (location = 0) in vec3 pos; 
layout (location = 1) in float prev_distance; 
uniform mat4 u_MVP;
uniform mat4 u_X;

out VS_OUT 
{
   vec4 PD;
} vs_out;


void main()
{
    gl_Position = u_MVP *vec4(pos, 1.0); 
    
     vec4 temp_d= u_X *vec4(prev_distance,0.0,0.0,1.0);
     vs_out.PD=temp_d;
}

//############################################################

#shader geometry
#version 330 core
layout (lines_adjacency) in;
layout (triangle_strip, max_vertices = 4) out;

in VS_OUT {
    vec4 PD;
} gs_in[]; 

out vec2 TXC;
flat out float PL;
flat out float L;

uniform float   u_W;      
uniform float   MITER_LIMIT=0.75f;    
uniform vec2    WIN_SCALE= vec2(500,500);  
uniform mat4    u_MVP;

vec2 screen_space(vec4 vertex)
{
    return vec2( vertex.xy /vertex.w ) * (WIN_SCALE/2.0f);
}

vec2 rotate(vec2 v, vec2 ref)
{
   float s= ref.y;
   float c = ref.x;
   mat2 m = mat2(c,-s,s,c);
   return m*v;
}

void main(void)
{
  float THICKNESS=u_W/2.0f;
   
  // get the four vertices passed to the shader:
 
  vec2 p0 = screen_space(gl_in[0].gl_Position );   
  // start of previous segment
  
  vec2 p1 = screen_space(gl_in[1].gl_Position);   
  // end of previous segment, start of current segment
  
  vec2 p2 = screen_space( gl_in[2].gl_Position );   
  // end of current segment, start of next segment
  
  vec2 p3 = screen_space( gl_in[3].gl_Position);   
  // end of next segment

  vec2 pdv = screen_space(gs_in[1].PD);
  PL=length(pdv);

   L=length(p2-p1);
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

  vec2 mra=rotate(miter_a,v1);
  vec2 mrb=rotate(miter_b,v1);
  
  // generate the triangle strip
 
  TXC=vec2( (length_a*mra).x,THICKNESS);
  gl_Position = vec4( 2.0f*(p1 + length_a * miter_a) / WIN_SCALE, 0.0, 1.0 );
  EmitVertex();
  

  TXC=vec2(-(length_a*mra).x,-THICKNESS);
  gl_Position = vec4( 2.0f*(p1 - length_a * miter_a) / WIN_SCALE, 0.0, 1.0 );
  EmitVertex();
  
 
  TXC=vec2(L+(length_b*mrb).x,THICKNESS);
  gl_Position = vec4( 2.0f*(p2 + length_b * miter_b) / WIN_SCALE, 0.0, 1.0 );
  EmitVertex();
  
  
  TXC=vec2(L-(length_b*mrb).x,-THICKNESS);
  gl_Position = vec4( 2.0f*(p2 - length_b * miter_b) / WIN_SCALE, 0.0, 1.0 );
  EmitVertex();

  EndPrimitive();
}
//############################################################

#shader fragment

#version 330 core

uniform vec4 u_Color;
out vec4 out_Color;

in vec2 TXC;
flat in float PL;
flat in float L;

uniform float  dashes[64];       // dash-gap data (in pixel)
uniform int dashes_size;         // size of dash gap data
uniform float dashes_sum;        // total sum of all dash-gap(in pixel)

void main() 
{
    
    float u=TXC.x;              // u or x coordinate of current fragment
    float TD= u + PL;           // total distance from start point(in pixel)

    float R = fract(TD / (dashes_sum)); // ratio of length of current  
                                        //  fragment to total dash-gap length


   //--------------------- deciding in dash or gap-------
            
    float r=0; float s=0;
    for(int i=0;i<dashes_size;i++)
    {
      s+=dashes[i];

      r=s/dashes_sum;

      if(R<=r)          // current fragment belong to this
      {
        if(i%2!=0)          // this is gap
        {
          discard;
        }
          break;
      }
    }
               
    //---------------------------------------------------- 
      out_Color = u_Color;
} 

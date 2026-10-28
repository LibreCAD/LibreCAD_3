#version 440
layout(location = 0) in vec4 vColor;
layout(location = 1) in vec2 vUv;
layout(binding = 1) uniform sampler2D atlas;
layout(location = 0) out vec4 fragColor;
void main() { fragColor = vColor * texture(atlas, vUv); }

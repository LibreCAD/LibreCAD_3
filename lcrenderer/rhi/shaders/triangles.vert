#version 440
layout(location = 0) in vec2 position;
layout(location = 1) in vec4 color;
layout(location = 0) out vec4 vColor;
layout(std140, binding = 0) uniform Transform { mat4 mvp; };
void main() {
    gl_Position = mvp * vec4(position, 0.0, 1.0);
    vColor = color;
}

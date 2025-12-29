#version 330 core
layout(location = 0) in vec2 position;
layout(location = 1) in float radius;
layout(location = 2) in vec4 color;
layout(location = 3) in uint depth;
out vec4 fragColor;
flat out uint fragDepth;
void main() {
    fragColor = color;
    fragDepth = depth;
    gl_Position = vec4(position, 0.0, 1.0);
    gl_PointSize = radius;
}


#version 330 core
layout(location = 0) in vec2 position;
layout(location = 1) in float radius;
layout(location = 2) in vec4 color;
layout(location = 3) in float depth;

uniform mat4 projection;
uniform mat4 view;

out vec4 fragColor;

void main() {
    fragColor = color;
    gl_Position = projection * view * vec4(position, depth, 1.0);
    gl_PointSize = radius;
}


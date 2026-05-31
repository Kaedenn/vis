#version 330 core
layout(location = 0) in vec2 position;
layout(location = 1) in float radius;
layout(location = 2) in vec4 color;
layout(location = 3) in float depth;
layout(location = 4) in uint vertices;
layout(location = 5) in float angle;

uniform mat4 projection;
uniform mat4 view;

out vec4 vColor;
flat out uint vVertices;
out float vAngle;

void main() {
    vColor = color;
    vVertices = vertices;
    vAngle = angle;
    gl_Position = projection * view * vec4(position, depth, 1.0);
    gl_PointSize = radius;
}


#version 330 core
layout(location = 0) in vec3 position;
layout(location = 1) in float radius;
layout(location = 2) in vec4 color;
layout(location = 3) in uint vertices;
layout(location = 4) in float angle;

uniform mat4 projection;
uniform mat4 view;

out vec4 vColor;
flat out uint vVertices;
out float vAngle;
out vec3 vPosWorld;

void main() {
    vColor = color;
    vVertices = vertices;
    vAngle = angle;
    vPosWorld = position;
    gl_Position = projection * view * vec4(position, 1.0);
    gl_PointSize = radius;
}


#version 330 core
layout(location = 0) in vec3 position;
layout(location = 1) in float radius;
layout(location = 2) in vec4 color;
layout(location = 3) in uint vertices;
layout(location = 4) in float angle;

uniform mat4 projection;
uniform mat4 view;

#ifdef IS_STRIP
out vec4 fragColor;
#else
out vec4 vColor;
flat out uint vVertices;
out float vAngle;
#endif

out vec3 vPosWorld;

void main() {
    vPosWorld = position;
    gl_Position = projection * view * vec4(position, 1.0);
    gl_PointSize = radius;

#ifdef IS_STRIP
    fragColor = color;
#else
    vColor = color;
    vVertices = vertices;
    vAngle = angle;
#endif
}


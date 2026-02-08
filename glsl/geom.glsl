#version 330 core
layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

void main() {
    vec2 position = gl_in[0].gl_Position.xy;
    float size = gl_in[0].gl_PointSize;
    gl_Position = vec4(position - vec2(size, 0), 0, 0);
    EmitVertex();
    gl_Position = vec4(position - vec2(0, size), 0, 0);
    EmitVertex();
    gl_Position = vec4(position + vec2(size, 0), 0, 0);
    EmitVertex();
    gl_Position = vec4(position + vec2(0, size), 0, 0);
    EmitVertex();
    EndPrimitive();
}


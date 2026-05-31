#version 330 core
layout (points) in;
layout (triangle_strip, max_vertices = 96) out;

in vec4 vColor[];
flat in uint vVertices[];
in float vAngle[];

out vec4 fragColor;

uniform mat4 projection;

const float PI = 3.14159265359;

void main() {
    vec4 position = gl_in[0].gl_Position;
    float size = gl_in[0].gl_PointSize;
    uint num_vertices = min(vVertices[0], 32u);
    vec4 color = vColor[0];
    float base_angle = vAngle[0];

    // Fast-path optimization for particles ~1 pixel in radius or smaller.
    // At this size, they are just tiny specks, so there's no visual benefit 
    // to emitting up to 96 vertices for a perfect regular polygon. We can 
    // just emit a simple 4-vertex quad (square) triangle strip and return.
    if (size <= 1.5) {
        fragColor = color;
        
        gl_Position = position + projection * vec4(-size, -size, 0.0, 0.0);
        EmitVertex();
        
        gl_Position = position + projection * vec4(size, -size, 0.0, 0.0);
        EmitVertex();
        
        gl_Position = position + projection * vec4(-size, size, 0.0, 0.0);
        EmitVertex();
        
        gl_Position = position + projection * vec4(size, size, 0.0, 0.0);
        EmitVertex();
        
        EndPrimitive();
        return;
    }

    // Calculate rotation offset so that squares (num_vertices == 4) 
    // are axis-aligned if desired, or just start at angle 0.
    // The previous square was a diamond (angles 0, pi/2, pi, 3pi/2).
    // We'll stick to a simple rotation starting at 0.

    for (uint i = 0u; i < num_vertices; i++) {
        // Center vertex
        fragColor = color;
        gl_Position = position;
        EmitVertex();

        // Vertex i
        float angle1 = base_angle + float(i) * 2.0 * PI / float(num_vertices);
        fragColor = color;
        gl_Position = position + projection * vec4(cos(angle1) * size, sin(angle1) * size, 0.0, 0.0);
        EmitVertex();

        // Vertex i+1
        float angle2 = base_angle + float(i + 1u) * 2.0 * PI / float(num_vertices);
        fragColor = color;
        gl_Position = position + projection * vec4(cos(angle2) * size, sin(angle2) * size, 0.0, 0.0);
        EmitVertex();

        EndPrimitive();
    }
}


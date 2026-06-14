#version 330 core
layout (points) in;
layout (triangle_strip, max_vertices = 96) out;

in vec4 vColor[];
flat in uint vVertices[];
in float vAngle[];
in vec3 vPosWorld[];

out vec4 fragColor;

uniform mat4 projection;
uniform mat4 view;

const float PI = 3.14159265359;

void main() {
    vec3 pos_world = vPosWorld[0];
    float size = gl_in[0].gl_PointSize;
    uint num_vertices = min(vVertices[0], 32u);
    vec4 color = vColor[0];
    float base_angle = vAngle[0];

    // Extract right and up vectors from the view matrix for billboarding
    vec3 right = vec3(view[0][0], view[1][0], view[2][0]);
    vec3 up    = vec3(view[0][1], view[1][1], view[2][1]);

    // Fast-path optimization for particles ~1 pixel in radius or smaller.
    if (size <= 1.5) {
        fragColor = color;
        
        gl_Position = projection * view * vec4(pos_world + right * (-size) + up * (-size), 1.0);
        EmitVertex();
        
        gl_Position = projection * view * vec4(pos_world + right * (size) + up * (-size), 1.0);
        EmitVertex();
        
        gl_Position = projection * view * vec4(pos_world + right * (-size) + up * (size), 1.0);
        EmitVertex();
        
        gl_Position = projection * view * vec4(pos_world + right * (size) + up * (size), 1.0);
        EmitVertex();
        
        EndPrimitive();
        return;
    }

    // Calculate rotation offset so that squares (num_vertices == 4) 
    // are axis-aligned if desired, or just start at angle 0.
    for (uint i = 0u; i < num_vertices; i++) {
        // Center vertex
        fragColor = color;
        gl_Position = projection * view * vec4(pos_world, 1.0);
        EmitVertex();

        // Vertex i
        float angle1 = base_angle + float(i) * 2.0 * PI / float(num_vertices);
        fragColor = color;
        vec3 offset1 = right * (cos(angle1) * size) + up * (sin(angle1) * size);
        gl_Position = projection * view * vec4(pos_world + offset1, 1.0);
        EmitVertex();

        // Vertex i+1
        float angle2 = base_angle + float(i + 1u) * 2.0 * PI / float(num_vertices);
        fragColor = color;
        vec3 offset2 = right * (cos(angle2) * size) + up * (sin(angle2) * size);
        gl_Position = projection * view * vec4(pos_world + offset2, 1.0);
        EmitVertex();

        EndPrimitive();
    }
}


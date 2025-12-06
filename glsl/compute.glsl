#version 430

layout (local_size_x = 1) in;

layout(std430, binding=0) buffer Pos {
    vec2 Position[];
};

layout(std430, binding=1) buffer Vel {
    vec2 Velocity[];
};

void main() {
    uint idx = gl_GlobalInvocationID.x;
    vec2 p = Position[idx].xy;
    vec2 v = Velocity[idx].xy;

    Position[idx] = p + v;
}


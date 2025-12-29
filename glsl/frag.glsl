#version 330 core
in vec4 fragColor;
flat in uint fragDepth;
out vec4 color;
void main() {
    color = vec4(fragColor);
    gl_FragDepth = fragDepth;
}


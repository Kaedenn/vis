#version 330

layout(location=0) in vec2 position;
layout(location=1) in vec3 color;

out vec3 fragment_color;
void main() {
    fragment_color = color;
    glPosition = vec3(position, 1);
}


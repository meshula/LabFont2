#version 450

// Fragment input
layout(location = 0) in vec2 inTexCoord;
layout(location = 1) in vec4 inColor;

// Fragment output
layout(location = 0) out vec4 outColor;

void main() {
    outColor = inColor;
}

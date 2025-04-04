#version 450

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in vec4 inColor;

layout(location = 0) out vec2 outTexCoord;
layout(location = 1) out vec4 outColor;

void main() {
    gl_Position = vec4(inPosition, 0.0, 1.0);
    outTexCoord = inTexCoord;
    outColor = inColor;
}

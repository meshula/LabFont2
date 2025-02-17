#version 450

// Fragment input
layout(location = 0) in vec2 inTexCoord;
layout(location = 1) in vec4 inColor;

// Fragment output
layout(location = 0) out vec4 outColor;

void main() {
    // Calculate distance from line center for anti-aliasing
    float dist = abs(inTexCoord.y - 0.5) * 2.0;
    float alpha = 1.0 - smoothstep(0.8, 1.0, dist);
    
    // Apply alpha for smooth edges
    outColor = vec4(inColor.rgb, inColor.a * alpha);
}

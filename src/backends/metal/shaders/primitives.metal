#include <metal_stdlib>
using namespace metal;

struct VertexIn {
    float2 position [[attribute(0)]];
    float2 texcoord [[attribute(1)]];
    float4 color    [[attribute(2)]];
};

struct VertexOut {
    float4 position [[position]];
    float2 texcoord;
    float4 color;
};

// Vertex shader for triangles and lines
vertex VertexOut vertex_main(VertexIn in [[stage_in]]) {
    VertexOut out;
    out.position = float4(in.position, 0.0, 1.0);
    out.texcoord = in.texcoord;
    out.color = in.color;
    return out;
}

// Fragment shader for triangles and lines
fragment float4 fragment_color(VertexOut in [[stage_in]]) {
    return in.color;
}

// fragment shader for textured triangles
fragment float4 fragment_texture(VertexOut in [[stage_in]],
                               texture2d<float> colorTexture [[texture(0)]]) {
    constexpr sampler textureSampler (mag_filter::linear,
                                      min_filter::linear);
    // Sample the texture to obtain a color
    const float4 colorSample = colorTexture.sample(textureSampler, in.texcoord);
    return colorSample;
}

// Fragment shader for lines with anti-aliasing
fragment float4 fragment_line(VertexOut in [[stage_in]]) {
    // Calculate distance from center line
    float dist = abs(in.texcoord.y - 0.5) * 2.0;
    
    // Apply anti-aliasing
    float alpha = 1.0 - smoothstep(0.8, 1.0, dist);
    return float4(in.color.rgb, in.color.a * alpha);
}

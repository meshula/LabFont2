// Vertex shader input
struct VertexInput {
    @location(0) position: vec2<f32>,
    @location(1) texcoord: vec2<f32>,
    @location(2) color: vec4<f32>,
};

// Vertex shader output / Fragment shader input
struct VertexOutput {
    @builtin(position) position: vec4<f32>,
    @location(0) texcoord: vec2<f32>,
    @location(1) color: vec4<f32>,
};

// Vertex shader
@vertex
fn vertex_main(input: VertexInput) -> VertexOutput {
    var output: VertexOutput;
    output.position = vec4<f32>(input.position, 0.0, 1.0);
    output.texcoord = input.texcoord;
    output.color = input.color;
    return output;
}

// Fragment shader for triangles
@fragment
fn fragment_main(input: VertexOutput) -> @location(0) vec4<f32> {
    return input.color;
}

// Fragment shader for lines with anti-aliasing
@fragment
fn fragment_line(input: VertexOutput) -> @location(0) vec4<f32> {
    // Calculate distance from center line
    let dist = abs(input.texcoord.y - 0.5) * 2.0;
    
    // Apply anti-aliasing
    let alpha = 1.0 - smoothstep(0.8, 1.0, dist);
    return vec4<f32>(input.color.rgb, input.color.a * alpha);
}

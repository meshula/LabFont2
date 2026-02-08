#include "cpu_backend.h"
#include "rasterizer.h"
#include <cstring>

namespace labfont {

lab_result CPUBackend::SubmitCommands(const std::vector<DrawCommand>& commands) {
    // Get current render target
    if (!m_currentRenderTarget) {
        return LAB_RESULT_STATE_NO_RENDER_TARGET_SET;
    }
    
    auto colorTexture = static_cast<CPUTexture*>(m_currentRenderTarget->GetColorTexture());
    if (!colorTexture) {
        return LAB_RESULT_INVALID_RENDER_TARGET;
    }
    
    // Get render target dimensions and buffer
    uint32_t width = colorTexture->GetWidth();
    uint32_t height = colorTexture->GetHeight();
    uint8_t* colorBuffer = colorTexture->GetData();
    
    // Process each command
    for (const auto& cmd : commands) {
        switch (cmd.type) {
            case DrawCommandType::Clear: {
                // Convert float color to uint8_t
                uint8_t clearColor[4];
                for (int i = 0; i < 4; ++i) {
                    clearColor[i] = static_cast<uint8_t>(cpu::Clamp(cmd.clear.color[i] * 255.0f, 0.0f, 255.0f));
                }
                
                // Fill color buffer
                for (size_t i = 0; i < width * height; ++i) {
                    std::memcpy(&colorBuffer[i * 4], clearColor, 4);
                }
                break;
            }
            
            case DrawCommandType::DrawTriangles: {
                const auto& params = cmd.triangles;
                // Create a copy of vertices to transform
                std::vector<lab_vertex_2TC> transformedVertices(params.vertices, params.vertices + params.vertexCount);
                
                // Apply coordinate transformation to each vertex
                for (auto& vertex : transformedVertices) {
                    TransformVertexToViewport(vertex);
                }
                
                for (uint32_t i = 0; i < params.vertexCount; i += 3) {
                    cpu::DrawTriangle(
                        colorBuffer,
                        nullptr, // TODO: Depth buffer
                        width,
                        height,
                        &transformedVertices[i],
                        m_currentBlendMode
                    );
                }
                break;
            }
            
            case DrawCommandType::DrawLines: {
                const auto& params = cmd.lines;
                for (uint32_t i = 0; i < params.vertexCount; i += 2) {
                    cpu::DrawLine(
                        colorBuffer,
                        width,
                        height,
                        &params.vertices[i],
                        params.lineWidth,
                        m_currentBlendMode
                    );
                }
                break;
            }
                
            case DrawCommandType::BindTexture: {
                break;
            }
            
            case DrawCommandType::SetViewportAPI: {
                // Update viewport state
                m_viewportX = cmd.viewport.x;
                m_viewportY = cmd.viewport.y;
                m_viewportWidth = cmd.viewport.width;
                m_viewportHeight = cmd.viewport.height;
                break;
            }
            
            case DrawCommandType::SetBlendMode: {
                m_currentBlendMode = cmd.blend.mode;
                break;
            }
            
            case DrawCommandType::SetScissor: {
                // TODO: Implement scissor clipping in CPU backend
                break;
            }
            
            case DrawCommandType::SetViewport: {
                // Internal viewport command (same as API version)
                m_viewportX = cmd.viewport.x;
                m_viewportY = cmd.viewport.y;
                m_viewportWidth = cmd.viewport.width;
                m_viewportHeight = cmd.viewport.height;
                break;
            }
        }
    }
    
    // Update the texture with the modified buffer
    colorTexture->SetData(colorBuffer, width * height * 4);

    // Store commands for testing/debugging
    m_commands.insert(m_commands.end(), commands.begin(), commands.end());
    
    return LAB_RESULT_OK;
}

} // namespace labfont

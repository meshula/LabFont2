#ifndef LABFONT_CONTEXT_INTERNAL_H
#define LABFONT_CONTEXT_INTERNAL_H

#include <labfont/labfont.h>
#include <memory>
#include <vector>
#include "backend.h"
#include "font_manager.h"
#include "draw_state.h"
#include "resource_manager.h"
#include "memory.h"
#include "error.h"
#include "coordinate_system.h"

namespace labfont {

class Context {
public:
    static lab_result Create(lab_backend_type type, const lab_context_desc* desc, Context** out_context);
    ~Context();

    lab_result Resize(unsigned int width, unsigned int height);
    
    // Frame management
    void BeginFrame();
    void EndFrame();
    void Clear(lab_color color);
    
    // State management
    void BeginText();
    void EndText();
    void BeginDraw();
    void EndDraw();
    
    // Getters
    Backend* GetBackend() { return m_backend.get(); }
    FontManager* GetFontManager() { return m_fontManager.get(); }
    DrawState* GetDrawState() { return m_drawState.get(); }
    ResourceManagerImpl* GetResourceManager() { return m_resourceManager.get(); }
    MemoryManager* GetMemoryManager() { return &MemoryManager::Instance(); }
    
    // Viewport
    void SetViewport(float x, float y, float width, float height);
    
    // Coordinate system management
    void SetCoordinateSystem(const lab_coordinate_system& coord_system);
    const lab_coordinate_system& GetCoordinateSystem() const;
    
protected:
    Context() = default;  // Protected constructor, use Create() instead
    
    // Factory method to create the appropriate backend
    std::unique_ptr<Backend> CreateBackend(lab_backend_type type);
    
    lab_result Initialize(lab_backend_type type, const lab_context_desc* desc);
    
    std::unique_ptr<Backend> m_backend;
    std::unique_ptr<FontManager> m_fontManager;
    std::unique_ptr<DrawState> m_drawState;
    std::unique_ptr<ResourceManagerImpl> m_resourceManager;
    
    unsigned int m_width;
    unsigned int m_height;
    bool m_inTextMode;
    bool m_inDrawMode;
    
    // Coordinate system
    lab_coordinate_system m_coordinateSystem;
    bool m_coordinateSystemInitialized;
};

// Helper function to convert C handle to C++ object
inline Context* GetContextImpl(lab_context ctx) {
    return reinterpret_cast<Context*>(ctx);
}

// Helper function to convert C++ object to C handle
inline lab_context GetContextHandle(Context* ctx) {
    return reinterpret_cast<lab_context>(ctx);
}

}  // namespace labfont

#endif /* LABFONT_CONTEXT_INTERNAL_H */

#ifndef LABFONT_BACKEND_H
#define LABFONT_BACKEND_H

#include <labfont/labfont_types.h>

namespace labfont {

class Backend {
public:
    virtual ~Backend() = default;
    
    virtual lab_result Initialize(const lab_context_desc* desc) = 0;
    virtual lab_result Resize(unsigned int width, unsigned int height) = 0;
    
    virtual void BeginFrame() = 0;
    virtual void EndFrame() = 0;
    virtual void Clear(lab_color color) = 0;
    virtual void SetViewport(float x, float y, float width, float height) = 0;
};

} // namespace labfont

#endif // LABFONT_BACKEND_H

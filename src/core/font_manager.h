#ifndef LABFONT_FONT_MANAGER_H
#define LABFONT_FONT_MANAGER_H

#include <labfont/labfont_types.h>
#include <string>
#include <unordered_map>

namespace labfont {

class FontManager {
public:
    FontManager() = default;
    ~FontManager() = default;

    lab_result LoadFont(const char* path, lab_font* out_font);
    void DestroyFont(lab_font font);

private:
    // Will be implemented later
    std::unordered_map<std::string, void*> m_fonts;  // Placeholder for font storage
};

} // namespace labfont

#endif // LABFONT_FONT_MANAGER_H

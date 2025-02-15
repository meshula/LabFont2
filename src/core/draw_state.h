#ifndef LABFONT_DRAW_STATE_H
#define LABFONT_DRAW_STATE_H

#include <labfont/labfont_types.h>

namespace labfont {

class DrawState {
public:
    DrawState() = default;
    ~DrawState() = default;

    // State setters
    void SetColor(lab_color color) { m_currentColor = color; }
    void SetLineWidth(float width) { m_lineWidth = width; }
    void SetFont(lab_font font) { m_currentFont = font; }
    void SetFontSize(float size) { m_fontSize = size; }
    void SetTextColor(lab_color color) { m_textColor = color; }

    // State getters
    lab_color GetColor() const { return m_currentColor; }
    float GetLineWidth() const { return m_lineWidth; }
    lab_font GetFont() const { return m_currentFont; }
    float GetFontSize() const { return m_fontSize; }
    lab_color GetTextColor() const { return m_textColor; }

private:
    lab_color m_currentColor = {1.0f, 1.0f, 1.0f, 1.0f};
    float m_lineWidth = 1.0f;
    lab_font m_currentFont = nullptr;
    float m_fontSize = 12.0f;
    lab_color m_textColor = {0.0f, 0.0f, 0.0f, 1.0f};
};

} // namespace labfont

#endif // LABFONT_DRAW_STATE_H

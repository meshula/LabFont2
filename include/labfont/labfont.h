#ifndef LABFONT_H
#define LABFONT_H

#include "labfont_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Library initialization and shutdown */
lab_result lab_initialize(void);
void lab_shutdown(void);

/* Context Management */
lab_result lab_create_context(lab_backend_type type, const lab_context_desc* desc, lab_context* out_context);
void lab_destroy_context(lab_context ctx);
lab_result lab_resize_context(lab_context ctx, unsigned int width, unsigned int height);

/* Font Management */
lab_result lab_load_font(lab_context ctx, const char* path, lab_font* out_font);
void lab_destroy_font(lab_font font);

/* State Management */
void lab_begin_frame(lab_context ctx);
void lab_end_frame(lab_context ctx);
void lab_clear(lab_context ctx, lab_color color);

/* Text Rendering */
void lab_begin_text(lab_context ctx);
void lab_set_font(lab_context ctx, lab_font font);
void lab_set_font_size(lab_context ctx, float size);
void lab_set_text_color(lab_context ctx, lab_color color);
void lab_set_text_align(lab_context ctx, lab_text_align align);
void lab_set_text_style(lab_context ctx, lab_text_style style);
lab_result lab_draw_text(lab_context ctx, float x, float y, const char* text);
void lab_end_text(lab_context ctx);

/* Immediate Mode Drawing */
void lab_begin_draw(lab_context ctx);
void lab_set_color(lab_context ctx, lab_color color);
void lab_set_line_width(lab_context ctx, float width);
void lab_draw_line(lab_context ctx, float x1, float y1, float x2, float y2);
void lab_draw_rect(lab_context ctx, float x, float y, float w, float h);
void lab_draw_filled_rect(lab_context ctx, float x, float y, float w, float h);
void lab_draw_circle(lab_context ctx, float x, float y, float radius);
void lab_draw_filled_circle(lab_context ctx, float x, float y, float radius);
void lab_end_draw(lab_context ctx);

/* Utility Functions */
lab_color lab_color_rgba(float r, float g, float b, float a);
lab_color lab_color_rgb(float r, float g, float b);
const char* lab_get_error_string(lab_error error);
void lab_set_viewport(lab_context ctx, float x, float y, float width, float height);

#ifdef __cplusplus
}
#endif

#endif /* LABFONT_H */

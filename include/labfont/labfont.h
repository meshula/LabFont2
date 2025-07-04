#ifndef LABFONT_H
#define LABFONT_H

#include "labfont_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Context management */
lab_result lab_create_context(const lab_backend_desc* desc, lab_context* out_context);
void lab_destroy_context(lab_context ctx);

/* Memory management */
void* lab_alloc(size_t size, lab_memory_category category);
void lab_free(void* ptr);
lab_memory_stats lab_get_memory_stats(void);
void lab_reset_memory_stats(void);

/* Font management */
lab_result lab_load_font(lab_context ctx, const char* path, lab_font* out_font);
void lab_destroy_font(lab_context ctx, lab_font font);

/* Resource management */
lab_result lab_create_texture(lab_context ctx, const lab_texture_desc* desc, lab_texture* out_texture);
lab_result lab_load_texture(lab_context ctx, const char* path, lab_texture* out_texture);
void lab_destroy_texture(lab_context ctx, lab_texture texture);
lab_result lab_create_buffer(lab_context ctx, const lab_buffer_desc* desc, lab_buffer* out_buffer);
void lab_destroy_buffer(lab_context ctx, lab_buffer buffer);

lab_result lab_texture_width(lab_texture texture, int* width);
lab_result lab_texture_height(lab_texture texture, int* height);

/* Render target management */
lab_result lab_create_render_target(lab_context ctx, const lab_render_target_desc* desc, lab_render_target* out_target);
void lab_destroy_render_target(lab_context ctx, lab_render_target target);
lab_result lab_resize_render_target(lab_context ctx, lab_render_target target, uint32_t width, uint32_t height);
lab_result lab_set_render_target(lab_context ctx, lab_render_target target);
lab_result lab_save_render_target(lab_context ctx, lab_render_target target, const char* filename);
lab_result lab_get_render_target_data(lab_context ctx, lab_render_target target, lab_render_target_desc* desc, uint8_t** out_data, size_t* out_size);

/* Frame management */
lab_result lab_begin_frame(lab_context ctx);
lab_result lab_end_frame(lab_context ctx);

/* Coordinate system management */
lab_result lab_init_coordinate_system(lab_coordinate_system* coord_system, const lab_coordinate_desc* desc);
lab_result lab_set_coordinate_system(lab_context ctx, const lab_coordinate_system* coord_system);
lab_result lab_get_coordinate_system(lab_context ctx, lab_coordinate_system* out_coord_system);
lab_result lab_transform_point(const lab_coordinate_system* coord_system, 
                              lab_coordinate_space from_space, lab_coordinate_space to_space,
                              const float input[2], float output[2]);
lab_result lab_transform_vertex(const lab_coordinate_system* coord_system,
                               lab_coordinate_space from_space, lab_coordinate_space to_space,
                               const lab_vertex_2TC* input, lab_vertex_2TC* output);

/* Draw commands */
lab_result lab_submit_commands(lab_context ctx, const lab_draw_command* commands, uint32_t commandCount);

/* Text rendering */
lab_result lab_draw_text(lab_context ctx, lab_font font, const char* text, float x, float y, lab_color color);
lab_result lab_draw_text_formatted(lab_context ctx, lab_font font, const char* text, float x, float y, 
                                 lab_color color, lab_text_align align, lab_text_style style);

/* Error handling */
const char* lab_get_error_string(lab_result error);

#ifdef __cplusplus
}
#endif

#endif /* LABFONT_H */

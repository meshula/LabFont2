/**
 * labfont_renderer.h - Rich text rendering based on markup styles
 * 
 * This component connects the style parser with the LabFont rendering API,
 * handling the application of styles to text and managing layout.
 */

 #ifndef LABFONT_RENDERER_H
 #define LABFONT_RENDERER_H
 
 #include "labfont_draw.h"
 #include "labfont_style_parser.h"
 
 #ifdef __cplusplus
 extern "C" {
 #endif
 
 /**
  * Types and structures
  */
 typedef struct {
    float x, y;
} labfont_xy;

 /**
  * Rich text renderer context
  */
 typedef struct labfont_renderer labfont_renderer;
 
 /**
  * Options for rich text layout
  */
 typedef struct {
     float wrap_width;  /* Maximum line width for wrapping (0 = no wrap) */
     float line_height; /* Line height multiplier (1.0 = normal) */
     int max_lines;     /* Maximum number of lines to render (0 = unlimited) */
     bool ellipsis;     /* Add ellipsis if text is truncated */
 } labfont_layout_options;
 
 /**
  * Text measurement result
  */
 typedef struct {
     float width;       /* Total width of the text */
     float height;      /* Total height of the text */
     int line_count;    /* Number of lines */
     bool truncated;    /* Whether the text was truncated */
 } labfont_text_metrics;
 
 /**
  * Create a new rich text renderer
  */
 labfont_renderer* labfont_renderer_create(void);
 
 /**
  * Destroy a rich text renderer
  */
 void labfont_renderer_destroy(labfont_renderer* renderer);
 
 /**
  * Define a global style that can be referenced with {@name}
  */
 bool labfont_renderer_define_global_style(labfont_renderer* renderer, 
                                          const char* name, 
                                          const char* style_def);
 
 /**
  * Remove a global style
  */
 bool labfont_renderer_remove_global_style(labfont_renderer* renderer, 
                                          const char* name);
 
 /**
  * Check if a global style exists
  */
 bool labfont_renderer_has_global_style(labfont_renderer* renderer, 
                                       const char* name);
 
 /**
  * Clear all global styles
  */
 void labfont_renderer_clear_global_styles(labfont_renderer* renderer);
 
 /**
  * Load global styles from a file
  * Format: @name: property=value property=value
  */
 bool labfont_renderer_load_stylefile(labfont_renderer* renderer, 
                                     const char* path);
 
 /**
  * Save current global styles to a file
  */
 bool labfont_renderer_save_stylefile(labfont_renderer* renderer, 
                                     const char* path);
 
 /**
  * Measure rich text dimensions without rendering
  */
 labfont_text_metrics labfont_renderer_measure_text(labfont_renderer* renderer, 
                                                  const char* markup_text,
                                                  const labfont_layout_options* options);
 
 /**
  * Draw rich text on screen
  * Returns the end position (for continuing text)
  */
 labfont_xy labfont_renderer_draw_text(labfont_renderer* renderer,
                                     LabFontDrawState* draw_state,
                                     float x, float y,
                                     const char* markup_text,
                                     const labfont_layout_options* options);
 
 /**
  * Same as labfont_renderer_draw_text but with printf formatting
  */
 labfont_xy labfont_renderer_printf(labfont_renderer* renderer,
                                   LabFontDrawState* draw_state,
                                   float x, float y,
                                   const labfont_layout_options* options,
                                   const char* format, ...);
 
 /**
  * Utility to convert a standard LabFont alignment to labfont_style
  */
 bool labfont_renderer_set_alignment(labfont_style* style, 
                                    int lab_font_alignment);
 
 /**
  * Get the last error message from the renderer
  */
 const char* labfont_renderer_get_last_error(void);
 
 /**
  * Advanced usage: get the style manager from the renderer
  * for direct manipulation
  */
 labfont_style_manager* labfont_renderer_get_style_manager(labfont_renderer* renderer);
 
 /**
  * Cache control
  */
 
 /**
  * Clear the LabFontState cache to free memory
  */
 void labfont_renderer_clear_cache(labfont_renderer* renderer);
 
 /**
  * Set the maximum size of the LabFontState cache
  * (0 = unlimited)
  */
 void labfont_renderer_set_cache_size(labfont_renderer* renderer, size_t max_entries);
 
 #ifdef __cplusplus
 }
 #endif
 
 #endif /* LABFONT_RENDERER_H */

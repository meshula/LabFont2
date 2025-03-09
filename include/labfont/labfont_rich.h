/**
 * labfont_rich.h - Rich text rendering with markup and style management
 * 
 * A modern, markup-based text rendering system with support for
 * named styles, style inheritance, and global style definitions.
 */

 #ifndef LABFONT_RICH_H
 #define LABFONT_RICH_H
 
 #include <stddef.h>
 #include <stdint.h>
 #include <stdarg.h>
 #include <stdbool.h>
 
 #ifdef __cplusplus
 extern "C" {
 #endif
 
 /**
  * Basic types
  */
 typedef struct {
     float x, y;
 } labfont_xy;
 
 typedef struct labfont_drawstate labfont_drawstate;
 typedef struct labfont_style labfont_style;
 typedef struct labfont_style_manager labfont_style_manager;
 
 /**
  * Font color definition (RGBA)
  */
 typedef struct {
     uint8_t r, g, b, a;
 } labfont_color;
 
 /**
  * Alignment constants
  */
 #define LABFONT_ALIGN_TOP       (1 << 0)
 #define LABFONT_ALIGN_MIDDLE    (1 << 1)
 #define LABFONT_ALIGN_BASELINE  (1 << 2)
 #define LABFONT_ALIGN_BOTTOM    (1 << 3)
 #define LABFONT_ALIGN_LEFT      (1 << 4)
 #define LABFONT_ALIGN_CENTER    (1 << 5)
 #define LABFONT_ALIGN_RIGHT     (1 << 6)
 
 /**
  * Drawing context initialization and cleanup
  */
 labfont_drawstate* labfont_draw_begin(float origin_x, float origin_y, 
                                       float width, float height);
 void labfont_draw_end(labfont_drawstate* ds);
 
 /**
  * Core rich text rendering functions
  */
 
 /**
  * Renders rich text with markup and returns the next position for rendering
  * 
  * Markup format:
  *   {name: property=value property=value} - Define a named style
  *   {property=value} - Apply style properties to current text
  *   {name} - Apply a previously defined style
  *   {@name} - Apply a global style
  *   {/} or {/name} - Revert to previous style
  * 
  * Common properties:
  *   font    - Font name (e.g., "sans-normal", "sans-italic", "serif-bold")
  *   size    - Font size in pixels
  *   color   - Text color in hex format (#RRGGBB or #RRGGBBAA)
  *   align   - Text alignment (top, middle, baseline, bottom, left, center, right)
  *   spacing - Additional spacing between characters
  *   blur    - Blur radius for text
  * 
  * Shortcuts:
  *   {b} - Bold
  *   {i} - Italic
  *   {u} - Underline
  *   {c=#RRGGBB} - Color
  */
 labfont_xy labfont_draw_rich(labfont_drawstate* ds, float x, float y, const char* markup);
 
 /**
  * Same as labfont_draw_rich but with printf-style formatting
  */
 labfont_xy labfont_printf_rich(labfont_drawstate* ds, float x, float y, 
                                const char* markup_format, ...);
 labfont_xy labfont_vprintf_rich(labfont_drawstate* ds, float x, float y, 
                                 const char* markup_format, va_list args);
 
 /**
  * Measures text dimensions without rendering
  */
 typedef struct {
     float width;      /* Total width of the text */
     float height;     /* Total height of the text */
     float ascender;   /* Height above baseline */
     float descender;  /* Depth below baseline */
 } labfont_size;
 
 labfont_size labfont_measure_rich(const char* markup);
 
 /**
  * Global style management
  */
 
 /**
  * Define a global style that can be referenced in any draw call with {@name}
  * Returns true if style was successfully defined
  */
 bool labfont_define_global_style(const char* name, const char* style_def);
 
 /**
  * Check if a global style exists
  */
 bool labfont_has_global_style(const char* name);
 
 /**
  * Get the definition string for a global style
  * Returns NULL if style does not exist
  */
 const char* labfont_get_global_style_def(const char* name);
 
 /**
  * Remove a global style
  */
 void labfont_remove_global_style(const char* name);
 
 /**
  * Clear all global styles
  */
 void labfont_clear_global_styles(void);
 
 /**
  * Style file management
  */
 
 /**
  * Load global styles from a file
  * Format: @name: property=value property=value
  * Returns true if file was successfully loaded
  */
 bool labfont_load_stylefile(const char* path);
 
 /**
  * Save current global styles to a file
  * Returns true if file was successfully saved
  */
 bool labfont_save_stylefile(const char* path);
 
 /**
  * Low-level style manipulation for advanced usage
  */
 
 /**
  * Create a style manager for manual style handling
  */
 labfont_style_manager* labfont_create_style_manager(void);
 
 /**
  * Destroy a style manager
  */
 void labfont_destroy_style_manager(labfont_style_manager* manager);
 
 /**
  * Parse a style definition string into a style object
  * Returns NULL if parsing fails
  */
 labfont_style* labfont_parse_style(labfont_style_manager* manager, 
                                   const char* style_def);
 
 /**
  * Define a named style in a style manager
  * Returns true if style was successfully defined
  */
 bool labfont_define_style(labfont_style_manager* manager, 
                          const char* name, 
                          const char* style_def);
 
 /**
  * Get a previously defined style by name
  * Returns NULL if style does not exist
  */
 labfont_style* labfont_get_style(labfont_style_manager* manager, 
                                 const char* name);
 
 /**
  * Draw text with a specific style
  */
 labfont_xy labfont_draw_with_style(labfont_drawstate* ds, 
                                   float x, float y, 
                                   const char* text, 
                                   labfont_style* style);
 
 /**
  * Getters/setters for style properties
  */
 bool labfont_style_set_font(labfont_style* style, const char* font_name);
 bool labfont_style_set_size(labfont_style* style, float size);
 bool labfont_style_set_color(labfont_style* style, labfont_color color);
 bool labfont_style_set_color_hex(labfont_style* style, const char* hex_color);
 bool labfont_style_set_alignment(labfont_style* style, int alignment);
 bool labfont_style_set_spacing(labfont_style* style, float spacing);
 bool labfont_style_set_blur(labfont_style* style, float blur);
 
 const char* labfont_style_get_font(const labfont_style* style);
 float labfont_style_get_size(const labfont_style* style);
 labfont_color labfont_style_get_color(const labfont_style* style);
 int labfont_style_get_alignment(const labfont_style* style);
 float labfont_style_get_spacing(const labfont_style* style);
 float labfont_style_get_blur(const labfont_style* style);
 
 /**
  * Utility functions
  */
 
 /**
  * Parse a hex color string (#RGB, #RGBA, #RRGGBB, or #RRGGBBAA)
  * Returns true if parsing was successful
  */
 bool labfont_parse_color(const char* hex_color, labfont_color* out_color);
 
 /**
  * Convert a color to a hex string
  * Buffer should be at least 10 characters (#RRGGBBAA\0)
  */
 void labfont_color_to_hex(labfont_color color, char* buffer, size_t buffer_size);
 
 /**
  * Error handling
  */
 
 /**
  * Get the last error message
  */
 const char* labfont_get_last_error(void);
 
 /**
  * Clear the last error
  */
 void labfont_clear_error(void);
 
 #ifdef __cplusplus
 }
 #endif
 
 #endif /* LABFONT_RICH_H */

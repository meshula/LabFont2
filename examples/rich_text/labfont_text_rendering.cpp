/**
 * Test program for labfont_renderer
 */

 #include <stdio.h>
 #include "labfont/labfont.h"
 #include "labfont/labfont_renderer.h"
 
 // Font demo structure from your original example
 extern struct {
     struct LabFont* font_japanese;
     struct LabFont* font_normal;
     struct LabFont* font_italic;
     struct LabFont* font_bold;
     struct LabFont* font_cousine;
     struct LabFont* font_robot18;
     struct LabFont* font_c64;
 } font_demo;
 
 // Forward declarations for existing functions
 extern void font_demo_init(const char* path);
 extern void line(float x1, float y1, float x2, float y2);
 
 // New demo function using rich text
 void richTextDemo(LabFontDrawState* ds, float x, float y) {
     // Create a renderer
     labfont_renderer* renderer = labfont_renderer_create();
     if (!renderer) {
         printf("Failed to create renderer: %s\n", labfont_renderer_get_last_error());
         return;
     }
     
     // Define some global styles
     labfont_renderer_define_global_style(renderer, "normal", 
         "font=serif-normal size=24 color=#FFFFFF");
     
     labfont_renderer_define_global_style(renderer, "heading", 
         "font=serif-bold size=36 color=#FFFF00");
     
     labfont_renderer_define_global_style(renderer, "emphasis", 
         "font=serif-italic size=24 color=#00FFFF");
     
     labfont_renderer_define_global_style(renderer, "code", 
         "font=cousine-regular size=20 color=#00FF00");
     
     // Set layout options for wrapping
     labfont_layout_options layout = {0};
     layout.wrap_width = 600.0f;
     layout.line_height = 1.5f;
     layout.max_lines = 0;  // No limit
     layout.ellipsis = true;
     
     // Draw rich text
     const char* markup = 
         "{@heading}Rich Text Demo{/}\n\n"
         "{@normal}This is {b}bold{/b} text and this is {i}italic{/i} text. "
         "You can also have {c=#FF00FF}colored{/c} text or use {@emphasis}predefined styles{/}.\n\n"
         "For code samples, use the {@code}code{/} style:\n"
         "{@code}void main() {\n"
         "    printf(\"Hello, world!\\n\");\n"
         "}{/}\n\n"
         "{normal: font=serif-normal size=18 color=#AAAAAA}You can also define styles inline "
         "and {size=24}change{/} {color=#FFAA00}individual{/} {style=italic}properties{/} as needed.{/}";
     
     // Measure the text
     labfont_text_metrics metrics = labfont_renderer_measure_text(renderer, markup, &layout);
     printf("Text measurements: %.1f x %.1f, %d lines%s\n", 
            metrics.width, metrics.height, metrics.line_count,
            metrics.truncated ? " (truncated)" : "");
     
     // Draw a border around the text area
     float border_x = x - 10;
     float border_y = y - 10;
     float border_width = metrics.width + 20;
     float border_height = metrics.height + 20;
     
     line(border_x, border_y, border_x + border_width, border_y);
     line(border_x + border_width, border_y, border_x + border_width, border_y + border_height);
     line(border_x + border_width, border_y + border_height, border_x, border_y + border_height);
     line(border_x, border_y + border_height, border_x, border_y);
     
     // Draw the text
     labfont_xy end_pos = labfont_renderer_draw_text(renderer, ds, x, y, markup, &layout);
     
     // Clean up
     labfont_renderer_destroy(renderer);
 }
 
 // Modified main demo function
 void fontDemo(LabFontDrawState* ds, float& dx, float& dy, float sx, float sy) {
     if (font_demo.font_japanese == nullptr)
         return;
 
     // Original demos
     float dpis = 1.f;
     float sz;
     
     // ... (your original demo functions)
     
     // Add our rich text demo
     dx = sx;
     dy += 100.0f;
     richTextDemo(ds, dx, dy);
 }
 
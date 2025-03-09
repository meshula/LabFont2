/**
 * labfont_renderer.c - Implementation of the rich text renderer
 */

 #include "labfont/labfont_renderer.h"
 #include <ctype.h>
 #include <math.h>
 #include <stdlib.h>
 #include <stdio.h>
 #include <string.h>
 #include <stdarg.h>
 #include <assert.h>
 
 /*
  * Error handling
  */
 static char labfont_renderer_last_error[256] = {0};
 
 const char* labfont_renderer_get_last_error(void) {
     return labfont_renderer_last_error;
 }
 
 static void labfont_renderer_set_error(const char* format, ...) {
     va_list args;
     va_start(args, format);
     vsnprintf(labfont_renderer_last_error, sizeof(labfont_renderer_last_error), format, args);
     va_end(args);
 }
 
 /*
  * Font state cache entry
  */
 typedef struct {
     labfont_style* style;
     LabFontState* font_state;
 } labfont_state_cache_entry;
 
 /*
  * Renderer context structure
  */
 struct labfont_renderer {
     /* Style manager for global styles */
     labfont_style_manager* global_styles;
     
     /* LabFontState cache to avoid recreating font states */
     labfont_state_cache_entry* state_cache;
     size_t cache_size;
     size_t cache_capacity;
     size_t max_cache_entries;
     
     /* Style stack for rendering */
     struct {
         labfont_style** styles;
         size_t size;
         size_t capacity;
     } style_stack;
     
     /* Current layout state for line wrapping */
     struct {
         float x;               /* Current x position */
         float y;               /* Current y position */
         float line_start_x;    /* Starting x position of current line */
         float line_height;     /* Current line height */
         int line_count;        /* Number of lines rendered */
         bool truncated;        /* Whether text was truncated */
     } layout;
     
     /* Temporary working memory */
     char* temp_buffer;
     size_t temp_buffer_size;
 };
 
 /*
  * Helper function to create a deep copy of a labfont_style
  * We could use the one from the parser, but this avoids exposing it
  */
 static labfont_style* labfont_style_copy(const labfont_style* style) {
     return labfont_style_clone(style);
 }
 
 /*
  * Helper to compare two styles for equality
  */
 static bool labfont_style_equals(const labfont_style* a, const labfont_style* b) {
     if (!a || !b) return a == b;
     
     // Check if all properties match
     for (int i = 0; i < LABFONT_PROP_COUNT; i++) {
         if (a->has_property[i] != b->has_property[i]) {
             return false;
         }
         
         if (a->has_property[i]) {
             switch (i) {
                 case LABFONT_PROP_FONT:
                 case LABFONT_PROP_INHERIT:
                     // Compare strings
                     if (a->properties[i].string_val == NULL || b->properties[i].string_val == NULL) {
                         if (a->properties[i].string_val != b->properties[i].string_val) {
                             return false;
                         }
                     } else if (strcmp(a->properties[i].string_val, b->properties[i].string_val) != 0) {
                         return false;
                     }
                     break;
                     
                 case LABFONT_PROP_SIZE:
                 case LABFONT_PROP_SPACING:
                 case LABFONT_PROP_BLUR:
                     // Compare floats with small epsilon
                     if (fabsf(a->properties[i].float_val - b->properties[i].float_val) > 0.0001f) {
                         return false;
                     }
                     break;
                     
                 case LABFONT_PROP_COLOR:
                 case LABFONT_PROP_BGCOLOR:
                     // Compare colors
                     if (memcmp(&a->properties[i].color_val, &b->properties[i].color_val, sizeof(labfont_color)) != 0) {
                         return false;
                     }
                     break;
                     
                 default:
                     // Compare ints
                     if (a->properties[i].int_val != b->properties[i].int_val) {
                         return false;
                     }
                     break;
             }
         }
     }
     
     return true;
 }
 
 /*
  * Helper to reset the layout state
  */
 static void labfont_renderer_reset_layout(labfont_renderer* renderer, float x, float y) {
     if (!renderer) return;
     
     renderer->layout.x = x;
     renderer->layout.y = y;
     renderer->layout.line_start_x = x;
     renderer->layout.line_height = 0.0f;
     renderer->layout.line_count = 0;
     renderer->layout.truncated = false;
 }
 
 /*
  * Create a new rich text renderer
  */
 labfont_renderer* labfont_renderer_create(void) {
     labfont_renderer* renderer = (labfont_renderer*)malloc(sizeof(labfont_renderer));
     if (!renderer) {
         labfont_renderer_set_error("Failed to allocate renderer context");
         return NULL;
     }
     
     // Initialize global styles
     renderer->global_styles = labfont_style_manager_create();
     if (!renderer->global_styles) {
         labfont_renderer_set_error("Failed to create style manager");
         free(renderer);
         return NULL;
     }
     
     // Initialize state cache
     renderer->state_cache = NULL;
     renderer->cache_size = 0;
     renderer->cache_capacity = 0;
     renderer->max_cache_entries = 100; // Default cache size
     
     // Initialize style stack
     renderer->style_stack.styles = NULL;
     renderer->style_stack.size = 0;
     renderer->style_stack.capacity = 0;
     
     // Initialize layout state
     labfont_renderer_reset_layout(renderer, 0.0f, 0.0f);
     
     // Initialize temporary buffer
     renderer->temp_buffer = NULL;
     renderer->temp_buffer_size = 0;
     
     return renderer;
 }
 
 /*
  * Destroy a rich text renderer
  */
 void labfont_renderer_destroy(labfont_renderer* renderer) {
     if (!renderer) return;
     
     // Destroy global styles
     labfont_style_manager_destroy(renderer->global_styles);
     
     // Clear state cache
     labfont_renderer_clear_cache(renderer);
     free(renderer->state_cache);
     
     // Free style stack
     for (size_t i = 0; i < renderer->style_stack.size; i++) {
         labfont_style_destroy(renderer->style_stack.styles[i]);
     }
     free(renderer->style_stack.styles);
     
     // Free temporary buffer
     free(renderer->temp_buffer);
     
     // Free renderer context
     free(renderer);
 }
 
 /*
  * Define a global style
  */
 bool labfont_renderer_define_global_style(labfont_renderer* renderer, 
                                          const char* name, 
                                          const char* style_def) {
     if (!renderer || !name || !style_def) {
         labfont_renderer_set_error("Invalid parameters for define_global_style");
         return false;
     }
     
     // Parse the style definition
     labfont_style* style = labfont_style_create();
     if (!style) {
         labfont_renderer_set_error("Failed to create style");
         return false;
     }
     
     if (!labfont_style_parse(style_def, style, renderer->global_styles)) {
         labfont_renderer_set_error("Failed to parse style definition: %s", labfont_parser_get_last_error());
         labfont_style_destroy(style);
         return false;
     }
     
     // Add to global styles
     bool result = labfont_style_manager_define(renderer->global_styles, name, style);
     
     // Clean up
     labfont_style_destroy(style);
     
     return result;
 }
 
 /*
  * Remove a global style
  */
 bool labfont_renderer_remove_global_style(labfont_renderer* renderer, const char* name) {
     if (!renderer || !name) {
         labfont_renderer_set_error("Invalid parameters for remove_global_style");
         return false;
     }
     
     return labfont_style_manager_remove(renderer->global_styles, name);
 }
 
 /*
  * Check if a global style exists
  */
 bool labfont_renderer_has_global_style(labfont_renderer* renderer, const char* name) {
     if (!renderer || !name) return false;
     
     return labfont_style_manager_has(renderer->global_styles, name);
 }
 
 /*
  * Clear all global styles
  */
 void labfont_renderer_clear_global_styles(labfont_renderer* renderer) {
     if (!renderer) return;
     
     labfont_style_manager_clear(renderer->global_styles);
 }
 
 /*
  * Load global styles from a file
  */
 bool labfont_renderer_load_stylefile(labfont_renderer* renderer, const char* path) {
     if (!renderer || !path) {
         labfont_renderer_set_error("Invalid parameters for load_stylefile");
         return false;
     }
     
     FILE* file = fopen(path, "r");
     if (!file) {
         labfont_renderer_set_error("Failed to open style file: %s", path);
         return false;
     }
     
     char line[1024];
     int line_number = 0;
     bool success = true;
     
     while (fgets(line, sizeof(line), file)) {
         line_number++;
         
         // Skip empty lines and comments
         char* p = line;
         while (isspace(*p)) p++;
         if (*p == '\0' || *p == '#' || *p == '/' || *p == ';') continue;
         
         // Check for @name: format
         if (*p != '@') {
             labfont_renderer_set_error("Invalid style format at line %d (expected @name:)", line_number);
             success = false;
             break;
         }
         
         p++; // Skip @
         
         // Extract name
         char* name_start = p;
         while (*p && *p != ':' && *p != '\n') p++;
         
         if (*p != ':') {
             labfont_renderer_set_error("Invalid style format at line %d (expected ':' after name)", line_number);
             success = false;
             break;
         }
         
         // Null-terminate name
         *p = '\0';
         p++; // Skip :
         
         // Trim name
         char* name_end = name_start + strlen(name_start) - 1;
         while (name_end >= name_start && isspace(*name_end)) *name_end-- = '\0';
         
         // Extract style definition
         while (isspace(*p)) p++;
         
         // Remove trailing newline
         char* end = p + strlen(p) - 1;
         while (end >= p && (*end == '\n' || *end == '\r')) *end-- = '\0';
         
         // Define style
         if (!labfont_renderer_define_global_style(renderer, name_start, p)) {
             labfont_renderer_set_error("Failed to define style '%s' at line %d: %s", 
                                      name_start, line_number, labfont_renderer_get_last_error());
             success = false;
             break;
         }
     }
     
     fclose(file);
     return success;
 }
 
 /*
  * Save current global styles to a file
  */
 bool labfont_renderer_save_stylefile(labfont_renderer* renderer, const char* path) {
     // This function would need to iterate through the global styles
     // and write them to a file. However, the style manager doesn't
     // currently expose a way to iterate through styles or get their
     // definitions, so we'd need to extend it.
     
     labfont_renderer_set_error("Save stylefile not implemented");
     return false;
 }
 
 /*
  * Get the style manager from the renderer
  */
 labfont_style_manager* labfont_renderer_get_style_manager(labfont_renderer* renderer) {
     if (!renderer) return NULL;
     return renderer->global_styles;
 }
 
 /*
  * Clear the LabFontState cache
  */
 void labfont_renderer_clear_cache(labfont_renderer* renderer) {
     if (!renderer) return;
     
     for (size_t i = 0; i < renderer->cache_size; i++) {
         labfont_style_destroy(renderer->state_cache[i].style);
         // Here you would free the LabFontState, but the current API
         // doesn't seem to provide a way to free these objects.
         // If your API supports it, you should add:
         // YourFunctionToFreeLabFontState(renderer->state_cache[i].font_state);
     }
     
     renderer->cache_size = 0;
 }
 
 /*
  * Set the maximum size of the LabFontState cache
  */
 void labfont_renderer_set_cache_size(labfont_renderer* renderer, size_t max_entries) {
     if (!renderer) return;
     
     renderer->max_cache_entries = max_entries;
     
     // If the new limit is smaller than the current cache size,
     // remove oldest entries until we're within limits
     if (max_entries > 0 && renderer->cache_size > max_entries) {
         size_t entries_to_remove = renderer->cache_size - max_entries;
         
         // Remove oldest entries (from the beginning of the array)
         for (size_t i = 0; i < entries_to_remove; i++) {
             labfont_style_destroy(renderer->state_cache[i].style);
             // Free LabFontState if API supports it
         }
         
         // Move remaining entries to beginning of array
         memmove(renderer->state_cache, 
                 renderer->state_cache + entries_to_remove, 
                 (renderer->cache_size - entries_to_remove) * sizeof(labfont_state_cache_entry));
         
         renderer->cache_size -= entries_to_remove;
     }
 }
 
 /*
  * Helper to find or create a LabFontState for a style
  */
 static LabFontState* labfont_renderer_get_font_state(labfont_renderer* renderer, 
                                                    const labfont_style* style) {
     if (!renderer || !style) return NULL;
     
     // Check if we have this style in the cache
     for (size_t i = 0; i < renderer->cache_size; i++) {
         if (labfont_style_equals(style, renderer->state_cache[i].style)) {
             return renderer->state_cache[i].font_state;
         }
     }
     
     // Not found, create a new LabFontState
     
     // Get required style properties with defaults
     const char* font_name = style->has_property[LABFONT_PROP_FONT] ? 
                           style->properties[LABFONT_PROP_FONT].string_val : "sans-normal";
     
     float size = style->has_property[LABFONT_PROP_SIZE] ? 
                style->properties[LABFONT_PROP_SIZE].float_val : 12.0f;
     
     int style_flags = style->has_property[LABFONT_PROP_STYLE] ? 
                     style->properties[LABFONT_PROP_STYLE].int_val : LABFONT_STYLE_NORMAL;
     
     labfont_color color = style->has_property[LABFONT_PROP_COLOR] ? 
                         style->properties[LABFONT_PROP_COLOR].color_val : 
                         (labfont_color){255, 255, 255, 255};
     
     int alignment = style->has_property[LABFONT_PROP_ALIGNMENT] ? 
                    style->properties[LABFONT_PROP_ALIGNMENT].int_val : 
                    (LABFONT_ALIGN_LEFT | LABFONT_ALIGN_BASELINE);
     
     float spacing = style->has_property[LABFONT_PROP_SPACING] ? 
                   style->properties[LABFONT_PROP_SPACING].float_val : 0.0f;
     
     float blur = style->has_property[LABFONT_PROP_BLUR] ? 
                style->properties[LABFONT_PROP_BLUR].float_val : 0.0f;
     
     // Find the right font based on name and style
     // This part is specific to your API and might need modification
     struct LabFont* font = NULL;
     
     if (font_name) {
         // Try to get the font by name
         font = LabFontGet(font_name);
         
         // If not found, try to find a font with style suffix
         if (!font) {
             // Append style suffixes based on style flags
             char font_name_with_style[256];
             
             if (style_flags & LABFONT_STYLE_BOLD) {
                 snprintf(font_name_with_style, sizeof(font_name_with_style), "%s-bold", font_name);
                 font = LabFontGet(font_name_with_style);
             }
             
             if (!font && (style_flags & LABFONT_STYLE_ITALIC)) {
                 snprintf(font_name_with_style, sizeof(font_name_with_style), "%s-italic", font_name);
                 font = LabFontGet(font_name_with_style);
             }
             
             if (!font && (style_flags & LABFONT_STYLE_BOLD) && (style_flags & LABFONT_STYLE_ITALIC)) {
                 snprintf(font_name_with_style, sizeof(font_name_with_style), "%s-bolditalic", font_name);
                 font = LabFontGet(font_name_with_style);
             }
         }
     }
     
     // If we still don't have a font, use a default one
     if (!font) {
         font = LabFontGet("sans-normal");
         if (!font) {
             labfont_renderer_set_error("Failed to find any usable font");
             return NULL;
         }
     }
     
     // Convert colors and alignment to LabFont format
     struct LabFontColor lab_color = {{color.r, color.g, color.b, color.a}};
     struct LabFontAlign lab_alignment = {alignment};
     
     // Create the LabFontState
     LabFontState* font_state = LabFontStateBake(font, size, lab_color, lab_alignment, spacing, blur);
     
     if (!font_state) {
         labfont_renderer_set_error("Failed to create LabFontState");
         return NULL;
     }
     
     // Add to cache if we have room
     if (renderer->max_cache_entries == 0 || renderer->cache_size < renderer->max_cache_entries) {
         // Grow cache if needed
         if (renderer->cache_size == renderer->cache_capacity) {
             size_t new_capacity = renderer->cache_capacity == 0 ? 16 : renderer->cache_capacity * 2;
             
             // Limit to max cache size if set
             if (renderer->max_cache_entries > 0 && new_capacity > renderer->max_cache_entries) {
                 new_capacity = renderer->max_cache_entries;
             }
             
             labfont_state_cache_entry* new_cache = (labfont_state_cache_entry*)realloc(
                 renderer->state_cache, new_capacity * sizeof(labfont_state_cache_entry));
             
             if (!new_cache) {
                 labfont_renderer_set_error("Failed to resize state cache");
                 // Not a fatal error, we just won't cache this state
             } else {
                 renderer->state_cache = new_cache;
                 renderer->cache_capacity = new_capacity;
             }
         }
         
         // Add to cache if we have capacity
         if (renderer->cache_size < renderer->cache_capacity) {
             renderer->state_cache[renderer->cache_size].style = labfont_style_copy(style);
             renderer->state_cache[renderer->cache_size].font_state = font_state;
             renderer->cache_size++;
         }
     } else if (renderer->cache_size > 0) {
         // Cache is full, replace oldest entry
         labfont_style_destroy(renderer->state_cache[0].style);
         // Free old LabFontState if API supports it
         
         // Move all entries down
         memmove(renderer->state_cache, 
                 renderer->state_cache + 1, 
                 (renderer->cache_size - 1) * sizeof(labfont_state_cache_entry));
         
         // Add new entry at the end
         renderer->state_cache[renderer->cache_size - 1].style = labfont_style_copy(style);
         renderer->state_cache[renderer->cache_size - 1].font_state = font_state;
     }
     
     return font_state;
 }
 
 /*
  * Push a style onto the stack
  */
 static bool labfont_renderer_push_style(labfont_renderer* renderer, const labfont_style* style) {
     if (!renderer || !style) return false;
     
     // Grow stack if needed
     if (renderer->style_stack.size == renderer->style_stack.capacity) {
         size_t new_capacity = renderer->style_stack.capacity == 0 ? 8 : renderer->style_stack.capacity * 2;
         
         labfont_style** new_stack = (labfont_style**)realloc(
             renderer->style_stack.styles, new_capacity * sizeof(labfont_style*));
         
         if (!new_stack) {
             labfont_renderer_set_error("Failed to resize style stack");
             return false;
         }
         
         renderer->style_stack.styles = new_stack;
         renderer->style_stack.capacity = new_capacity;
     }
     
     // Copy the style and push it
     renderer->style_stack.styles[renderer->style_stack.size++] = labfont_style_copy(style);
     
     return true;
 }
 
 /*
  * Pop a style from the stack
  */
 static labfont_style* labfont_renderer_pop_style(labfont_renderer* renderer) {
     if (!renderer || renderer->style_stack.size == 0) return NULL;
     
     // Return the top style (caller is responsible for freeing it)
     return renderer->style_stack.styles[--renderer->style_stack.size];
 }
 
 /*
  * Get the current active style (top of stack)
  */
 static labfont_style* labfont_renderer_get_active_style(labfont_renderer* renderer) {
     if (!renderer || renderer->style_stack.size == 0) return NULL;
     
     return renderer->style_stack.styles[renderer->style_stack.size - 1];
 }
 
 /*
  * Process a style definition token
  */
 static bool labfont_renderer_process_style_def(labfont_renderer* renderer, 
                                              labfont_token* token,
                                              labfont_style_manager* local_styles) {
     if (!renderer || !token || token->type != LABFONT_TOKEN_STYLE_DEF) return false;
     
     // Parse the style definition
     labfont_style* style = labfont_style_create();
     if (!style) {
         labfont_renderer_set_error("Failed to create style");
         return false;
     }
     
     if (!labfont_style_parse(token->props, style, renderer->global_styles)) {
         labfont_renderer_set_error("Failed to parse style definition: %s", labfont_parser_get_last_error());
         labfont_style_destroy(style);
         return false;
     }
     
     // Add to local styles
     bool result = labfont_style_manager_define(local_styles, token->name, style);
     
     // Push onto stack
     if (result) {
         result = labfont_renderer_push_style(renderer, style);
     }
     
     // Clean up
     labfont_style_destroy(style);
     
     return result;
 }
 
 /*
  * Process a style reference token
  */
 static bool labfont_renderer_process_style_ref(labfont_renderer* renderer, 
                                              labfont_token* token,
                                              labfont_style_manager* local_styles) {
     if (!renderer || !token || token->type != LABFONT_TOKEN_STYLE_REF) return false;
     
     // Look up the style in local styles first, then global
     labfont_style* style = labfont_style_manager_get(local_styles, token->name);
     
     if (!style) {
         // Not found in local styles, try global
         style = labfont_style_manager_get(renderer->global_styles, token->name);
         
         if (!style) {
             labfont_renderer_set_error("Style not found: %s", token->name);
             return false;
         }
     }
     // Push style onto stack
    return labfont_renderer_push_style(renderer, style);
}

/*
 * Process a global style reference token
 */
static bool labfont_renderer_process_global_ref(labfont_renderer* renderer, 
                                             labfont_token* token) {
    if (!renderer || !token || token->type != LABFONT_TOKEN_GLOBAL_REF) return false;
    
    // Look up the style in global styles
    labfont_style* style = labfont_style_manager_get(renderer->global_styles, token->name);
    
    if (!style) {
        labfont_renderer_set_error("Global style not found: %s", token->name);
        return false;
    }
    
    // Push style onto stack
    return labfont_renderer_push_style(renderer, style);
}

/*
 * Process a style properties token
 */
static bool labfont_renderer_process_style_props(labfont_renderer* renderer, 
                                               labfont_token* token) {
    if (!renderer || !token || token->type != LABFONT_TOKEN_STYLE_PROPS) return false;
    
    // Get current style or create a new one
    labfont_style* current = labfont_renderer_get_active_style(renderer);
    labfont_style* new_style = current ? labfont_style_copy(current) : labfont_style_create();
    
    if (!new_style) {
        labfont_renderer_set_error("Failed to create style");
        return false;
    }
    
    // Parse and apply properties
    if (!labfont_style_parse(token->props, new_style, renderer->global_styles)) {
        labfont_renderer_set_error("Failed to parse style properties: %s", labfont_parser_get_last_error());
        labfont_style_destroy(new_style);
        return false;
    }
    
    // Push onto stack
    bool result = labfont_renderer_push_style(renderer, new_style);
    
    // Clean up
    labfont_style_destroy(new_style);
    
    return result;
}

/*
 * Process a shorthand token
 */
static bool labfont_renderer_process_shorthand(labfont_renderer* renderer, 
                                             labfont_token* token) {
    if (!renderer || !token || token->type != LABFONT_TOKEN_SHORTHAND) return false;
    
    // Get current style or create a new one
    labfont_style* current = labfont_renderer_get_active_style(renderer);
    labfont_style* new_style = current ? labfont_style_copy(current) : labfont_style_create();
    
    if (!new_style) {
        labfont_renderer_set_error("Failed to create style");
        return false;
    }
    
    // Apply shorthand style
    switch (token->shorthand) {
        case 'b': // Bold
            new_style->properties[LABFONT_PROP_STYLE].int_val |= LABFONT_STYLE_BOLD;
            new_style->has_property[LABFONT_PROP_STYLE] = true;
            break;
            
        case 'i': // Italic
            new_style->properties[LABFONT_PROP_STYLE].int_val |= LABFONT_STYLE_ITALIC;
            new_style->has_property[LABFONT_PROP_STYLE] = true;
            break;
            
        case 'u': // Underline
            new_style->properties[LABFONT_PROP_STYLE].int_val |= LABFONT_STYLE_UNDERLINE;
            new_style->has_property[LABFONT_PROP_STYLE] = true;
            break;
            
        case 'c': // Color
            if (token->value) {
                if (!labfont_parse_color_hex(token->value, &new_style->properties[LABFONT_PROP_COLOR].color_val)) {
                    labfont_renderer_set_error("Invalid color format: %s", token->value);
                    labfont_style_destroy(new_style);
                    return false;
                }
                new_style->has_property[LABFONT_PROP_COLOR] = true;
            }
            break;
            
        default:
            labfont_renderer_set_error("Unknown shorthand: %c", token->shorthand);
            labfont_style_destroy(new_style);
            return false;
    }
    
    // Push onto stack
    bool result = labfont_renderer_push_style(renderer, new_style);
    
    // Clean up
    labfont_style_destroy(new_style);
    
    return result;
}

/*
 * Process a style pop token
 */
static bool labfont_renderer_process_style_pop(labfont_renderer* renderer, 
                                             labfont_token* token) {
    if (!renderer || !token || token->type != LABFONT_TOKEN_STYLE_POP) return false;
    
    // Check if we have a specific style to pop
    if (token->name && *token->name) {
        // Pop styles until we find the named one
        labfont_style* popped = NULL;
        bool found = false;
        
        while (renderer->style_stack.size > 0) {
            popped = labfont_renderer_pop_style(renderer);
            
            // For shorthand styles, the name is the shorthand character
            if (token->name[0] == 'b' || token->name[0] == 'i' || 
                token->name[0] == 'u' || token->name[0] == 'c') {
                // For shorthand pops, we just pop once - the named check isn't reliable
                found = true;
                labfont_style_destroy(popped);
                break;
            }
            
            // TODO: This would need to check if the style matches the name,
            // but we don't store the name with the style. This would require
            // changes to our style stack implementation.
            found = true; // For now, just assume it's found
            labfont_style_destroy(popped);
            break;
        }
        
        if (!found) {
            labfont_renderer_set_error("Style not found for pop: %s", token->name);
            return false;
        }
    } else {
        // Just pop the top style
        labfont_style* popped = labfont_renderer_pop_style(renderer);
        if (popped) {
            labfont_style_destroy(popped);
        } else {
            labfont_renderer_set_error("No style to pop");
            return false;
        }
    }
    
    return true;
}

/*
 * Ensure we have enough temporary buffer space
 */
static bool labfont_renderer_ensure_temp_buffer(labfont_renderer* renderer, size_t size) {
    if (!renderer) return false;
    
    if (renderer->temp_buffer_size < size) {
        char* new_buffer = (char*)realloc(renderer->temp_buffer, size);
        if (!new_buffer) {
            labfont_renderer_set_error("Failed to allocate temporary buffer");
            return false;
        }
        
        renderer->temp_buffer = new_buffer;
        renderer->temp_buffer_size = size;
    }
    
    return true;
}

/*
 * Handle line wrapping
 */
static bool labfont_renderer_handle_wrap(labfont_renderer* renderer, 
                                       const char* text, 
                                       LabFontState* font_state,
                                       const labfont_layout_options* options,
                                       float* text_width,
                                       bool drawing) {
    if (!renderer || !text || !font_state || !options || !text_width) return false;
    
    // If no wrapping, just measure the text
    if (options->wrap_width <= 0.0f) {
        *text_width = LabFontMeasure(text, font_state).width;
        return true;
    }
    
    // Check if text fits in remaining space
    LabFontSize text_size = LabFontMeasure(text, font_state);
    *text_width = text_size.width;
    
    // If it fits, no wrapping needed
    float remaining_width = options->wrap_width - (renderer->layout.x - renderer->layout.line_start_x);
    if (*text_width <= remaining_width) {
        return true;
    }
    
    // Text doesn't fit, need to find a break point
    const char* p = text;
    const char* last_break = text;
    const char* last_space = NULL;
    float width_so_far = 0.0f;
    
    while (*p) {
        if (isspace(*p)) {
            last_space = p;
        }
        
        // Measure up to this point
        LabFontSize size = LabFontMeasureSubstring(text, p + 1, font_state);
        
        // If we've gone too far, break at the last space
        if (size.width > remaining_width) {
            if (last_space) {
                // Break at last space
                last_break = last_space;
                width_so_far = LabFontMeasureSubstring(text, last_break, font_state).width;
            } else {
                // No space found, break at current position
                last_break = p;
                width_so_far = size.width;
            }
            break;
        }
        
        p++;
    }
    
    // If we made it through the entire text, no need to wrap
    if (!*p) {
        return true;
    }
    
    // We need to wrap, set the width to what fits on this line
    *text_width = width_so_far;
    
    // If we're drawing, actually handle the line break
    if (drawing) {
        // Move to next line
        renderer->layout.x = renderer->layout.line_start_x;
        renderer->layout.y += text_size.height * options->line_height;
        renderer->layout.line_count++;
        
        // Check if we've exceeded max lines
        if (options->max_lines > 0 && renderer->layout.line_count >= options->max_lines) {
            renderer->layout.truncated = true;
            return false;
        }
    }
    
    return true;
}

/*
 * Draw text with the current style
 */
static labfont_xy labfont_renderer_draw_text_segment(labfont_renderer* renderer,
                                                  LabFontDrawState* draw_state,
                                                  const char* text,
                                                  const labfont_layout_options* options,
                                                  bool measure_only) {
    if (!renderer || !text) {
        return (labfont_xy){renderer->layout.x, renderer->layout.y};
    }
    
    // Get current style or use default
    labfont_style* style = labfont_renderer_get_active_style(renderer);
    if (!style) {
        // Create a default style
        style = labfont_style_create();
        labfont_renderer_push_style(renderer, style);
        labfont_style_destroy(style);
        style = labfont_renderer_get_active_style(renderer);
    }
    
    // Get LabFontState for current style
    LabFontState* font_state = labfont_renderer_get_font_state(renderer, style);
    if (!font_state) {
        return (labfont_xy){renderer->layout.x, renderer->layout.y};
    }
    
    // Measure the text to see if it fits
    float text_width = 0.0f;
    
    if (options && options->wrap_width > 0.0f) {
        if (!labfont_renderer_handle_wrap(renderer, text, font_state, options, &text_width, !measure_only)) {
            // Truncated or error
            return (labfont_xy){renderer->layout.x, renderer->layout.y};
        }
    } else {
        // Just measure the whole text
        text_width = LabFontMeasure(text, font_state).width;
    }
    
    // Draw the text if not just measuring
    if (!measure_only && draw_state) {
        renderer->layout.x = LabFontDraw(draw_state, text, renderer->layout.x, renderer->layout.y, font_state);
    } else {
        renderer->layout.x += text_width;
    }
    
    // Update line height if this text is taller
    LabFontSize text_size = LabFontMeasure(text, font_state);
    if (text_size.height > renderer->layout.line_height) {
        renderer->layout.line_height = text_size.height;
    }
    
    return (labfont_xy){renderer->layout.x, renderer->layout.y};
}

/*
 * Render a markup string
 */
static labfont_xy labfont_renderer_render_markup(labfont_renderer* renderer,
                                              LabFontDrawState* draw_state,
                                              float x, float y,
                                              const char* markup_text,
                                              const labfont_layout_options* options,
                                              bool measure_only) {
    if (!renderer || !markup_text) {
        labfont_xy result = {x, y};
        return result;
    }
    
    // Set up layout state
    labfont_renderer_reset_layout(renderer, x, y);
    
    // Create local style manager for this rendering session
    labfont_style_manager* local_styles = labfont_style_manager_create();
    if (!local_styles) {
        labfont_renderer_set_error("Failed to create local style manager");
        labfont_xy result = {x, y};
        return result;
    }
    
    // Parse the markup
    labfont_markup_result* result = labfont_parse_markup(markup_text);
    if (!result) {
        labfont_renderer_set_error("Failed to parse markup: %s", labfont_parser_get_last_error());
        labfont_style_manager_destroy(local_styles);
        labfont_xy xy_result = {x, y};
        return xy_result;
    }
    
    if (result->has_error) {
        labfont_renderer_set_error("Markup error: %s", result->error_msg);
        labfont_free_markup_result(result);
        labfont_style_manager_destroy(local_styles);
        labfont_xy xy_result = {x, y};
        return xy_result;
    }
    
    // Process tokens
    for (size_t i = 0; i < result->num_tokens; i++) {
        labfont_token* token = &result->tokens[i];
        
        // Stop if we've hit max lines and are truncated
        if (options && options->max_lines > 0 && renderer->layout.truncated) {
            // Add ellipsis if requested
            if (options->ellipsis && !measure_only && draw_state) {
                labfont_style* style = labfont_renderer_get_active_style(renderer);
                if (style) {
                    LabFontState* font_state = labfont_renderer_get_font_state(renderer, style);
                    if (font_state) {
                        LabFontDraw(draw_state, "...", renderer->layout.x, renderer->layout.y, font_state);
                    }
                }
            }
            break;
        }
        
        // Process token based on type
        switch (token->type) {
            case LABFONT_TOKEN_TEXT:
                // Extract text string
                {
                    size_t text_len = token->end - token->start;
                    if (!labfont_renderer_ensure_temp_buffer(renderer, text_len + 1)) {
                        labfont_free_markup_result(result);
                        labfont_style_manager_destroy(local_styles);
                        labfont_xy xy_result = {x, y};
                        return xy_result;
                    }
                    
                    memcpy(renderer->temp_buffer, token->start, text_len);
                    renderer->temp_buffer[text_len] = '\0';
                    
                    // Draw or measure the text
                    labfont_renderer_draw_text_segment(renderer, draw_state, renderer->temp_buffer, options, measure_only);
                }
                break;
                
            case LABFONT_TOKEN_STYLE_DEF:
                labfont_renderer_process_style_def(renderer, token, local_styles);
                break;
                
            case LABFONT_TOKEN_STYLE_REF:
                labfont_renderer_process_style_ref(renderer, token, local_styles);
                break;
                
            case LABFONT_TOKEN_GLOBAL_REF:
                labfont_renderer_process_global_ref(renderer, token);
                break;
                
            case LABFONT_TOKEN_STYLE_PROPS:
                labfont_renderer_process_style_props(renderer, token);
                break;
                
            case LABFONT_TOKEN_SHORTHAND:
                labfont_renderer_process_shorthand(renderer, token);
                break;
                
            case LABFONT_TOKEN_STYLE_POP:
                labfont_renderer_process_style_pop(renderer, token);
                break;
        }
    }
    
    // Clean up
    labfont_free_markup_result(result);
    
    // Clear local styles
    labfont_style_manager_destroy(local_styles);
    
    // Clear style stack
    while (renderer->style_stack.size > 0) {
        labfont_style* style = labfont_renderer_pop_style(renderer);
        labfont_style_destroy(style);
    }
    
    // Return final position
    labfont_xy xy_result = {renderer->layout.x, renderer->layout.y};
    return xy_result;
}

/*
 * Measure rich text dimensions without rendering
 */
labfont_text_metrics labfont_renderer_measure_text(labfont_renderer* renderer, 
                                                 const char* markup_text,
                                                 const labfont_layout_options* options) {
    // Default result
    labfont_text_metrics metrics = {0};
    
    if (!renderer || !markup_text) {
        return metrics;
    }
    
    // Use default options if none provided
    labfont_layout_options default_options = {0};
    default_options.wrap_width = 0.0f; // No wrapping
    default_options.line_height = 1.2f; // Default line height
    default_options.max_lines = 0;     // No limit
    default_options.ellipsis = false;  // No ellipsis
    
    const labfont_layout_options* opts = options ? options : &default_options;
    
    // Render in measuring mode
    float start_x = 0.0f;
    float start_y = 0.0f;
    
    labfont_xy end_pos = labfont_renderer_render_markup(renderer, NULL, start_x, start_y, 
                                                     markup_text, opts, true);
    
    // Fill in metrics
    metrics.width = end_pos.x - start_x;
    metrics.height = end_pos.y - start_y + renderer->layout.line_height;
    metrics.line_count = renderer->layout.line_count + 1; // +1 for the current line
    metrics.truncated = renderer->layout.truncated;
    
    return metrics;
}

/*
 * Draw rich text on screen
 */
labfont_xy labfont_renderer_draw_text(labfont_renderer* renderer,
                                    LabFontDrawState* draw_state,
                                    float x, float y,
                                    const char* markup_text,
                                    const labfont_layout_options* options) {
    if (!renderer || !draw_state || !markup_text) {
        labfont_xy result = {x, y};
        return result;
    }
    
    // Use default options if none provided
    labfont_layout_options default_options = {0};
    default_options.wrap_width = 0.0f; // No wrapping
    default_options.line_height = 1.2f; // Default line height
    default_options.max_lines = 0;     // No limit
    default_options.ellipsis = false;  // No ellipsis
    
    const labfont_layout_options* opts = options ? options : &default_options;
    
    // Render with drawing
    return labfont_renderer_render_markup(renderer, draw_state, x, y, markup_text, opts, false);
}

/*
 * Printf-style rich text drawing
 */
labfont_xy labfont_renderer_printf(labfont_renderer* renderer,
                                  LabFontDrawState* draw_state,
                                  float x, float y,
                                  const labfont_layout_options* options,
                                  const char* format, ...) {
    if (!renderer || !draw_state || !format) {
        labfont_xy result = {x, y};
        return result;
    }
    
    // Format the string
    va_list args;
    va_start(args, format);
    
    // Get required buffer size
    va_list args_copy;
    va_copy(args_copy, args);
    int buffer_size = vsnprintf(NULL, 0, format, args_copy) + 1;
    va_end(args_copy);
    
    if (buffer_size <= 0) {
        va_end(args);
        labfont_xy result = {x, y};
        return result;
    }
    
    // Ensure we have enough buffer space
    if (!labfont_renderer_ensure_temp_buffer(renderer, buffer_size)) {
        va_end(args);
        labfont_xy result = {x, y};
        return result;
    }
    
    // Format the string
    vsnprintf(renderer->temp_buffer, buffer_size, format, args);
    va_end(args);
    
    // Draw the formatted string
    return labfont_renderer_draw_text(renderer, draw_state, x, y, 
                                    renderer->temp_buffer, options);
}

/*
 * Utility to convert a standard LabFont alignment to labfont_style
 */
bool labfont_renderer_set_alignment(labfont_style* style, int lab_font_alignment) {
    if (!style) return false;
    
    style->properties[LABFONT_PROP_ALIGNMENT].int_val = lab_font_alignment;
    style->has_property[LABFONT_PROP_ALIGNMENT] = true;
    
    return true;
}

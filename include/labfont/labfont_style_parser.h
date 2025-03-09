/**
 * labfont_style_parser.h - Style parsing for rich text rendering
 * 
 * A single-header library for parsing style definitions and markup text.
 * This handles the parsing of style definitions like:
 *   "font=sans-normal size=24 color=#FF0000"
 * And markup text containing style directives like:
 *   "{normal: font=sans-normal size=24}Hello {b}world{/b}"
 *
 */

 /*
 How to use this library:
 
 1. Include this header in your project:
    #include "labfont_style_parser.h"
 
 2. In exactly one source file, define the implementation:
    #define LABFONT_STYLE_PARSER_IMPLEMENTATION
    #include "labfont_style_parser.h"
 
 3. Optionally, run the examples:
    #define LABFONT_STYLE_PARSER_IMPLEMENTATION
    #define LABFONT_STYLE_PARSER_EXAMPLES
    #include "labfont_style_parser.h"
    
 Basic usage:
 
 // Create a style manager
 labfont_style_manager* manager = labfont_style_manager_create();
 
 // Define styles
 labfont_style* base_style = labfont_style_create();
 labfont_style_parse("font=sans-normal size=16 color=#333333", base_style, NULL);
 labfont_style_manager_define(manager, "base", base_style);
 
 // Parse markup
 const char* markup = "{base}Hello {b}World{/b}!";
 labfont_markup_result* result = labfont_parse_markup(markup);
 
 // Process the tokens
 for (size_t i = 0; i < result->num_tokens; i++) {
     labfont_token* token = &result->tokens[i];
     
     // Process each token based on its type...
 }
 
 // Clean up
 labfont_free_markup_result(result);
 labfont_style_destroy(base_style);
 labfont_style_manager_destroy(manager);
 */


 #ifndef LABFONT_STYLE_PARSER_H
 #define LABFONT_STYLE_PARSER_H
 
 #include <stdint.h>
 #include <stdbool.h>
 #include <stddef.h>
 
 #ifdef __cplusplus
 extern "C" {
 #endif
 
 /*
  * Type definitions
  */
 
 /* Style property types */
 typedef enum {
     LABFONT_PROP_NONE = 0,
     LABFONT_PROP_FONT,       /* Font name (string) */
     LABFONT_PROP_SIZE,       /* Font size (float) */
     LABFONT_PROP_COLOR,      /* Text color (color) */
     LABFONT_PROP_BGCOLOR,    /* Background color (color) */
     LABFONT_PROP_ALIGNMENT,  /* Text alignment (int flags) */
     LABFONT_PROP_SPACING,    /* Character spacing (float) */
     LABFONT_PROP_BLUR,       /* Text blur (float) */
     LABFONT_PROP_WEIGHT,     /* Font weight (int) */
     LABFONT_PROP_STYLE,      /* Font style (int flags) */
     LABFONT_PROP_INHERIT,    /* Inherit from style (string) */
     LABFONT_PROP_COUNT       /* Number of property types */
 } labfont_property_type;
 
 /* Color with RGBA components */
 typedef struct {
     uint8_t r, g, b, a;
 } labfont_color;
 
 /* Font style flags */
 #define LABFONT_STYLE_NORMAL    0
 #define LABFONT_STYLE_BOLD      (1 << 0)
 #define LABFONT_STYLE_ITALIC    (1 << 1)
 #define LABFONT_STYLE_UNDERLINE (1 << 2)
 
 /* Alignment flags (same as in main header) */
 #define LABFONT_ALIGN_TOP       (1 << 0)
 #define LABFONT_ALIGN_MIDDLE    (1 << 1)
 #define LABFONT_ALIGN_BASELINE  (1 << 2)
 #define LABFONT_ALIGN_BOTTOM    (1 << 3)
 #define LABFONT_ALIGN_LEFT      (1 << 4)
 #define LABFONT_ALIGN_CENTER    (1 << 5)
 #define LABFONT_ALIGN_RIGHT     (1 << 6)
 
 /* Property value variant */
 typedef struct {
     labfont_property_type type;
     union {
         char* string_val;     /* For font, inherit */
         float float_val;      /* For size, spacing, blur */
         int int_val;          /* For alignment, weight, style */
         labfont_color color_val; /* For color, bgcolor */
     };
 } labfont_property_value;
 
 /* Style structure with all possible properties */
 typedef struct {
     labfont_property_value properties[LABFONT_PROP_COUNT];
     bool has_property[LABFONT_PROP_COUNT];
 } labfont_style;
 
 /* Forward declaration for the style manager */
 typedef struct labfont_style_manager labfont_style_manager;
 
 /* Token types from markup parsing */
 typedef enum {
     LABFONT_TOKEN_TEXT,        /* Regular text */
     LABFONT_TOKEN_STYLE_DEF,   /* Style definition {name: props} */
     LABFONT_TOKEN_STYLE_REF,   /* Style reference {name} */
     LABFONT_TOKEN_GLOBAL_REF,  /* Global style reference {@name} */
     LABFONT_TOKEN_STYLE_POP,   /* Style pop {/} or {/name} */
     LABFONT_TOKEN_STYLE_PROPS, /* Inline properties {props} */
     LABFONT_TOKEN_SHORTHAND,   /* Shorthand like {b} {i} {c=#rrggbb} */
 } labfont_token_type;
 
 /* Token from markup parsing */
 typedef struct {
     labfont_token_type type;
     const char* start;        /* Start of token in original string */
     const char* end;          /* End of token in original string */
     
     /* For STYLE_DEF, STYLE_REF, GLOBAL_REF, STYLE_POP */
     char* name;
     
     /* For STYLE_DEF, STYLE_PROPS */
     char* props;
     
     /* For SHORTHAND */
     char shorthand;
     char* value;              /* For {c=#rrggbb} */
 } labfont_token;
 
 /* Markup parsing result */
 typedef struct {
     labfont_token* tokens;
     size_t num_tokens;
     size_t capacity;
     
     /* Error handling */
     bool has_error;
     char error_msg[256];
     const char* error_pos;
 } labfont_markup_result;
 
 /*
  * Style management functions
  */
 
 /* Create a style manager for tracking named styles */
 labfont_style_manager* labfont_style_manager_create(void);
 
 /* Destroy a style manager and free all associated resources */
 void labfont_style_manager_destroy(labfont_style_manager* manager);
 
 /* Define a named style in the manager */
 bool labfont_style_manager_define(labfont_style_manager* manager, 
                                  const char* name, 
                                  const labfont_style* style);
 
 /* Get a named style from the manager */
 labfont_style* labfont_style_manager_get(labfont_style_manager* manager, 
                                          const char* name);
 
 /* Remove a named style from the manager */
 bool labfont_style_manager_remove(labfont_style_manager* manager, 
                                  const char* name);
 
 /* Check if a style exists in the manager */
 bool labfont_style_manager_has(labfont_style_manager* manager, 
                               const char* name);
 
 /* Clear all styles from the manager */
 void labfont_style_manager_clear(labfont_style_manager* manager);
 
 /*
  * Style parsing functions
  */
 
 /* Initialize a new empty style */
 void labfont_style_init(labfont_style* style);
 
 /* Create a new style */
 labfont_style* labfont_style_create(void);
 
 /* Free a style */
 void labfont_style_destroy(labfont_style* style);
 
 /* Clone a style */
 labfont_style* labfont_style_clone(const labfont_style* style);
 
 /* Parse a style definition string into a style object */
 bool labfont_style_parse(const char* style_def, labfont_style* style, 
                         labfont_style_manager* manager);
 
 /* Parse property name from a string */
 labfont_property_type labfont_parse_property_name(const char* name);
 
 /* Parse a color from hex string (#RGB, #RGBA, #RRGGBB, #RRGGBBAA) */
 bool labfont_parse_color_hex(const char* hex, labfont_color* color);
 
 /* Parse an alignment from string (e.g., "top", "center", "bottom|left") */
 bool labfont_parse_alignment(const char* align_str, int* alignment);
 
 /* Apply properties from source to destination style */
 void labfont_style_apply(labfont_style* dest, const labfont_style* src);
 
 /* Apply inheritance - resolve all inherit=X properties */
 bool labfont_style_resolve_inheritance(labfont_style* style, 
                                      labfont_style_manager* manager,
                                      int max_depth);
 
 /*
  * Markup parsing functions
  */
 
 /* Parse markup text into tokens */
 labfont_markup_result* labfont_parse_markup(const char* markup);
 
 /* Free a markup parsing result */
 void labfont_free_markup_result(labfont_markup_result* result);
 
 /*
  * Utility functions
  */
 
 /* Get the last error message */
 const char* labfont_parser_get_last_error(void);
 
 /* Clear the last error */
 void labfont_parser_clear_error(void);
 
 #ifdef __cplusplus
 }
 #endif
 

 /*
  * Implementation
  */
 #ifdef LABFONT_STYLE_PARSER_IMPLEMENTATION
 
 #include <stdlib.h>
 #include <string.h>
 #include <stdio.h>
 #include <ctype.h>
 #include <stdarg.h>
 #include <assert.h>

 /* Error handling */
 static char labfont_last_error[256] = {0};
 
 const char* labfont_parser_get_last_error(void) {
     return labfont_last_error;
 }
 
 void labfont_parser_clear_error(void) {
     labfont_last_error[0] = '\0';
 }
 
 static void labfont_set_error(const char* format, ...) {
     va_list args;
     va_start(args, format);
     vsnprintf(labfont_last_error, sizeof(labfont_last_error), format, args);
     va_end(args);
 }
 
 /* String utilities */
 static char* labfont_strdup(const char* str) {
     if (!str) return NULL;
     
     size_t len = strlen(str);
     char* dup = (char*)malloc(len + 1);
     
     if (dup) {
         memcpy(dup, str, len + 1);
     }
     
     return dup;
 }
 
 static char* labfont_strndup(const char* str, size_t n) {
     if (!str) return NULL;
     
     size_t len = 0;
     while (len < n && str[len]) len++;
     
     char* dup = (char*)malloc(len + 1);
     
     if (dup) {
         memcpy(dup, str, len);
         dup[len] = '\0';
     }
     
     return dup;
 }
 
 static void labfont_str_trim(char* str) {
     if (!str) return;
     
     // Trim end
     char* end = str + strlen(str) - 1;
     while (end >= str && isspace(*end)) {
         *end-- = '\0';
     }
     
     // Trim start
     char* start = str;
     while (*start && isspace(*start)) {
         start++;
     }
     
     if (start > str) {
         memmove(str, start, strlen(start) + 1);
     }
 }
 
typedef struct {
    char* name;
    labfont_style* style;
} labfont_style_entry;

/* Style manager implementation */
struct labfont_style_manager {
    labfont_style_entry *styles;
    size_t num_styles;
    size_t capacity;
};
 
 labfont_style_manager* labfont_style_manager_create(void) {
     labfont_style_manager* manager = (labfont_style_manager*)malloc(sizeof(labfont_style_manager));
     if (!manager) {
         labfont_set_error("Failed to allocate style manager");
         return NULL;
     }
     
     manager->styles = NULL;
     manager->num_styles = 0;
     manager->capacity = 0;
     
     return manager;
 }
 
 void labfont_style_manager_destroy(labfont_style_manager* manager) {
     if (!manager) return;
     
     for (size_t i = 0; i < manager->num_styles; i++) {
         free(manager->styles[i].name);
         labfont_style_destroy(manager->styles[i].style);
     }
     
     free(manager->styles);
     free(manager);
 }
 


 bool labfont_style_manager_define(labfont_style_manager* manager, 
                                  const char* name, 
                                  const labfont_style* style) {
     if (!manager || !name || !style) {
         labfont_set_error("Invalid parameters for style_manager_define");
         return false;
     }
     
     // Check if style already exists, if so, replace it
     for (size_t i = 0; i < manager->num_styles; i++) {
         if (strcmp(manager->styles[i].name, name) == 0) {
             labfont_style_destroy(manager->styles[i].style);
             manager->styles[i].style = labfont_style_clone(style);
             return true;
         }
     }
     
     // Grow array if needed
     if (manager->num_styles == manager->capacity) {
         size_t new_capacity = manager->capacity == 0 ? 8 : manager->capacity * 2;
         void* new_styles = realloc(manager->styles, new_capacity * sizeof(*manager->styles));
         
         if (!new_styles) {
             labfont_set_error("Failed to allocate memory for style manager");
             return false;
         }
         manager->styles = (labfont_style_entry*)new_styles;
         manager->capacity = new_capacity;
     }
     
     // Add new style
     manager->styles[manager->num_styles].name = labfont_strdup(name);
     manager->styles[manager->num_styles].style = labfont_style_clone(style);
     manager->num_styles++;
     
     return true;
 }
 
 labfont_style* labfont_style_manager_get(labfont_style_manager* manager, const char* name) {
     if (!manager || !name) return NULL;
     
     for (size_t i = 0; i < manager->num_styles; i++) {
         if (strcmp(manager->styles[i].name, name) == 0) {
             return manager->styles[i].style;
         }
     }
     
     return NULL;
 }
 
 bool labfont_style_manager_remove(labfont_style_manager* manager, const char* name) {
     if (!manager || !name) return false;
     
     for (size_t i = 0; i < manager->num_styles; i++) {
         if (strcmp(manager->styles[i].name, name) == 0) {
             free(manager->styles[i].name);
             labfont_style_destroy(manager->styles[i].style);
             
             // Move last item to this position (unless it's the last one)
             if (i < manager->num_styles - 1) {
                 manager->styles[i] = manager->styles[manager->num_styles - 1];
             }
             
             manager->num_styles--;
             return true;
         }
     }
     
     return false;
 }
 
 bool labfont_style_manager_has(labfont_style_manager* manager, const char* name) {
     if (!manager || !name) return false;
     
     for (size_t i = 0; i < manager->num_styles; i++) {
         if (strcmp(manager->styles[i].name, name) == 0) {
             return true;
         }
     }
     
     return false;
 }
 
 void labfont_style_manager_clear(labfont_style_manager* manager) {
     if (!manager) return;
     
     for (size_t i = 0; i < manager->num_styles; i++) {
         free(manager->styles[i].name);
         labfont_style_destroy(manager->styles[i].style);
     }
     
     manager->num_styles = 0;
 }
 
 /* Style manipulation implementation */
 void labfont_style_init(labfont_style* style) {
     if (!style) return;
     
     memset(style, 0, sizeof(labfont_style));
     
     // Set default values
     style->properties[LABFONT_PROP_SIZE].float_val = 12.0f;
     style->properties[LABFONT_PROP_COLOR].color_val = (labfont_color){255, 255, 255, 255};
     style->properties[LABFONT_PROP_BGCOLOR].color_val = (labfont_color){0, 0, 0, 0};
     style->properties[LABFONT_PROP_ALIGNMENT].int_val = LABFONT_ALIGN_LEFT | LABFONT_ALIGN_BASELINE;
     style->properties[LABFONT_PROP_SPACING].float_val = 0.0f;
     style->properties[LABFONT_PROP_BLUR].float_val = 0.0f;
     style->properties[LABFONT_PROP_WEIGHT].int_val = 400; // Normal weight
     style->properties[LABFONT_PROP_STYLE].int_val = LABFONT_STYLE_NORMAL;
 }
 
 labfont_style* labfont_style_create(void) {
     labfont_style* style = (labfont_style*)malloc(sizeof(labfont_style));
     if (!style) {
         labfont_set_error("Failed to allocate style");
         return NULL;
     }
     
     labfont_style_init(style);
     return style;
 }
 
 void labfont_style_destroy(labfont_style* style) {
     if (!style) return;
     
     // Free string properties
     if (style->has_property[LABFONT_PROP_FONT] && style->properties[LABFONT_PROP_FONT].string_val) {
         free(style->properties[LABFONT_PROP_FONT].string_val);
     }
     
     if (style->has_property[LABFONT_PROP_INHERIT] && style->properties[LABFONT_PROP_INHERIT].string_val) {
         free(style->properties[LABFONT_PROP_INHERIT].string_val);
     }
     
     free(style);
 }
 
 labfont_style* labfont_style_clone(const labfont_style* style) {
     if (!style) return NULL;
     
     labfont_style* clone = labfont_style_create();
     if (!clone) return NULL;
     
     // Copy all properties
     memcpy(clone->has_property, style->has_property, sizeof(style->has_property));
     
     for (int i = 0; i < LABFONT_PROP_COUNT; i++) {
         if (style->has_property[i]) {
             switch (i) {
                 case LABFONT_PROP_FONT:
                 case LABFONT_PROP_INHERIT:
                     // Deep copy strings
                     if (style->properties[i].string_val) {
                         clone->properties[i].string_val = labfont_strdup(style->properties[i].string_val);
                     }
                     break;
                 default:
                     // Simple value copy for non-string properties
                     clone->properties[i] = style->properties[i];
                     break;
             }
         }
     }
     
     return clone;
 }
 
 labfont_property_type labfont_parse_property_name(const char* name) {
     if (!name) return LABFONT_PROP_NONE;
     
     if (strcmp(name, "font") == 0) return LABFONT_PROP_FONT;
     if (strcmp(name, "size") == 0) return LABFONT_PROP_SIZE;
     if (strcmp(name, "color") == 0) return LABFONT_PROP_COLOR;
     if (strcmp(name, "bgcolor") == 0 || strcmp(name, "background") == 0) return LABFONT_PROP_BGCOLOR;
     if (strcmp(name, "align") == 0 || strcmp(name, "alignment") == 0) return LABFONT_PROP_ALIGNMENT;
     if (strcmp(name, "spacing") == 0) return LABFONT_PROP_SPACING;
     if (strcmp(name, "blur") == 0) return LABFONT_PROP_BLUR;
     if (strcmp(name, "weight") == 0) return LABFONT_PROP_WEIGHT;
     if (strcmp(name, "style") == 0) return LABFONT_PROP_STYLE;
     if (strcmp(name, "inherit") == 0) return LABFONT_PROP_INHERIT;
     
     return LABFONT_PROP_NONE;
 }
 
 bool labfont_parse_color_hex(const char* hex, labfont_color* color) {
     if (!hex || !color) return false;
     
     // Skip leading '#' if present
     if (hex[0] == '#') hex++;
     
     size_t len = strlen(hex);
     uint32_t value = 0;
     
     for (size_t i = 0; i < len; i++) {
         char c = tolower(hex[i]);
         uint8_t digit;
         
         if (c >= '0' && c <= '9') {
             digit = c - '0';
         } else if (c >= 'a' && c <= 'f') {
             digit = c - 'a' + 10;
         } else {
             labfont_set_error("Invalid hex character in color: %c", c);
             return false;
         }
         
         value = (value << 4) | digit;
     }
     
     switch (len) {
         case 3: // RGB
             color->r = ((value >> 8) & 0xF) * 17;
             color->g = ((value >> 4) & 0xF) * 17;
             color->b = ((value >> 0) & 0xF) * 17;
             color->a = 255;
             break;
         case 4: // RGBA
             color->r = ((value >> 12) & 0xF) * 17;
             color->g = ((value >> 8) & 0xF) * 17;
             color->b = ((value >> 4) & 0xF) * 17;
             color->a = ((value >> 0) & 0xF) * 17;
             break;
         case 6: // RRGGBB
             color->r = (value >> 16) & 0xFF;
             color->g = (value >> 8) & 0xFF;
             color->b = (value >> 0) & 0xFF;
             color->a = 255;
             break;
         case 8: // RRGGBBAA
             color->r = (value >> 24) & 0xFF;
             color->g = (value >> 16) & 0xFF;
             color->b = (value >> 8) & 0xFF;
             color->a = (value >> 0) & 0xFF;
             break;
         default:
             labfont_set_error("Invalid color format: %s", hex);
             return false;
     }
     
     return true;
 }
 
 bool labfont_parse_alignment(const char* align_str, int* alignment) {
     if (!align_str || !alignment) return false;
     
     *alignment = 0;
     
     // Split by '|' or ','
     char* str = labfont_strdup(align_str);
     if (!str) {
         labfont_set_error("Failed to allocate memory for alignment parsing");
         return false;
     }
     
     char* token = strtok(str, "|,");
     while (token) {
         // Trim whitespace
         labfont_str_trim(token);
         
         if (strcmp(token, "top") == 0) {
             *alignment |= LABFONT_ALIGN_TOP;
         } else if (strcmp(token, "middle") == 0) {
             *alignment |= LABFONT_ALIGN_MIDDLE;
         } else if (strcmp(token, "baseline") == 0) {
             *alignment |= LABFONT_ALIGN_BASELINE;
         } else if (strcmp(token, "bottom") == 0) {
             *alignment |= LABFONT_ALIGN_BOTTOM;
         } else if (strcmp(token, "left") == 0) {
             *alignment |= LABFONT_ALIGN_LEFT;
         } else if (strcmp(token, "center") == 0) {
             *alignment |= LABFONT_ALIGN_CENTER;
         } else if (strcmp(token, "right") == 0) {
             *alignment |= LABFONT_ALIGN_RIGHT;
         } else {
             free(str);
             labfont_set_error("Unknown alignment: %s", token);
             return false;
         }
         
         token = strtok(NULL, "|,");
     }
     
     free(str);
     return true;
 }
 
 bool labfont_style_parse(const char* style_def, labfont_style* style, 
                         labfont_style_manager* manager) {
     if (!style_def || !style) {
         labfont_set_error("Invalid parameters for style parsing");
         return false;
     }
     
     const char* p = style_def;
     
     // Skip leading whitespace
     while (isspace(*p)) p++;
     
     while (*p) {
         // Find property name
         const char* name_start = p;
         while (*p && *p != '=' && !isspace(*p)) p++;
         
         if (!*p || p == name_start) {
             labfont_set_error("Invalid property syntax in style definition");
             return false;
         }
         
         // Extract property name
         size_t name_len = p - name_start;
         char* prop_name = labfont_strndup(name_start, name_len);
         if (!prop_name) {
             labfont_set_error("Failed to allocate memory for property name");
             return false;
         }
         
         // Skip whitespace before '='
         while (isspace(*p)) p++;
         
         // Expect '='
         if (*p != '=') {
             free(prop_name);
             labfont_set_error("Expected '=' after property name");
             return false;
         }
         p++; // Skip '='
         
         // Skip whitespace after '='
         while (isspace(*p)) p++;
         
         // Find property value
         const char* value_start;
         const char* value_end;
         
         if (*p == '"' || *p == '\'') {
             // Quoted value
             char quote = *p;
             p++; // Skip opening quote
             value_start = p;
             
             // Find closing quote
             while (*p && *p != quote) p++;
             
             if (!*p) {
                 free(prop_name);
                 labfont_set_error("Unterminated quoted value");
                 return false;
             }
             
             value_end = p;
             p++; // Skip closing quote
         } else {
             // Unquoted value
             value_start = p;
             
             // Find end of value (whitespace or end of string)
             while (*p && !isspace(*p)) p++;
             value_end = p;
         }
         
         // Extract property value
         size_t value_len = value_end - value_start;
         char* prop_value = labfont_strndup(value_start, value_len);
         if (!prop_value) {
             free(prop_name);
             labfont_set_error("Failed to allocate memory for property value");
             return false;
         }
         
         // Parse property
         labfont_property_type prop_type = labfont_parse_property_name(prop_name);
         
         if (prop_type == LABFONT_PROP_NONE) {
             free(prop_name);
             free(prop_value);
             labfont_set_error("Unknown property: %s", prop_name);
             return false;
         }
         
         // Set property value based on type
         bool parse_success = true;
         
         switch (prop_type) {
             case LABFONT_PROP_FONT:
             case LABFONT_PROP_INHERIT:
                 if (style->has_property[prop_type] && style->properties[prop_type].string_val) {
                     free(style->properties[prop_type].string_val);
                 }
                 style->properties[prop_type].string_val = labfont_strdup(prop_value);
                 style->has_property[prop_type] = true;
                 break;
                 
             case LABFONT_PROP_SIZE:
             case LABFONT_PROP_SPACING:
             case LABFONT_PROP_BLUR:
                 style->properties[prop_type].float_val = (float)atof(prop_value);
                 style->has_property[prop_type] = true;
                 break;
                 
             case LABFONT_PROP_WEIGHT:
                 style->properties[prop_type].int_val = atoi(prop_value);
                 style->has_property[prop_type] = true;
                 break;
                 
             case LABFONT_PROP_COLOR:
             case LABFONT_PROP_BGCOLOR:
                 parse_success = labfont_parse_color_hex(prop_value, &style->properties[prop_type].color_val);
                 style->has_property[prop_type] = parse_success;
                 break;
                 
             case LABFONT_PROP_ALIGNMENT:
                 parse_success = labfont_parse_alignment(prop_value, &style->properties[prop_type].int_val);
                 style->has_property[prop_type] = parse_success;
                 break;
                 
             case LABFONT_PROP_STYLE: {
                 style->properties[prop_type].int_val = LABFONT_STYLE_NORMAL;
                 
                 // Parse comma-separated style flags
                 char* style_str = labfont_strdup(prop_value);
                 char* style_token = strtok(style_str, ",|");
                 
                 while (style_token) {
                     labfont_str_trim(style_token);
                     
                     if (strcmp(style_token, "bold") == 0) {
                         style->properties[prop_type].int_val |= LABFONT_STYLE_BOLD;
                     } else if (strcmp(style_token, "italic") == 0) {
                         style->properties[prop_type].int_val |= LABFONT_STYLE_ITALIC;
                     } else if (strcmp(style_token, "underline") == 0) {
                         style->properties[prop_type].int_val |= LABFONT_STYLE_UNDERLINE;
                     } else if (strcmp(style_token, "normal") != 0) {
                         free(style_str);
                         free(prop_name);
                         free(prop_value);
                         labfont_set_error("Unknown font style: %s", style_token);
                         return false;
                     }
                     
                     style_token = strtok(NULL, ",|");
                 }
                 
                 free(style_str);
                 style->has_property[prop_type] = true;
                 break;
             }
                 
             default:
                 parse_success = false;
                 labfont_set_error("Unhandled property type: %d", prop_type);
                 break;
         }
         
         free(prop_name);
         free(prop_value);
         
         if (!parse_success) {
             return false;
         }
         
         // Skip trailing whitespace
         while (isspace(*p)) p++;
     }
     
     // If inheritance is specified, resolve it immediately if manager is provided
     if (style->has_property[LABFONT_PROP_INHERIT] && manager) {
         return labfont_style_resolve_inheritance(style, manager, 10);
     }
     
     return true;
 }
 
 void labfont_style_apply(labfont_style* dest, const labfont_style* src) {
     if (!dest || !src) return;
     
     for (int i = 0; i < LABFONT_PROP_COUNT; i++) {
         if (src->has_property[i]) {
             // Handle string properties with deep copy
             if (i == LABFONT_PROP_FONT || i == LABFONT_PROP_INHERIT) {
                 if (dest->has_property[i] && dest->properties[i].string_val) {
                     free(dest->properties[i].string_val);
                 }
                 
                 if (src->properties[i].string_val) {
                     dest->properties[i].string_val = labfont_strdup(src->properties[i].string_val);
                 } else {
                     dest->properties[i].string_val = NULL;
                 }
             } else {
                 // Simple property copy
                 dest->properties[i] = src->properties[i];
             }
             
             dest->has_property[i] = true;
         }
     }
 }
 
 bool labfont_style_resolve_inheritance(labfont_style* style, 
                                      labfont_style_manager* manager,
                                      int max_depth) {
     if (!style || !manager || max_depth <= 0) {
         if (max_depth <= 0) {
             labfont_set_error("Maximum inheritance depth exceeded");
         } else {
             labfont_set_error("Invalid parameters for style inheritance resolution");
         }
         return false;
     }
     
     // If this style doesn't inherit, nothing to do
     if (!style->has_property[LABFONT_PROP_INHERIT] || 
         !style->properties[LABFONT_PROP_INHERIT].string_val) {
         return true;
     }
     
     // Get the parent style name
     const char* parent_name = style->properties[LABFONT_PROP_INHERIT].string_val;
     
     // Check if it's a global style reference (starts with @)
     bool is_global = parent_name[0] == '@';
     if (is_global) {
         parent_name++; // Skip the @ symbol
     }
     
     // Get the parent style
     labfont_style* parent = labfont_style_manager_get(manager, parent_name);
     if (!parent) {
         labfont_set_error("Inherited style not found: %s", 
                           style->properties[LABFONT_PROP_INHERIT].string_val);
         return false;
     }
     
     // Resolve parent's inheritance first
     if (parent->has_property[LABFONT_PROP_INHERIT]) {
         if (!labfont_style_resolve_inheritance(parent, manager, max_depth - 1)) {
             return false;
         }
     }
     
     // Create a temporary style for merging
     labfont_style* merged = labfont_style_clone(parent);
     if (!merged) {
         labfont_set_error("Failed to clone parent style for inheritance");
         return false;
     }
     
     // Now apply this style's properties onto the parent properties
     labfont_style_apply(merged, style);
     
     // Remove inherit property to avoid circular references
     if (merged->has_property[LABFONT_PROP_INHERIT]) {
         free(merged->properties[LABFONT_PROP_INHERIT].string_val);
         merged->has_property[LABFONT_PROP_INHERIT] = false;
     }
     
     // Copy merged properties back to this style
     for (int i = 0; i < LABFONT_PROP_COUNT; i++) {
         if (i == LABFONT_PROP_INHERIT) continue; // Skip inherit property
         
         if (style->has_property[i] && (i == LABFONT_PROP_FONT)) {
             free(style->properties[i].string_val);
         }
         
         style->has_property[i] = merged->has_property[i];
         if (merged->has_property[i]) {
             if (i == LABFONT_PROP_FONT) {
                 style->properties[i].string_val = 
                     merged->properties[i].string_val ? 
                     labfont_strdup(merged->properties[i].string_val) : NULL;
             } else {
                 style->properties[i] = merged->properties[i];
             }
         }
     }
     
     // Clean up
     labfont_style_destroy(merged);
     
     return true;
 }
 
 /* Helper functions for markup parsing */
 static void set_error(labfont_markup_result* result, const char* msg, const char* pos) {
     if (!result) return;
     
     result->has_error = true;
     strncpy(result->error_msg, msg, sizeof(result->error_msg) - 1);
     result->error_msg[sizeof(result->error_msg) - 1] = '\0';
     result->error_pos = pos;
     
     labfont_set_error("%s", msg);
 }
 
static bool add_token(labfont_markup_result* result, labfont_token token) {
    if (!result) return false;
     
    // Grow array if needed
    if (result->num_tokens == result->capacity) {
        size_t new_capacity = result->capacity == 0 ? 16 : result->capacity * 2;
         
        labfont_token* new_tokens = (labfont_token*)realloc(
            result->tokens, new_capacity * sizeof(labfont_token));
         
        if (!new_tokens) {
            set_error(result, "Failed to allocate memory for tokens", NULL);
            return false;
        }
         
        result->tokens = new_tokens;
        result->capacity = new_capacity;
    }
     
    // Add the token
    result->tokens[result->num_tokens++] = token;
    return true;
 }
 
static bool add_text_token(labfont_markup_result* result, 
                           const char* start, const char* end) {
    labfont_token token;
    memset(&token, 0, sizeof(token));
     
    token.type = LABFONT_TOKEN_TEXT;
    token.start = start;
    token.end = end;
    
    return add_token(result, token);
}


static bool parse_tag(labfont_markup_result* result, 
                      const char* start, const char* end) {
    // Skip leading whitespace
    while (start < end && isspace(*start)) start++;

    // Handle empty tag
    if (start == end) {
        set_error(result, "Empty tag", start - 1);
        return false;
    }

    // Handle style pop {/}
    if (*start == '/') {
        labfont_token token;
        memset(&token, 0, sizeof(token));

        token.type = LABFONT_TOKEN_STYLE_POP;
        token.start = start - 1; // Include opening brace
        token.end = end + 1;     // Include closing brace

        // Check if there's a name after the /
        start++;
        while (start < end && isspace(*start)) start++;

        if (start < end) {
            size_t name_len = end - start;
            token.name = (char*)malloc(name_len + 1);
            if (!token.name) {
                set_error(result, "Failed to allocate memory for style pop name", start);
                return false;
            }

            strncpy(token.name, start, name_len);
            token.name[name_len] = '\0';
            labfont_str_trim(token.name);
        }

        return add_token(result, token);
    }

    // Handle shorthand tags like {b}, {i}, {c=#rrggbb}
    if (end - start == 1 || 
        (start[0] == 'c' && start[1] == '=' && start + 2 < end)) {

    char shorthand = *start;

    if (shorthand == 'b' || shorthand == 'i' || shorthand == 'u' || shorthand == 'c') {
        labfont_token token;
        memset(&token, 0, sizeof(token));

        token.type = LABFONT_TOKEN_SHORTHAND;
        token.start = start - 1; // Include opening brace
        token.end = end + 1;     // Include closing brace
        token.shorthand = shorthand;

        // For color shorthand, extract the value
        if (shorthand == 'c' && start + 2 < end) {
            size_t value_len = end - (start + 2);
            token.value = (char*)malloc(value_len + 1);
            if (!token.value) {
                set_error(result, "Failed to allocate memory for color value", start + 2);
                return false;
            }

            strncpy(token.value, start + 2, value_len);
            token.value[value_len] = '\0';
            labfont_str_trim(token.value);
        }

        return add_token(result, token);
        }
    }

    // Check for name: prefix indicating a style definition
    const char* colon = NULL;
    for (const char* p = start; p < end; p++) {
        if (*p == ':') {
            colon = p;
            break;
        }
    }

    if (colon) {
        // This is a style definition {name: props}
        labfont_token token;
        memset(&token, 0, sizeof(token));

        token.type = LABFONT_TOKEN_STYLE_DEF;
        token.start = start - 1; // Include opening brace
        token.end = end + 1;     // Include closing brace

        // Extract name
        size_t name_len = colon - start;
        token.name = (char*)malloc(name_len + 1);
        if (!token.name) {
            set_error(result, "Failed to allocate memory for style name", start);
            return false;
        }

        strncpy(token.name, start, name_len);
        token.name[name_len] = '\0';
        labfont_str_trim(token.name);

        // Extract properties
        const char* props_start = colon + 1;
        while (props_start < end && isspace(*props_start)) props_start++;

        size_t props_len = end - props_start;
        token.props = (char*)malloc(props_len + 1);
        if (!token.props) {
            free(token.name);
            set_error(result, "Failed to allocate memory for style properties", props_start);
            return false;
        }

        strncpy(token.props, props_start, props_len);
        token.props[props_len] = '\0';

        return add_token(result, token);
    } else if (*start == '@') {
        // This is a global style reference {@name}
        labfont_token token;
        memset(&token, 0, sizeof(token));

        token.type = LABFONT_TOKEN_GLOBAL_REF;
        token.start = start - 1; // Include opening brace
        token.end = end + 1;     // Include closing brace

        // Extract name
        size_t name_len = end - (start + 1);
        token.name = (char*)malloc(name_len + 1);
        if (!token.name) {
            set_error(result, "Failed to allocate memory for global style name", start + 1);
            return false;
        }

        strncpy(token.name, start + 1, name_len);
        token.name[name_len] = '\0';
        labfont_str_trim(token.name);

        return add_token(result, token);
    } else if (strchr(start, '=')) {
        // This is an inline property set {prop=value prop=value}
        labfont_token token;
        memset(&token, 0, sizeof(token));

        token.type = LABFONT_TOKEN_STYLE_PROPS;
        token.start = start - 1; // Include opening brace
        token.end = end + 1;     // Include closing brace

        // Extract properties
        size_t props_len = end - start;
        token.props = (char*)malloc(props_len + 1);
        if (!token.props) {
            set_error(result, "Failed to allocate memory for properties", start);
            return false;
        }

        strncpy(token.props, start, props_len);
        token.props[props_len] = '\0';

        return add_token(result, token);
    } else {
        // This is a style reference {name}
        labfont_token token;
        memset(&token, 0, sizeof(token));

        token.type = LABFONT_TOKEN_STYLE_REF;
        token.start = start - 1; // Include opening brace
        token.end = end + 1;     // Include closing brace

        // Extract name
        size_t name_len = end - start;
        token.name = (char*)malloc(name_len + 1);
        if (!token.name) {
            set_error(result, "Failed to allocate memory for style name", start);
            return false;
        }

        strncpy(token.name, start, name_len);
        token.name[name_len] = '\0';
        labfont_str_trim(token.name);

        return add_token(result, token);
    }
}

 /* Parse markup text into tokens */
labfont_markup_result* labfont_parse_markup(const char* markup) {
    if (!markup) {
        labfont_set_error("NULL markup string");
        return NULL;
    }
    
    // Initialize result
    labfont_markup_result* result = (labfont_markup_result*)malloc(sizeof(labfont_markup_result));
    if (!result) {
        labfont_set_error("Failed to allocate markup result");
        return NULL;
    }
    
    result->tokens = NULL;
    result->num_tokens = 0;
    result->capacity = 0;
    result->has_error = false;
    result->error_msg[0] = '\0';
    result->error_pos = NULL;
    
    const char* p = markup;
    const char* text_start = p;
    
    while (*p) {
        // Look for opening brace
        if (*p == '{') {
            // If there was text before this, add it as a TEXT token
            if (p > text_start) {
                if (!add_text_token(result, text_start, p)) {
                    goto error;
                }
            }
            
            const char* tag_start = p;
            p++; // Skip the opening brace
            
            // Look for closing brace
            const char* tag_end = NULL;
            int brace_depth = 1;
            
            while (*p) {
                if (*p == '{') {
                    brace_depth++;
                } else if (*p == '}') {
                    brace_depth--;
                    if (brace_depth == 0) {
                        tag_end = p;
                        break;
                    }
                }
                p++;
            }
            
            if (!tag_end) {
                set_error(result, "Unterminated tag", tag_start);
                goto error;
            }
            
            // Parse the tag content
            if (!parse_tag(result, tag_start + 1, tag_end)) {
                goto error;
            }
            
            p = tag_end + 1; // Move past the closing brace
            text_start = p;
        } else {
            p++;
        }
    }
    
    // Add any remaining text
    if (*text_start) {
        if (!add_text_token(result, text_start, p)) {
            goto error;
        }
    }
    
    return result;
    
error:
    labfont_free_markup_result(result);
    return NULL;
}

/* Free a markup parsing result */
void labfont_free_markup_result(labfont_markup_result* result) {
    if (!result) return;
    
    // Free token data
    for (size_t i = 0; i < result->num_tokens; i++) {
        free(result->tokens[i].name);
        free(result->tokens[i].props);
        free(result->tokens[i].value);
    }
    
    free(result->tokens);
    free(result);
}

#endif /* LABFONT_STYLE_PARSER_IMPL_H */
#endif /* LABFONT_STYLE_PARSER_H */

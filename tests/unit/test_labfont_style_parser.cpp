
#define LABFONT_STYLE_PARSER_IMPLEMENTATION
#include <labfont/labfont_style_parser.h>

void example_style_parsing() {
    printf("Example 1: Basic style parsing\n");
    
    labfont_style* style = labfont_style_create();
    
    const char* style_def = "font=serif-normal size=24 color=#FF5500 align=center|baseline";
    printf("Parsing style: %s\n", style_def);
    
    if (labfont_style_parse(style_def, style, NULL)) {
        printf("Successfully parsed style!\n");
        printf("  Font: %s\n", style->has_property[LABFONT_PROP_FONT] ? 
              style->properties[LABFONT_PROP_FONT].string_val : "not set");
        printf("  Size: %.1f\n", style->has_property[LABFONT_PROP_SIZE] ? 
              style->properties[LABFONT_PROP_SIZE].float_val : 0.0f);
        
        if (style->has_property[LABFONT_PROP_COLOR]) {
            labfont_color c = style->properties[LABFONT_PROP_COLOR].color_val;
            printf("  Color: #%02X%02X%02X%02X\n", c.r, c.g, c.b, c.a);
        }
        
        if (style->has_property[LABFONT_PROP_ALIGNMENT]) {
            int align = style->properties[LABFONT_PROP_ALIGNMENT].int_val;
            printf("  Alignment: %d\n", align);
        }
    } else {
        printf("Failed to parse style: %s\n", labfont_parser_get_last_error());
    }
    
    labfont_style_destroy(style);
    printf("\n");
}

void example_style_inheritance() {
    printf("Example 2: Style inheritance\n");
    
    labfont_style_manager* manager = labfont_style_manager_create();
    
    // Define base style
    labfont_style* base = labfont_style_create();
    labfont_style_parse("font=sans-normal size=16 color=#333333", base, NULL);
    labfont_style_manager_define(manager, "base", base);
    
    // Define heading style that inherits from base
    labfont_style* heading = labfont_style_create();
    labfont_style_parse("inherit=base size=24 weight=700", heading, manager);
    labfont_style_manager_define(manager, "heading", heading);
    
    // Define emphasis style that inherits from base
    labfont_style* emphasis = labfont_style_create();
    labfont_style_parse("inherit=base style=italic color=#0066CC", emphasis, manager);
    labfont_style_manager_define(manager, "emphasis", emphasis);
    
    // Check the resolved styles
    printf("Base style:\n");
    printf("  Font: %s\n", base->has_property[LABFONT_PROP_FONT] ? 
          base->properties[LABFONT_PROP_FONT].string_val : "not set");
    printf("  Size: %.1f\n", base->has_property[LABFONT_PROP_SIZE] ? 
          base->properties[LABFONT_PROP_SIZE].float_val : 0.0f);
    
    printf("Heading style (inherits from base):\n");
    printf("  Font: %s\n", heading->has_property[LABFONT_PROP_FONT] ? 
          heading->properties[LABFONT_PROP_FONT].string_val : "not set");
    printf("  Size: %.1f\n", heading->has_property[LABFONT_PROP_SIZE] ? 
          heading->properties[LABFONT_PROP_SIZE].float_val : 0.0f);
    printf("  Weight: %d\n", heading->has_property[LABFONT_PROP_WEIGHT] ? 
          heading->properties[LABFONT_PROP_WEIGHT].int_val : 0);
    
    printf("Emphasis style (inherits from base):\n");
    printf("  Font: %s\n", emphasis->has_property[LABFONT_PROP_FONT] ? 
          emphasis->properties[LABFONT_PROP_FONT].string_val : "not set");
    printf("  Size: %.1f\n", emphasis->has_property[LABFONT_PROP_SIZE] ? 
          emphasis->properties[LABFONT_PROP_SIZE].float_val : 0.0f);
    printf("  Style: %d\n", emphasis->has_property[LABFONT_PROP_STYLE] ? 
          emphasis->properties[LABFONT_PROP_STYLE].int_val : 0);
    
    labfont_style_destroy(base);
    labfont_style_destroy(heading);
    labfont_style_destroy(emphasis);
    labfont_style_manager_destroy(manager);
    printf("\n");
}

void example_markup_parsing() {
    printf("Example 3: Markup parsing\n");
    
    const char* markup = "{normal: font=sans-normal size=16}Normal text "
                         "{b}bold text{/b} and {i}italic text{/i} "
                         "with {c=#FF0000}colored text{/c}.";
    
    printf("Parsing markup: %s\n", markup);
    
    labfont_markup_result* result = labfont_parse_markup(markup);
    
    if (result && !result->has_error) {
        printf("Successfully parsed markup!\n");
        printf("Number of tokens: %zu\n", result->num_tokens);
        
        for (size_t i = 0; i < result->num_tokens; i++) {
            labfont_token* token = &result->tokens[i];
            
            printf("Token %zu: ", i);
            
            switch (token->type) {
                case LABFONT_TOKEN_TEXT:
                    printf("TEXT \"%.*s\"\n", (int)(token->end - token->start), token->start);
                    break;
                    
                case LABFONT_TOKEN_STYLE_DEF:
                    printf("STYLE_DEF name=\"%s\" props=\"%s\"\n", 
                          token->name, token->props);
                    break;
                    
                case LABFONT_TOKEN_STYLE_REF:
                    printf("STYLE_REF name=\"%s\"\n", token->name);
                    break;
                    
                case LABFONT_TOKEN_GLOBAL_REF:
                    printf("GLOBAL_REF name=\"%s\"\n", token->name);
                    break;
                    
                case LABFONT_TOKEN_STYLE_POP:
                    if (token->name) {
                        printf("STYLE_POP name=\"%s\"\n", token->name);
                    } else {
                        printf("STYLE_POP\n");
                    }
                    break;
                    
                case LABFONT_TOKEN_STYLE_PROPS:
                    printf("STYLE_PROPS props=\"%s\"\n", token->props);
                    break;
                    
                case LABFONT_TOKEN_SHORTHAND:
                    printf("SHORTHAND '%c'%s%s\n", 
                          token->shorthand,
                          token->value ? " value=\"" : "",
                          token->value ? token->value : "");
                    break;
            }
        }
    } else {
        printf("Failed to parse markup: %s\n", 
              result ? result->error_msg : labfont_parser_get_last_error());
    }
    
    labfont_free_markup_result(result);
    printf("\n");
}

int main() {
    example_style_parsing();
    example_style_inheritance();
    example_markup_parsing();
    return 0;
}

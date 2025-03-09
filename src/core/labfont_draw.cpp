#include "labfont/labfont_draw.h"
#include "labfont/labfont.h"
#include "../third_party/stb/fontstash.h"
#include "../third_party/stb/stb_truetype.h"
#include "../third_party/stb/stb_image.h"
#include "../third_party/stb/stb_image_write.h"
#include "cJSON/cJSON.h"
#include <array>
#include <map>
#include <memory>
#include <string>
#include <vector>

typedef struct LabFont
{
    lab_texture texture_slot;
    
    int id;           // >= zero for a TTF

    int img_w, img_h; // non-zero for a QuadPlay texture
    int baseline;
    int charsz_x, charsz_y;
    int charspc_x, charspc_y;
    std::array<int8_t, 256> kern;

} LabFont;

static std::array<int, 256> qp_font_map;
std::array<int, 256> build_quadplay_font_map();

static std::map<std::string, std::unique_ptr<LabFont>> fonts;

namespace lf_internal {
    void sokol8x8_unpack_font(const uint8_t* in_font, 
            int first_char, int last_char, uint8_t* out_pixels); 

    extern const uint8_t sokol_font_kc853[2048];
    extern const uint8_t sokol_font_kc854[2048];
    extern const uint8_t sokol_font_z1013[2048];
    extern const uint8_t sokol_font_cpc[2048];
    extern const uint8_t sokol_font_c64[2048];
    extern const uint8_t sokol_font_oric[2048];
}


namespace LabFontInternal {
    FONScontext* _imm_ctx = nullptr;

    struct LabFontState
    {
        LabFont* font;
        float size;
        LabFontColor color;
        LabFontAlign alignment;
        float spacing;
        float blur;
    };


    struct LoadResult { uint8_t* buff; size_t len; };
    LoadResult load(const char* path) {
        FILE* f = fopen(path, "rb");
        if (!f)
            return { nullptr, 0 };

        fseek(f, 0, SEEK_END);
        size_t sz = ftell(f);

        if (sz == 0) {
            fclose(f);
            return { nullptr, 0 };
        }

        uint8_t* buff = (uint8_t*)malloc(sz);
        if (buff == nullptr) {
            fclose(f);
            return { nullptr, 0 };
        }

        fseek(f, 0, SEEK_SET);
        size_t len = fread(buff, 1, sz, f);

        fclose(f);
        return { buff, len };
    }

    FONScontext* fontStash()
    {
        return _imm_ctx;
    }

    constexpr uint32_t fons_rgba(const LabFontColor& c)
    {
        return ((uint32_t)c.rgba[0]) |
               ((uint32_t)c.rgba[1] << 8) |
               ((uint32_t)c.rgba[2] << 16) |
               ((uint32_t)c.rgba[3] << 24);
    }

    constexpr int fons_align(const LabFontAlign& a)
    {
        int r = 0;
        if (a.alignment & LabFontAlignTop)
            r |= FONS_ALIGN_TOP;
        else if (a.alignment & LabFontAlignMiddle)
            r |= FONS_ALIGN_MIDDLE;
        else if (a.alignment & LabFontAlignBaseline)
            r |= FONS_ALIGN_BASELINE;
        else if (a.alignment & LabFontAlignBottom)
            r |= FONS_ALIGN_BOTTOM;
        if (a.alignment & LabFontAlignLeft)
            r |= FONS_ALIGN_LEFT;
        else if (a.alignment & LabFontAlignCenter)
            r |= FONS_ALIGN_CENTER;
        else if (a.alignment & LabFontAlignRight)
            r |= FONS_ALIGN_RIGHT;
        return r;
    }

    void fontstash_bind(LabFontState* fs)
    {
        FONScontext* fc = fontStash();
        fonsSetFont(fc,    fs->font->id);
        fonsSetSize(fc,    fs->size);
        fonsSetColor(fc,   fons_rgba(fs->color));
        fonsSetAlign(fc,   fons_align(fs->alignment));
        fonsSetSpacing(fc, fs->spacing);
        fonsSetBlur(fc,    fs->blur);
    }
}

extern "C"
LabFont* LabFontLoad(lab_context ctx, const char* name, const char* path, LabFontType type)
{
    std::string key(name);
    if (type.type == LabFontTypeTTF)
    {
        LabFontInternal::LoadResult res = LabFontInternal::load(path);
        if (!res.len)
            return nullptr;

        LabFont* r = (LabFont*)calloc(1, sizeof(LabFont));
        if (!r)
            return nullptr;
        
        r->texture_slot = nullptr;

        // pass ownership of res.buff to fons
        r->id = fonsAddFontMem(LabFontInternal::fontStash(), name,
            (unsigned char*)res.buff, (int)res.len, false);

        fonts[key] = std::unique_ptr<LabFont>(r);
        return r;
    }
    else if (type.type == LabFontTypeQuadplay)
    {
        qp_font_map = build_quadplay_font_map();
        LabFont* r = (LabFont*)calloc(1, sizeof(LabFont));
        if (!r) {
            return nullptr;
        }
    
        r->id = -1;
    
        bool mono_numeric = true;
        bool monospaced = false;
        bool word_space_override = false;
        int word_space = 0;
        std::string path_str = path;
        size_t lastindex = path_str.find_last_of(".");
        if (lastindex != std::string::npos) {
            std::string jpath = path_str.substr(0, lastindex) + ".font.json";
            LabFontInternal::LoadResult res = LabFontInternal::load(jpath.c_str());
            if (res.len) {
                cJSON* json = cJSON_ParseWithLength((const char*)res.buff, res.len);
                if (json) {
                    cJSON* baseline = cJSON_GetObjectItem(json, "baseline");
                    if (cJSON_IsNumber(baseline)) {
                        r->baseline = baseline->valueint;
                    }
                    
                    cJSON* char_size = cJSON_GetObjectItem(json, "char_size");
                    if (cJSON_IsObject(char_size)) {
                        cJSON* x = cJSON_GetObjectItem(char_size, "x");
                        cJSON* y = cJSON_GetObjectItem(char_size, "y");
                        if (cJSON_IsNumber(x)) r->charsz_x = x->valueint;
                        if (cJSON_IsNumber(y)) r->charsz_y = y->valueint;
                    }
                    
                    cJSON* letter_spacing = cJSON_GetObjectItem(json, "letter_spacing");
                    if (cJSON_IsObject(letter_spacing)) {
                        cJSON* x = cJSON_GetObjectItem(letter_spacing, "x");
                        cJSON* y = cJSON_GetObjectItem(letter_spacing, "y");
                        if (cJSON_IsNumber(x)) r->charspc_x = x->valueint;
                        if (cJSON_IsNumber(y)) r->charspc_y = y->valueint;
                    }
                    
                    cJSON* atlas = cJSON_GetObjectItem(json, "atlas");
                    if (cJSON_IsString(atlas)) {
                        std::string atlas_str = atlas->valuestring;
                        if (atlas_str == "proportional") {
                            mono_numeric = true;
                        } else if (atlas_str == "monospaced") {
                            monospaced = true;
                        } else if (atlas_str == "proportional, mono-numeric") {
                            mono_numeric = true;
                        }
                    }
                    
                    cJSON* wordspace = cJSON_GetObjectItem(json, "word_spacing");
                    if (cJSON_IsNumber(wordspace)) {
                        word_space_override = true;
                        word_space = wordspace->valueint;
                    }
                    
                    cJSON_Delete(json);
                } else {
                    printf("JSON parse error: %s\n", cJSON_GetErrorPtr());
                }
                free(res.buff);
            }
        }

        // force the image to load as RGBA8
        int x, y, n;
        uint8_t* data = stbi_load(path, &x, &y, &n, STBI_rgb_alpha);
        if (data != nullptr && x > 0 && y > 0 && n > 0)
        {
            // stbi fills in alpha with 255, so zero out alpha for empty pixels
            for (int i = 0; i < x * y; ++i) {
                int addr = i * 4;
                data[addr + 3] = data[addr] == 0 ? 0 : 255;
            }
            
            lab_texture_desc desc = {
                .width = (unsigned int) x,
                .height = (unsigned int) y,
                .format = LAB_TEXTURE_FORMAT_RGBA8_UNORM,
                .initial_data = data
            };
            lab_result result = lab_create_texture(ctx, &desc, &r->texture_slot);

            if (result != LAB_RESULT_OK) {
                stbi_image_free(data);
                free(r);
                return nullptr;
            }
            
            r->img_w = x;
            r->img_h = y;
            r->charsz_x = x / 32;
            r->charsz_y = y / 14;

            if (monospaced) {
                for (int idx = 0; idx < 255; ++idx) {
                    r->kern[idx] = 0;
                }
            }
            else {
                bool printit = false;
                int char_w = x / 32;
                int char_h = y / 14;
                for (int idx = 0; idx < 255; ++idx) {
                    int i = qp_font_map[idx];
                    int px = (i & 0x1f) * char_w;
                    int py = (i / 32) * x * char_h;
                    printit = idx == '/';
                    int maxx = 0;
                    for (int cy = 0; cy < char_h; ++cy) {
                        for (int cx = 0; cx < char_w; ++cx) {
                            int addr = (py + (cy * x) + px + cx) * 4;
                            if (data[addr] != 0) {
                                if (printit)
                                    printf("A");
                                if (cx > maxx)
                                    maxx = cx;
                            }
                            else {
                                if (printit)
                                    printf(" ");
                            }
                        }
                        if (printit)
                            printf("\n");
                    }
                    r->kern[idx] = maxx - (x / 32);
                }
                if (word_space_override) {
                    r->kern[32] = word_space - (x / 32);
                }
                else {
                    r->kern[32] += char_w / 2;
                }
            }

            if (mono_numeric) {
                for (int i = '0'; i <= '9'; ++i)
                    r->kern[i] = -r->charspc_x;
            }

            stbi_image_free(data);
            fonts[key] = std::unique_ptr<LabFont>(r);
            return r;
        }
    }
    else if (type.type == LabFontTypeSokol8x8) {
        using namespace lf_internal;
        static bool unpack = true;
        static uint8_t* texture = nullptr;
        LabFont* r = (LabFont*)calloc(1, sizeof(LabFont));
        if (!r)
        {
            return nullptr;
        }
        if (unpack) {
            size_t sz = 2048 * 8 * 8;   // two extra slots. 6 * 8 is enough, but, power of 2.
            texture = (uint8_t*) malloc(sz);
            sokol8x8_unpack_font(sokol_font_kc853, 0, 0xff, texture + 2048 * 8 * 0);
            sokol8x8_unpack_font(sokol_font_kc854, 0, 0xff, texture + 2048 * 8 * 1);
            sokol8x8_unpack_font(sokol_font_z1013, 0, 0xff, texture + 2048 * 8 * 2);
            sokol8x8_unpack_font(sokol_font_cpc,   0, 0xff, texture + 2048 * 8 * 3);
            sokol8x8_unpack_font(sokol_font_c64,   0, 0xff, texture + 2048 * 8 * 4);
            sokol8x8_unpack_font(sokol_font_oric,  0, 0xff, texture + 2048 * 8 * 5);
            unpack = false;
        }

        lab_texture_desc desc = {
            .width = 256 * 8,
            .height = 8 * 8,
            .format = LAB_TEXTURE_FORMAT_R8_UNORM,
            .initial_data = texture
        };
        lab_result result = lab_create_texture(ctx, &desc, &r->texture_slot);

        if (result != LAB_RESULT_OK) {
            printf("Could not create a texture of size %d x %d\n", 256 * 8, 8 * 8);
            free(texture);
            free(r);
            return nullptr;
        }

        r->id = -2;
        r->img_w = 256 * 8;
        r->baseline = 7;
        r->charsz_x = 8;
        r->charsz_y = 8;
        r->charspc_x = 0;
        r->charspc_y = 0;

        if (!strcmp(name, "kc853")) {
            r->img_h = 8 * 0;
        }
        else if (!strcmp(name, "kc854")) {
            r->img_h = 8 * 1;
        }
        else if (!strcmp(name, "z1013")) {
            r->img_h = 8 * 2;
        }
        else if (!strcmp(name, "cpc")) {
            r->img_h = 8 * 3;
        }
        else if (!strcmp(name, "c64")) {
            r->img_h = 8 * 4;
        }
        else if (!strcmp(name, "oric")) {
            r->img_h = 8 * 5;
        }
        return r;
    }

    return nullptr;
}

LabFont* LabFontGet(const char* name)
{
    std::string key(name);
    auto it = fonts.find(key);
    if (it == fonts.end())
        return nullptr;

    return it->second.get();
}

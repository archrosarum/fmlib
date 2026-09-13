#include "internal.h"
#include <SDL3_ttf/SDL_ttf.h>
#include <string.h>

/* Bound retained memory even for constantly changing text and large images.
 * Texture bytes are estimated as RGBA; image snapshots and keys count too. */
#define CACHE_SLOTS 64
#define CACHE_BYTES ((size_t)32 * 1024 * 1024)

static const char* font_paths[] = {
    "/System/Library/Fonts/Supplemental/Times New Roman.ttf",
    "/Library/Fonts/Times New Roman.ttf",
    "C:/Windows/Fonts/times.ttf",
    "/usr/share/fonts/truetype/msttcorefonts/Times_New_Roman.ttf",
    "/usr/share/fonts/truetype/msttcorefonts/times.ttf",
    "/usr/share/fonts/TTF/times.ttf",
};

typedef struct {
    bool exists;
    SDL_PathInfo info;
} font_stamp;

typedef struct {
    fmgui_texture value;
    size_t bytes;
    Uint64 used;
    /* Image keys never dereference a previously supplied surface. */
    SDL_Surface* source;
    SDL_PixelFormat format;
    unsigned char* pixels;
    size_t row_bytes;
    char* content;
    char* font;
    font_stamp stamps[SDL_arraysize(font_paths)];
    float raster_size;
    int wrap_width;
    rgba color;
    text_align justify;
} cache_entry;

typedef struct render_cache {
    SDL_Renderer* renderer;
    cache_entry entries[CACHE_SLOTS];
    size_t bytes;
    Uint64 clock;
    struct render_cache* next;
} render_cache;

/* Like current_window, all access is on the main thread. */
static render_cache* caches;

static void remove_entry(render_cache* cache, cache_entry* entry)
{
    cache->bytes -= entry->bytes;
    if (entry->value.texture) SDL_DestroyTexture(entry->value.texture);
    SDL_free(entry->pixels);
    SDL_free(entry->content);
    SDL_free(entry->font);
    *entry = (cache_entry){0};
}

void fmgui_release_render_cache(SDL_Renderer* renderer)
{
    for (render_cache** link = &caches; *link; link = &(*link)->next) {
        render_cache* cache = *link;
        if (cache->renderer != renderer) continue;
        *link = cache->next;
        for (size_t i = 0; i < CACHE_SLOTS; ++i) remove_entry(cache, &cache->entries[i]);
        SDL_free(cache);
        return;
    }
}

void fmgui_release_image_cache(SDL_Surface* surface)
{
    if (!surface) return;
    for (render_cache* cache = caches; cache; cache = cache->next)
        for (size_t i = 0; i < CACHE_SLOTS; ++i)
            if (cache->entries[i].source == surface) remove_entry(cache, &cache->entries[i]);
}

static render_cache* get_cache(SDL_Renderer* renderer)
{
    for (render_cache* cache = caches; cache; cache = cache->next)
        if (cache->renderer == renderer) return cache;
    render_cache* cache = SDL_calloc(1, sizeof(*cache));
    if (!cache) return NULL;
    cache->renderer = renderer;
    cache->next = caches;
    caches = cache;
    return cache;
}

static void touch(render_cache* cache, cache_entry* entry)
{
    /* Overflow is fantastically unlikely, but must not break eviction. */
    if (cache->clock == SDL_MAX_UINT64) {
        for (size_t i = 0; i < CACHE_SLOTS; ++i) cache->entries[i].used = 0;
        cache->clock = 0;
    }
    entry->used = ++cache->clock;
}

static cache_entry* reserve_entry(render_cache* cache, size_t bytes)
{
    if (!cache || bytes > CACHE_BYTES) return NULL;
    for (;;) {
        cache_entry* empty = NULL;
        cache_entry* oldest = NULL;
        for (size_t i = 0; i < CACHE_SLOTS; ++i) {
            cache_entry* entry = &cache->entries[i];
            if (!entry->value.texture) empty = entry;
            else if (!oldest || entry->used < oldest->used) oldest = entry;
        }
        if (empty && bytes <= CACHE_BYTES - cache->bytes) return empty;
        if (!oldest) return NULL;
        remove_entry(cache, oldest);
    }
}

static bool create_texture(SDL_Renderer* renderer, SDL_Surface* surface, fmgui_texture* out)
{
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) return false;
    if (!SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_LINEAR) ||
        !SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND)) {
        SDL_DestroyTexture(texture);
        return false;
    }
    *out = (fmgui_texture){texture, surface->w, surface->h, true};
    return true;
}

static size_t texture_bytes(const fmgui_texture* value)
{
    if (value->width <= 0 || value->height <= 0 ||
        (size_t)value->width > CACHE_BYTES / 4 / (size_t)value->height)
        return CACHE_BYTES + 1;
    return (size_t)value->width * (size_t)value->height * 4;
}

static void font_stamps(const char* path, font_stamp* stamps)
{
    for (size_t i = 0; i < (path ? 1 : SDL_arraysize(font_paths)); ++i) {
        stamps[i].exists = SDL_GetPathInfo(path ? path : font_paths[i], &stamps[i].info);
    }
}

static bool same_stamps(const font_stamp* a, const font_stamp* b, bool explicit_font)
{
    for (size_t i = 0; i < (explicit_font ? 1 : SDL_arraysize(font_paths)); ++i) {
        if (a[i].exists != b[i].exists) return false;
        if (a[i].exists && (a[i].info.type != b[i].info.type ||
            a[i].info.size != b[i].info.size || a[i].info.create_time != b[i].info.create_time ||
            a[i].info.modify_time != b[i].info.modify_time)) return false;
    }
    return true;
}

static TTF_Font* open_font(const char* path, float size)
{
    if (path) return TTF_OpenFont(path, size);
    for (size_t i = 0; i < SDL_arraysize(font_paths); ++i) {
        TTF_Font* font = TTF_OpenFont(font_paths[i], size);
        if (font) return font;
    }
    SDL_SetError("Times New Roman not found; set text.font or FMGUI_DEFAULT_FONT to a font-file path");
    return NULL;
}

bool fmgui_text_texture(SDL_Renderer* renderer, const text* txt,
    float raster_size, int wrap_width, fmgui_texture* out)
{
    const char* path = txt->font;
    if (!path) {
        path = SDL_getenv("FMGUI_DEFAULT_FONT");
        if (path && !*path) path = NULL;
    }
    font_stamp stamps[SDL_arraysize(font_paths)] = {0};
    font_stamps(path, stamps);
    render_cache* cache = get_cache(renderer);
    if (cache) {
        for (size_t i = 0; i < CACHE_SLOTS; ++i) {
            cache_entry* entry = &cache->entries[i];
            if (!entry->content || entry->raster_size != raster_size ||
                entry->wrap_width != wrap_width || entry->justify != txt->justify ||
                entry->color.r != txt->color.r || entry->color.g != txt->color.g ||
                entry->color.b != txt->color.b || entry->color.a != txt->color.a ||
                (path == NULL) != (entry->font == NULL) ||
                (path && strcmp(path, entry->font)) || strcmp(txt->content, entry->content) ||
                !same_stamps(stamps, entry->stamps, path != NULL)) continue;
            touch(cache, entry);
            *out = entry->value;
            return true;
        }
    }
    /* Preserve SDL_ttf's externally observable initialization reference count.
     * Fonts only live on cache misses; warm draws never enter SDL_ttf. */
    if (!TTF_Init()) return false;
    TTF_Font* font = open_font(path, raster_size);
    SDL_Surface* surface = NULL;
    bool ok = false;
    if (font) {
        TTF_HorizontalAlignment align = TTF_HORIZONTAL_ALIGN_LEFT;
        if (txt->justify == TEXT_RIGHT) align = TTF_HORIZONTAL_ALIGN_RIGHT;
        if (txt->justify == TEXT_CENTER) align = TTF_HORIZONTAL_ALIGN_CENTER;
        TTF_SetFontWrapAlignment(font, align);
        surface = TTF_RenderText_Blended_Wrapped(font, txt->content, 0,
            (SDL_Color){txt->color.r, txt->color.g, txt->color.b, txt->color.a}, wrap_width);
        if (surface) ok = create_texture(renderer, surface, out);
    }
    if (surface) SDL_DestroySurface(surface);
    if (font) TTF_CloseFont(font);
    TTF_Quit();
    if (!ok) return false;

    size_t bytes = texture_bytes(out);
    size_t content_size = strlen(txt->content) + 1;
    size_t font_size = path ? strlen(path) + 1 : 0;
    if (!cache || bytes > CACHE_BYTES || content_size > CACHE_BYTES - bytes ||
        font_size > CACHE_BYTES - bytes - content_size) return true;
    char* content = SDL_strdup(txt->content);
    char* font_path = path ? SDL_strdup(path) : NULL;
    if (!content || (path && !font_path)) {
        SDL_free(content);
        SDL_free(font_path);
        return true;
    }
    bytes += content_size + font_size;
    cache_entry* entry = reserve_entry(cache, bytes);
    if (!entry) {
        SDL_free(content);
        SDL_free(font_path);
        return true;
    }
    out->temporary = false;
    *entry = (cache_entry){.value = *out, .bytes = bytes, .content = content,
        .font = font_path, .raster_size = raster_size, .wrap_width = wrap_width,
        .color = txt->color, .justify = txt->justify};
    SDL_memcpy(entry->stamps, stamps, sizeof(stamps));
    cache->bytes += bytes;
    touch(cache, entry);
    return true;
}

static bool image_cacheable(SDL_Surface* surface, size_t* row_bytes)
{
    /* Keep SDL's full conversion path for palettes, color keys, HDR, alternate
     * images and locked/RLE formats. No approximation of those semantics. */
    if (surface->w <= 0 || surface->h <= 0 || !surface->pixels ||
        (SDL_MUSTLOCK(surface) || (surface->flags & SDL_SURFACE_LOCKED)) ||
        SDL_ISPIXELFORMAT_INDEXED(surface->format) ||
        SDL_ISPIXELFORMAT_FOURCC(surface->format) || SDL_ISPIXELFORMAT_FLOAT(surface->format) ||
        SDL_BYTESPERPIXEL(surface->format) > 4 || SDL_BYTESPERPIXEL(surface->format) == 0 ||
        SDL_SurfaceHasColorKey(surface) || SDL_SurfaceHasAlternateImages(surface) ||
        SDL_GetSurfaceColorspace(surface) != SDL_COLORSPACE_SRGB) return false;
    SDL_PropertiesID props = SDL_GetSurfaceProperties(surface);
    if (SDL_HasProperty(props, SDL_PROP_SURFACE_SDR_WHITE_POINT_FLOAT) ||
        SDL_HasProperty(props, SDL_PROP_SURFACE_HDR_HEADROOM_FLOAT) ||
        SDL_HasProperty(props, SDL_PROP_SURFACE_TONEMAP_OPERATOR_STRING)) return false;
    if ((size_t)surface->w > CACHE_BYTES / SDL_BYTESPERPIXEL(surface->format)) return false;
    *row_bytes = (size_t)surface->w * SDL_BYTESPERPIXEL(surface->format);
    return surface->pitch > 0 && *row_bytes <= (size_t)surface->pitch &&
        *row_bytes <= CACHE_BYTES / (size_t)surface->h;
}

static bool same_pixels(const cache_entry* entry, SDL_Surface* surface, size_t row_bytes)
{
    if (entry->value.width != surface->w || entry->value.height != surface->h ||
        entry->format != surface->format || entry->row_bytes != row_bytes) return false;
    const unsigned char* pixels = surface->pixels;
    for (int y = 0; y < surface->h; ++y)
        if (memcmp(entry->pixels + (size_t)y * row_bytes,
            pixels + (size_t)y * surface->pitch, row_bytes)) return false;
    return true;
}

bool fmgui_image_texture(SDL_Renderer* renderer, SDL_Surface* surface, fmgui_texture* out)
{
    size_t row_bytes;
    if (!image_cacheable(surface, &row_bytes)) return create_texture(renderer, surface, out);
    render_cache* cache = get_cache(renderer);
    if (cache) {
        for (size_t i = 0; i < CACHE_SLOTS; ++i) {
            cache_entry* entry = &cache->entries[i];
            if (entry->source != surface) continue;
            if (same_pixels(entry, surface, row_bytes)) {
                Uint8 r, g, b, a;
                if (!SDL_GetSurfaceColorMod(surface, &r, &g, &b) ||
                    !SDL_GetSurfaceAlphaMod(surface, &a) ||
                    !SDL_SetTextureColorMod(entry->value.texture, r, g, b) ||
                    !SDL_SetTextureAlphaMod(entry->value.texture, a)) return false;
                touch(cache, entry);
                *out = entry->value;
                return true;
            }
            remove_entry(cache, entry);
            break;
        }
    }
    if (!create_texture(renderer, surface, out)) return false;
    size_t bytes = texture_bytes(out);
    size_t pixel_bytes = row_bytes * (size_t)surface->h;
    if (!cache || bytes > CACHE_BYTES || pixel_bytes > CACHE_BYTES - bytes) return true;
    unsigned char* copy = SDL_malloc(pixel_bytes);
    if (!copy) return true;
    for (int y = 0; y < surface->h; ++y)
        SDL_memcpy(copy + (size_t)y * row_bytes,
            (unsigned char*)surface->pixels + (size_t)y * surface->pitch, row_bytes);
    bytes += pixel_bytes;
    cache_entry* entry = reserve_entry(cache, bytes);
    if (!entry) { SDL_free(copy); return true; }
    out->temporary = false;
    *entry = (cache_entry){.value = *out, .bytes = bytes, .source = surface,
        .format = surface->format, .pixels = copy, .row_bytes = row_bytes};
    cache->bytes += bytes;
    touch(cache, entry);
    return true;
}

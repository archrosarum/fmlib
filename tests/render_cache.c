#include "internal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Compare a potentially cached draw to a fresh SDL conversion/rasterization.
 * This catches stale keys without depending on cache implementation details. */
static bool same_draw(window* win, rect* area, text* label, image* picture)
{
    SDL_Surface* warm = NULL;
    SDL_Surface* cold = NULL;
    bool ok = false;
    current_window = win;
    if (!clrwin() || !(label ? rendtxt(area, label) : rendimg(area, picture))) goto cleanup;
    warm = SDL_RenderReadPixels(win->renptr, NULL);
    fmgui_release_render_cache(win->renptr);
    if (!clrwin() || !(label ? rendtxt(area, label) : rendimg(area, picture))) goto cleanup;
    cold = SDL_RenderReadPixels(win->renptr, NULL);
    if (!warm || !cold || warm->w != cold->w || warm->h != cold->h ||
        warm->format != cold->format) goto cleanup;
    size_t row_bytes = (size_t)warm->w * SDL_BYTESPERPIXEL(warm->format);
    for (int y = 0; y < warm->h; ++y)
        if (memcmp((Uint8*)warm->pixels + (size_t)y * warm->pitch,
            (Uint8*)cold->pixels + (size_t)y * cold->pitch, row_bytes)) goto cleanup;
    ok = true;
cleanup:
    if (warm) SDL_DestroySurface(warm);
    if (cold) SDL_DestroySurface(cold);
    return ok;
}

int main(void)
{
    int status = EXIT_FAILURE;
    const char* stage = "setup";
    window win = mkwin(), other = {0};
    image picture = {0};
    if (!win.is_open) goto cleanup;
    current_window = &win;
    rect area = mkrect(&win.root, newvecmes(newmes(0, 150), newmes(0, 80)),
        newvecmes(newmes(0, 10), newmes(0, 10)), (vec_mes){0});
    char content[64] = "First label";
    text label = mktext(content);
    label.color = (rgba){255, 80, 30, 200};
    stage = "mutable text and geometry";
    if (!same_draw(&win, &area, &label, NULL) || !same_draw(&win, &area, &label, NULL)) goto cleanup;
    strcpy(content, "Second label");
    if (!same_draw(&win, &area, &label, NULL)) goto cleanup;
    label.color = (rgba){20, 240, 90, 120};
    if (!same_draw(&win, &area, &label, NULL)) goto cleanup;
    label.font_size = 26;
    if (!same_draw(&win, &area, &label, NULL)) goto cleanup;
    label.justify = TEXT_RIGHT;
    if (!same_draw(&win, &area, &label, NULL)) goto cleanup;
    area.size.x.px = 80;
    if (!same_draw(&win, &area, &label, NULL)) goto cleanup;
    area.size.y.px = 15;
    area.pos.x.px = 30;
    if (!same_draw(&win, &area, &label, NULL)) goto cleanup;

    stage = "mutable font path and environment";
    char path[256];
#ifdef SDL_PLATFORM_MACOS
    const char* font = "/System/Library/Fonts/Supplemental/Arial.ttf";
#elif defined(SDL_PLATFORM_WINDOWS)
    const char* font = "C:/Windows/Fonts/arial.ttf";
#else
    const char* font = "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf";
#endif
    SDL_strlcpy(path, font, sizeof(path));
    label.font = path;
    if (!same_draw(&win, &area, &label, NULL)) goto cleanup;
    SDL_strlcpy(path, "/nonexistent/font.ttf", sizeof(path));
    if (rendtxt(&area, &label)) goto cleanup;
    label.font = NULL;
    /* This test process owns its environment; no user environment is changed. */
    if (!SDL_SetEnvironmentVariable(SDL_GetEnvironment(), "FMGUI_DEFAULT_FONT", font, true) ||
        !same_draw(&win, &area, &label, NULL) ||
        !SDL_SetEnvironmentVariable(SDL_GetEnvironment(), "FMGUI_DEFAULT_FONT", path, true) ||
        rendtxt(&area, &label) ||
        !SDL_UnsetEnvironmentVariable(SDL_GetEnvironment(), "FMGUI_DEFAULT_FONT")) goto cleanup;

    stage = "entry eviction";
    area.size.y.px = 80;
    for (int i = 0; i < 90; ++i) {
        SDL_snprintf(content, sizeof(content), "Label %d", i);
        if (!rendtxt(&area, &label)) goto cleanup;
    }
    strcpy(content, "First label");
    if (!same_draw(&win, &area, &label, NULL)) goto cleanup;

    stage = "image pixel edits and modulation";
    picture.surface = SDL_CreateSurface(8, 8, SDL_PIXELFORMAT_RGBA32);
    if (!picture.surface || !SDL_FillSurfaceRect(picture.surface, NULL,
        SDL_MapSurfaceRGBA(picture.surface, 255, 30, 90, 180)) ||
        !same_draw(&win, &area, NULL, &picture) || !same_draw(&win, &area, NULL, &picture)) goto cleanup;
    if (!SDL_WriteSurfacePixel(picture.surface, 3, 4, 20, 255, 30, 255) ||
        !same_draw(&win, &area, NULL, &picture) ||
        !SDL_SetSurfaceColorMod(picture.surface, 50, 180, 230) ||
        !SDL_SetSurfaceAlphaMod(picture.surface, 70) ||
        !same_draw(&win, &area, NULL, &picture)) goto cleanup;
    stage = "color key fallback and return to cache";
    if (!SDL_SetSurfaceColorKey(picture.surface, true,
        SDL_MapSurfaceRGBA(picture.surface, 255, 30, 90, 180)) ||
        !same_draw(&win, &area, NULL, &picture) ||
        !SDL_SetSurfaceColorKey(picture.surface, false, 0) ||
        !same_draw(&win, &area, NULL, &picture)) goto cleanup;

    stage = "multi-window lifetime and device reset";
    other = mkwin();
    if (!other.is_open) goto cleanup;
    rect second = area;
    second.parent = &other.root;
    if (!same_draw(&other, &second, NULL, &picture) ||
        !same_draw(&other, &second, &label, NULL)) goto cleanup;
    current_window = &win;
    destroywin();
    if (!same_draw(&other, &second, NULL, &picture)) goto cleanup;
    SDL_Event reset = {0};
    reset.type = SDL_EVENT_RENDER_DEVICE_RESET;
    reset.render.windowID = SDL_GetWindowID(other.winptr);
    if (!SDL_PushEvent(&reset) || !updwin() ||
        !same_draw(&other, &second, &label, NULL)) goto cleanup;
    destroyimg(&picture);
    picture.surface = SDL_CreateSurface(8, 8, SDL_PIXELFORMAT_RGB24);
    if (!picture.surface || !SDL_FillSurfaceRect(picture.surface, NULL,
        SDL_MapSurfaceRGB(picture.surface, 30, 50, 255)) ||
        !same_draw(&other, &second, NULL, &picture)) goto cleanup;

    stage = "oversized image fallback";
    destroyimg(&picture);
    picture.surface = SDL_CreateSurface(2049, 2049, SDL_PIXELFORMAT_RGBA32);
    if (!picture.surface || !SDL_FillSurfaceRect(picture.surface, NULL,
        SDL_MapSurfaceRGBA(picture.surface, 100, 200, 20, 255)) ||
        !same_draw(&other, &second, NULL, &picture)) goto cleanup;

    stage = "deep layout fallback and cycles";
    rect chain[80];
    rect* parent = &other.root;
    for (size_t i = 0; i < SDL_arraysize(chain); ++i) {
        chain[i] = mkrect(parent, newvecmes(newmes(1, 0), newmes(1, 0)),
            newvecmes(newmes(0, 1), newmes(0, 1)), (vec_mes){0});
        parent = &chain[i];
    }
    SDL_FRect bounds;
    if (!resolve_rect(parent, &bounds) || bounds.x != 80 || bounds.y != 80) goto cleanup;
    chain[0].parent = parent;
    if (resolve_rect(parent, &bounds)) goto cleanup;
    status = EXIT_SUCCESS;
cleanup:
    if (status != EXIT_SUCCESS) fprintf(stderr, "Cache regression failed (%s): %s\n", stage, SDL_GetError());
    destroyimg(&picture);
    current_window = &other;
    destroywin();
    current_window = &win;
    destroywin();
    return status;
}

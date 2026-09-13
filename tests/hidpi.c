#include "fmgui.h"
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

static bool pixels_in_bounds(SDL_Surface* pixels, Uint8 channel, SDL_Rect* bounds)
{
    int left = pixels->w, top = pixels->h, right = -1, bottom = -1;
    for (int y = 0; y < pixels->h; ++y) {
        for (int x = 0; x < pixels->w; ++x) {
            Uint8 r, g, b, a;
            if (!SDL_ReadSurfacePixel(pixels, x, y, &r, &g, &b, &a)) return false;
            if ((channel == 0 ? r : g) > 0) {
                if (x < left) left = x;
                if (x > right) right = x;
                if (y < top) top = y;
                if (y > bottom) bottom = y;
            }
        }
    }
    if (right < left) return false;
    *bounds = (SDL_Rect){left, top, right - left + 1, bottom - top + 1};
    return true;
}

int main(void)
{
    int status = EXIT_FAILURE;
    const char* stage = "setup";
    window win = mkwin();
    current_window = &win;
    SDL_Texture* target = NULL;
    SDL_Surface* pixels = NULL;
    image img = {0};
    if (!win.is_open || !(SDL_GetWindowFlags(win.winptr) & SDL_WINDOW_HIGH_PIXEL_DENSITY)) goto cleanup;
    if (!SDL_SetWindowSize(win.winptr, 320, 240)) goto cleanup;
    img.surface = SDL_CreateSurface(3, 3, SDL_PIXELFORMAT_RGBA32);
    if (!img.surface || !SDL_FillSurfaceRect(img.surface, NULL,
        SDL_MapSurfaceRGBA(img.surface, 255, 0, 0, 255))) goto cleanup;
    rect box = mkrect(&win.root, (vec_mes){.x = {0, 64}, .y = {0, 32}},
        (vec_mes){.x = {0, 16}, .y = {0, 16}}, (vec_mes){0});
    rect area = mkrect(&win.root, (vec_mes){.x = {0, 200}, .y = {0, 100}},
        (vec_mes){.x = {0, 16}, .y = {0, 100}}, (vec_mes){0});
    text label = mktext("Hello.");
    label.color = (rgba){0, 255, 0, 255};
    label.font_size = 24;
    label.justify = TEXT_CENTER;
    SDL_Rect baseline = {0};
    const vec2f scales[] = {{1, 1}, {1.25f, 1.25f}, {1.5f, 1.5f}, {2, 2}, {2, 1.5f}, {.75f, .75f}, {1, 1}};
    for (size_t i = 0; i < SDL_arraysize(scales); ++i) {
        float sx = scales[i].x, sy = scales[i].y;
        stage = "target density";
        target = SDL_CreateTexture(win.renptr, SDL_PIXELFORMAT_RGBA8888,
            SDL_TEXTUREACCESS_TARGET, (int)(320 * sx), (int)(240 * sy));
        if (!target || !SDL_SetRenderTarget(win.renptr, target)) goto cleanup;
        /* Deliberately stale renderer settings must not introduce double scaling. */
        if (!SDL_SetRenderScale(win.renptr, 3, 3) || !clrwin()) goto cleanup;
        float actual_x, actual_y;
        if (!SDL_GetRenderScale(win.renptr, &actual_x, &actual_y) ||
            fabsf(actual_x - sx) > .001f || fabsf(actual_y - sy) > .001f) goto cleanup;
        SDL_FRect logical;
        if (!resolve_rect(&box, &logical) || logical.x != 16 || logical.y != 16 ||
            logical.w != 64 || logical.h != 32 || win.size.x != 320) goto cleanup;
        win.mouse_valid = true;
        win.mouse_x = 16;
        win.mouse_y = 16;
        if (!chkrect(&box, MOUSE_IN)) goto cleanup;
        win.mouse_x = 80;
        if (chkrect(&box, MOUSE_IN)) goto cleanup;
        stage = "image and text pixels";
        if (!rendimg(&box, &img) || !rendtxt(&area, &label)) goto cleanup;
        pixels = SDL_RenderReadPixels(win.renptr, NULL);
        SDL_Rect red, green;
        if (!pixels || !pixels_in_bounds(pixels, 0, &red) || !pixels_in_bounds(pixels, 1, &green)) goto cleanup;
        if (red.x != (int)(16 * sx) || red.y != (int)(16 * sy) ||
            red.w != (int)(64 * sx) || red.h != (int)(32 * sy)) goto cleanup;
        if (i == 0) baseline = green;
        if (fabsf(green.w / sx - baseline.w) > 3 || fabsf(green.h / sy - baseline.h) > 3 ||
            fabsf((green.x + green.w / 2.0f) / sx - (baseline.x + baseline.w / 2.0f)) > 2 ||
            fabsf(green.y / sy - baseline.y) > 3) goto cleanup;
        SDL_DestroySurface(pixels);
        pixels = NULL;
        stage = "rectangle fill and clipping";
        if (!clrwin() || !rendrect(&box, (rgba){255, 0, 0, 255})) goto cleanup;
        SDL_Rect clip = {16, 100, 100, 20}, after;
        if (!SDL_SetRenderClipRect(win.renptr, &clip) || !rendtxt(&area, &label) ||
            !SDL_GetRenderClipRect(win.renptr, &after) ||
            after.x != clip.x || after.y != clip.y || after.w != clip.w || after.h != clip.h) goto cleanup;
        pixels = SDL_RenderReadPixels(win.renptr, NULL);
        if (!pixels || !pixels_in_bounds(pixels, 0, &red) || !pixels_in_bounds(pixels, 1, &green) ||
            green.x + green.w > (int)(116 * sx) || green.y + green.h > (int)(120 * sy)) goto cleanup;
        SDL_DestroySurface(pixels);
        pixels = NULL;
        if (!SDL_SetRenderClipRect(win.renptr, NULL) || !SDL_SetRenderTarget(win.renptr, NULL)) goto cleanup;
        SDL_DestroyTexture(target);
        target = NULL;
    }
    stage = "resize and oversized text";
    if (!SDL_SetWindowSize(win.winptr, 400, 300) || !clrwin() || win.size.x != 400) goto cleanup;
    int pw, ph;
    float sx, sy;
    if (!SDL_GetRenderOutputSize(win.renptr, &pw, &ph) || !SDL_GetRenderScale(win.renptr, &sx, &sy) ||
        fabsf(sx - pw / 400.0f) > .001f || fabsf(sy - ph / 300.0f) > .001f) goto cleanup;
    label.font_size = FLT_MAX;
    if (rendtxt(&area, &label)) goto cleanup;
    status = EXIT_SUCCESS;
cleanup:
    if (status != EXIT_SUCCESS) fprintf(stderr, "HiDPI regression failed (%s): %s\n", stage, SDL_GetError());
    if (pixels) SDL_DestroySurface(pixels);
    if (win.renptr) SDL_SetRenderTarget(win.renptr, NULL);
    if (target) SDL_DestroyTexture(target);
    destroyimg(&img);
    current_window = &win;    destroywin();
    return status;
}

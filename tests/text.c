#include "fmgui.h"
#include <SDL3_ttf/SDL_ttf.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

/* Read actual output, including its color, instead of only testing API returns. */
static bool ink_bounds(window* win, const rect* rec, const text* label, SDL_Rect* bounds)
{
    current_window = win;
    if (!clrwin() || !rendtxt(rec, label)) return false;
    SDL_Surface* pixels = SDL_RenderReadPixels(win->renptr, NULL);
    if (!pixels) return false;
    int left = pixels->w, top = pixels->h, right = -1, bottom = -1;
    bool ok = true;
    for (int y = 0; y < pixels->h && ok; ++y) {
        for (int x = 0; x < pixels->w; ++x) {
            Uint8 r, g, b, a;
            if (!SDL_ReadSurfacePixel(pixels, x, y, &r, &g, &b, &a) || g || b) {
                ok = false;
                break;
            }
            if (r) {
                if (x < left) left = x;
                if (x > right) right = x;
                if (y < top) top = y;
                if (y > bottom) bottom = y;
            }
        }
    }
    SDL_DestroySurface(pixels);
    if (!ok || right < left) return false;
    *bounds = (SDL_Rect){left, top, right - left + 1, bottom - top + 1};
    return true;
}

int main(void)
{
    int status = EXIT_FAILURE;
    window win = mkwin();
    if (!win.is_open) goto cleanup;
    rect panel = mkrect(&win.root, (vec_mes){.x = {0, 400}, .y = {0, 200}},
        (vec_mes){.x = {0, 20}, .y = {0, 30}}, (vec_mes){0});
    text label = mktext("Hello");
    if (label.font || label.font_size != 16 || label.justify != TEXT_LEFT ||
        label.color.r != 0 || label.color.g != 0 || label.color.b != 0 || label.color.a != 255) goto cleanup;
    label.color = (rgba){255, 0, 0, 255};
    SDL_Rect left, right, center, moved, larger;
    if (!ink_bounds(&win, &panel, &label, &left)) goto cleanup;
    label.justify = TEXT_RIGHT;
    if (!ink_bounds(&win, &panel, &label, &right)) goto cleanup;
    label.justify = TEXT_CENTER;
    if (!ink_bounds(&win, &panel, &label, &center)) goto cleanup;
    if (left.x < 20 || right.x + right.w > 420 || right.x <= left.x ||
        left.w != right.w || right.w != center.w ||
        abs(2 * center.x - left.x - right.x) > 1 ||
        left.y != right.y || left.y != center.y) goto cleanup;
    panel.pos.x.px += 30;
    panel.pos.y.px += 20;
    if (!ink_bounds(&win, &panel, &label, &moved) || moved.x != center.x + 30 ||
        moved.y != center.y + 20) goto cleanup;
    label.font_size = 32;
    if (!ink_bounds(&win, &panel, &label, &larger) || larger.w <= moved.w ||
        larger.h <= moved.h) goto cleanup;
    label.content = "Hello\nWorld";
    if (!ink_bounds(&win, &panel, &label, &moved) || moved.h <= larger.h) goto cleanup;
    /* Reuse one style on another rectangle, and wrap/clip within its bounds. */
    rect narrow = mkrect(&panel, (vec_mes){.x = {0, 80}, .y = {0, 100}},
        (vec_mes){.x = {0, 10}, .y = {0, 10}}, (vec_mes){0});
    label.content = "Hello Hello Hello";
    if (!ink_bounds(&win, &narrow, &label, &moved) || moved.h <= larger.h ||
        moved.x < 60 || moved.x + moved.w > 140 || moved.y + moved.h > 160)
        goto cleanup;
    narrow.size.y.px = 20;
    if (!ink_bounds(&win, &narrow, &label, &moved) || moved.y + moved.h > 80)
        goto cleanup;
    narrow.size.x.px = 0;
    if (!rendtxt(&narrow, &label)) goto cleanup;
    label.content = "Caf\xc3\xa9";
    if (!ink_bounds(&win, &panel, &label, &moved)) goto cleanup;
    label.content = "";
    if (!rendtxt(&panel, &label)) goto cleanup;
    label.content = NULL;
    if (rendtxt(&panel, &label) || rendtxt(&panel, NULL)) goto cleanup;
    label.content = "Hello";
    label.font_size = NAN;
    if (rendtxt(&panel, &label)) goto cleanup;
    label.font_size = 0;
    if (rendtxt(&panel, &label)) goto cleanup;
    label.font_size = 16;
    label.justify = (text_align)99;
    if (rendtxt(&panel, &label)) goto cleanup;
    label.justify = TEXT_LEFT;
    panel.pos.x.pct = INFINITY;
    if (rendtxt(&panel, &label)) goto cleanup;
    panel.pos.x.pct = 0;
    if (rendtxt(NULL, &label)) goto cleanup;
    panel.parent = &panel;
    if (rendtxt(&panel, &label)) goto cleanup;
    panel.parent = &win.root;
    label.font = "/nonexistent/fmgui-font.ttf";
    if (rendtxt(&panel, &label)) goto cleanup;
    /* Explicitly choose another installed font to exercise customization. */
#ifdef SDL_PLATFORM_MACOS
    label.font = "/System/Library/Fonts/Supplemental/Arial.ttf";
#elif defined(SDL_PLATFORM_WINDOWS)
    label.font = "C:/Windows/Fonts/arial.ttf";
#else
    label.font = "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf";
#endif
    if (!ink_bounds(&win, &panel, &label, &moved)) goto cleanup;
    if (TTF_WasInit() != 0 || !TTF_Init()) goto cleanup;
    bool rendered = rendtxt(&panel, &label);
    int refs = TTF_WasInit();
    TTF_Quit();
    if (!rendered || refs != 1) goto cleanup;
    status = EXIT_SUCCESS;

cleanup:
    if (status != EXIT_SUCCESS) fprintf(stderr, "Text regression failed: %s\n", SDL_GetError());
    current_window = &win;    destroywin();
    return status;
}

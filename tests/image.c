#include "fmgui.h"
#include <stdio.h>
#include <stdlib.h>

/* A uniform translucent fixture makes scaling, bounds, and blending observable. */
static bool check_draw(window* win, rect* rec, image* img)
{
    current_window = win;
    SDL_FRect bounds;
    if (!resolve_rect(rec, &bounds) || !clrwin() || !rendimg(rec, img)) return false;
    SDL_Surface* pixels = SDL_RenderReadPixels(win->renptr, NULL);
    if (!pixels) return false;
    bool ok = true;
    for (int y = 0; y < pixels->h && ok; ++y) {
        for (int x = 0; x < pixels->w; ++x) {
            Uint8 r, g, b, a;
            if (!SDL_ReadSurfacePixel(pixels, x, y, &r, &g, &b, &a)) { ok = false; break; }
            bool inside = x >= bounds.x && x < bounds.x + bounds.w &&
                y >= bounds.y && y < bounds.y + bounds.h;
            if (g || b || (inside ? (r < 126 || r > 129) : r != 0)) {
                ok = false;
                break;
            }
        }
    }
    SDL_DestroySurface(pixels);
    return ok;
}

int main(void)
{
    int status = EXIT_FAILURE;
    window win = mkwin(), other = {0};
    image img = {0}, invalid = {0};
    SDL_Surface* fixture = NULL;
    const char* path = "bin/image-test.bmp";
    if (!win.is_open) goto cleanup;
    fixture = SDL_CreateSurface(2, 3, SDL_PIXELFORMAT_RGBA32);
    if (!fixture || !SDL_FillSurfaceRect(fixture, NULL,
        SDL_MapSurfaceRGBA(fixture, 255, 0, 0, 128)) || !SDL_SaveBMP(fixture, path)) goto cleanup;
    img = mkimg(path);
    if (!img.surface) goto cleanup;
    rect panel = mkrect(&win.root, (vec_mes){.x = {0, 100}, .y = {0, 80}},
        (vec_mes){.x = {0, 20}, .y = {0, 30}}, (vec_mes){0});
    rect box = mkrect(&panel, (vec_mes){.x = {.5f, 0}, .y = {.5f, 0}},
        (vec_mes){.x = {.5f, 0}, .y = {.5f, 0}},
        (vec_mes){.x = {.5f, 0}, .y = {.5f, 0}});
    if (!check_draw(&win, &box, &img)) goto cleanup;
    panel.pos.x.px += 40;
    panel.size.x.px = 200;
    if (!check_draw(&win, &box, &img)) goto cleanup;
    other = mkwin();
    if (!other.is_open) goto cleanup;
    box.parent = &other.root;
    if (!check_draw(&other, &box, &img)) goto cleanup;
    if (rendimg(NULL, &img) || rendimg(&box, NULL) || rendimg(&box, &invalid)) goto cleanup;
    box.size.x = (mes){0};
    if (!rendimg(&box, &img)) goto cleanup;
    box.parent = &box;
    if (rendimg(&box, &img)) goto cleanup;
    invalid = mkimg(NULL);
    if (invalid.surface) goto cleanup;
    invalid = mkimg("/nonexistent/fmgui-image.png");
    if (invalid.surface) goto cleanup;
    destroyimg(&img);
    destroyimg(&img);
    destroyimg(NULL);
    status = EXIT_SUCCESS;
cleanup:
    if (status != EXIT_SUCCESS) fprintf(stderr, "Image regression failed: %s\n", SDL_GetError());
    destroyimg(&invalid);
    destroyimg(&img);
    if (fixture) SDL_DestroySurface(fixture);
    remove(path);
    current_window = &other;    destroywin();
    current_window = &win;    destroywin();
    return status;
}

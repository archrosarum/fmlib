#include "fmgui.h"
#include <stdio.h>
#include <stdlib.h>

/* Informational benchmark, deliberately separate from correctness tests.
 * Run both revisions on the same renderer/machine for a useful comparison. */
int main(void)
{
    int status = EXIT_FAILURE;
    window win = mkwin();
    image picture = {0};
    current_window = &win;
    if (!win.is_open || !SDL_SetRenderVSync(win.renptr, 0)) goto cleanup;
    rect area = mkrect(&win.root, newvecmes(newmes(0, 200), newmes(0, 60)),
        (vec_mes){0}, (vec_mes){0});
    text label = mktext("Repeated UI label");
    label.color = (rgba){255, 255, 255, 255};
    picture.surface = SDL_CreateSurface(512, 512, SDL_PIXELFORMAT_RGBA32);
    if (!picture.surface || !SDL_FillSurfaceRect(picture.surface, NULL,
        SDL_MapSurfaceRGBA(picture.surface, 255, 100, 50, 255)) ||
        !rendtxt(&area, &label) || !rendimg(&area, &picture) || !preswin()) goto cleanup;
    Uint64 start = SDL_GetTicksNS();
    for (int i = 0; i < 1000; ++i)
        if (!rendtxt(&area, &label) || !preswin()) goto cleanup;
    printf("1000 text draws: %.3f ms\n", (SDL_GetTicksNS() - start) / 1e6);
    start = SDL_GetTicksNS();
    for (int i = 0; i < 1000; ++i)
        if (!rendimg(&area, &picture) || !preswin()) goto cleanup;
    printf("1000 image draws: %.3f ms\n", (SDL_GetTicksNS() - start) / 1e6);
    SDL_FRect bounds;
    start = SDL_GetTicksNS();
    for (int i = 0; i < 1000000; ++i)
        if (!resolve_rect(&area, &bounds)) goto cleanup;
    printf("1000000 layout queries: %.3f ms\n", (SDL_GetTicksNS() - start) / 1e6);
    status = EXIT_SUCCESS;
cleanup:
    if (status != EXIT_SUCCESS) fprintf(stderr, "Benchmark failed: %s\n", SDL_GetError());
    destroyimg(&picture);
    destroywin();
    return status;
}

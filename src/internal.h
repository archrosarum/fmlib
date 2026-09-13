#ifndef FMGUI_INTERNAL_H
#define FMGUI_INTERNAL_H

#include "fmgui.h"

typedef struct {
    float density;
    bool drawable;
} fmgui_output;

bool fmgui_resolve_measure(int extent, mes value, int* out);
bool fmgui_resolve_vec_measure(int x_extent, int y_extent, vec_mes value, vec_i* out);
bool fmgui_resolve_vec_measure_precise(int x_extent, int y_extent, vec_mes value,
    double* x, double* y);

/* Refresh the current target's window-coordinate to pixel transform. */
bool fmgui_prepare_output(window* win, fmgui_output* output);

bool fmgui_resolve_rect(const rect* rec, SDL_FRect* result, window** owner);

/* Cache handles are borrowed until the next cache operation on the main thread.
 * Oversized entries and allocation failures use a temporary texture instead. */
typedef struct {
    SDL_Texture* texture;
    int width, height;
    bool temporary;
} fmgui_texture;

bool fmgui_text_texture(SDL_Renderer* renderer, const text* txt,
    float raster_size, int wrap_width, fmgui_texture* out);
bool fmgui_image_texture(SDL_Renderer* renderer, SDL_Surface* surface, fmgui_texture* out);
void fmgui_release_render_cache(SDL_Renderer* renderer);
void fmgui_release_image_cache(SDL_Surface* surface);

#endif

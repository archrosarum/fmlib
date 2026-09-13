#include "internal.h"
#include <math.h>

bool fmgui_prepare_output(window* win, fmgui_output* output)
{
    if (!win || !win->winptr || !win->renptr || !output) return false;
    *output = (fmgui_output){.density = 1};
    if (!SDL_GetRenderTarget(win->renptr) &&
        (SDL_GetWindowFlags(win->winptr) & SDL_WINDOW_MINIMIZED)) return true;
    int width, height, pixels_w, pixels_h;
    if (!SDL_GetWindowSize(win->winptr, &width, &height)) return false;
    win->size = (vec_i){width, height};
    /* This backend owns the scale. A logical intermediate buffer would blur
     * text and risks applying the density twice. Clip/viewport stay untouched. */
    if (!SDL_SetRenderLogicalPresentation(win->renptr, 0, 0, SDL_LOGICAL_PRESENTATION_DISABLED) ||
        !SDL_GetCurrentRenderOutputSize(win->renptr, &pixels_w, &pixels_h)) return false;
    if (width <= 0 || height <= 0 || pixels_w <= 0 || pixels_h <= 0) return true;
    float sx = (float)((double)pixels_w / width);
    float sy = (float)((double)pixels_h / height);
    if (!isfinite(sx) || !isfinite(sy) || sx <= 0 || sy <= 0)
        return SDL_SetError("Invalid renderer pixel density");
    if (!SDL_SetRenderScale(win->renptr, sx, sy)) return false;
    /* Rasterize at the larger density, then map pixels back to layout units.
     * Also works with fractional and nonuniform target scales. */
    output->density = sx > sy ? sx : sy;
    output->drawable = true;
    return true;
}

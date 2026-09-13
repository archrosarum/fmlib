#include "internal.h"
#include <limits.h>
#include <math.h>

bool rendrect(rect* rec, rgba color)
{
    SDL_FRect bounds;
    window* win = NULL;
    if (!fmgui_resolve_rect(rec, &bounds, &win) || !win || !win->is_open || !win->renptr) return false;
    fmgui_output output;
    if (!fmgui_prepare_output(win, &output)) return false;
    if (!output.drawable) return true;
    return SDL_SetRenderDrawColor(win->renptr, color.r, color.g, color.b, color.a) &&
           SDL_RenderFillRect(win->renptr, &bounds);
}

bool rendtxt(const rect* rec, const text* txt)
{
    if (!txt || !txt->content || !isfinite(txt->font_size) || txt->font_size <= 0 ||
        (txt->justify != TEXT_LEFT && txt->justify != TEXT_RIGHT && txt->justify != TEXT_CENTER))
        return SDL_SetError("Invalid text, font size, or alignment");

    SDL_FRect dst;
    window* win = NULL;
    if (!fmgui_resolve_rect(rec, &dst, &win) || !win || !win->is_open || !win->renptr)
        return SDL_SetError("Text requires a valid rectangle and open window");
    fmgui_output output;
    if (!fmgui_prepare_output(win, &output)) return false;
    if (!output.drawable || !*txt->content || dst.w == 0 || dst.h == 0) return true;
    double density = output.density;
    double raster_size = (double)txt->font_size * density;
    double wrap_width = floor((double)dst.w * density);
    /* FreeType uses signed 26.6 point sizes; validate before its conversion.
     * A zero wrap width means "unbounded" in SDL_ttf, so clamp tiny boxes. */
    if (!isfinite(raster_size) || raster_size < 1.0 / 64 || raster_size > INT_MAX / 64 - 1 ||
        !isfinite(wrap_width) || wrap_width > INT_MAX)
        return SDL_SetError("Text raster size is outside the supported range");
    if (wrap_width < 1) wrap_width = 1;
    fmgui_texture cached;
    if (!fmgui_text_texture(win->renptr, txt, (float)raster_size, (int)wrap_width, &cached))
        return false;
    /* Crop the source instead of changing the renderer's shared clip state. */
    double width = cached.width / density;
    double height = cached.height / density;
    double offset = 0;
    if (txt->justify == TEXT_RIGHT) offset = dst.w - width;
    if (txt->justify == TEXT_CENTER) offset = (dst.w - width) / 2;
    double crop = offset < 0 ? -offset : 0;
    SDL_FRect src = {(float)(crop * density), 0, 0, 0};
    if (offset > 0) { dst.x += (float)offset; dst.w -= (float)offset; }
    if (dst.w > width - crop) dst.w = (float)(width - crop);
    if (dst.h > height) dst.h = (float)height;
    src.w = (float)(dst.w * density);
    src.h = (float)(dst.h * density);
    bool ok = SDL_RenderTexture(win->renptr, cached.texture, &src, &dst);
    if (cached.temporary) SDL_DestroyTexture(cached.texture);
    return ok;
}

bool rendimg(const rect* rec, const image* img)
{
    SDL_FRect dst;
    window* win = NULL;
    if (!img || !img->surface || !fmgui_resolve_rect(rec, &dst, &win) ||
        !win || !win->is_open || !win->renptr)
        return SDL_SetError("Image requires loaded pixels, a valid rectangle, and an open window");
    fmgui_output output;
    if (!fmgui_prepare_output(win, &output)) return false;
    if (!output.drawable || dst.w == 0 || dst.h == 0) return true;
    fmgui_texture cached;
    if (!fmgui_image_texture(win->renptr, img->surface, &cached)) return false;
    bool ok = SDL_RenderTexture(win->renptr, cached.texture, NULL, &dst);
    if (cached.temporary) SDL_DestroyTexture(cached.texture);
    return ok;
}

bool clrwin(void)
{
    window* win = current_window;
    fmgui_output output;
    if (!win || !win->is_open || !fmgui_prepare_output(win, &output)) return false;
    if (!output.drawable) return true;
    return SDL_SetRenderDrawColor(win->renptr, win->bg.r, win->bg.g, win->bg.b, win->bg.a) &&
        SDL_RenderClear(win->renptr);
}

bool preswin(void)
{
    window* win = current_window;
    fmgui_output output;
    if (!win || !win->is_open || !fmgui_prepare_output(win, &output)) return false;
    return !output.drawable || SDL_RenderPresent(win->renptr);
}

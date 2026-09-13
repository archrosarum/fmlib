#include "internal.h"
#include <limits.h>
#include <math.h>

static bool resolve_measure_precise(int extent, mes value, double* out)
{
    double result = (double)extent * value.pct + value.px;
    if (!out || !isfinite(result)) return false;
    *out = result;
    return true;
}

bool fmgui_resolve_measure(int extent, mes value, int* out)
{
    double result;
    if (!out || !resolve_measure_precise(extent, value, &result) ||
        result < INT_MIN || result > INT_MAX) return false;
    *out = (int)result;
    return true;
}

bool fmgui_resolve_vec_measure_precise(int x_extent, int y_extent, vec_mes value,
    double* x, double* y)
{
    if (!x || !y) return false;
    switch (value.mode) {
        case VEC_MES_INDEPENDENT:
            return resolve_measure_precise(x_extent, value.x, x) &&
                resolve_measure_precise(y_extent, value.y, y);
        case VEC_MES_FROM_X:
            if (!resolve_measure_precise(x_extent, value.x, x)) return false;
            *y = *x;
            return true;
        case VEC_MES_FROM_Y:
            if (!resolve_measure_precise(y_extent, value.y, y)) return false;
            *x = *y;
            return true;
        default:
            return false;
    }
}

bool fmgui_resolve_vec_measure(int x_extent, int y_extent, vec_mes value, vec_i* out)
{
    double x, y;
    if (!out || !fmgui_resolve_vec_measure_precise(x_extent, y_extent, value, &x, &y) ||
        x < INT_MIN || x > INT_MAX || y < INT_MIN || y > INT_MAX) return false;
    out->x = (int)x;
    out->y = (int)y;
    return true;
}

mes newmes(float pct, int px) { return (mes){pct, px}; }
vec_mes newvecmes(mes x, mes y)
{
    return (vec_mes){.x = x, .y = y, .mode = VEC_MES_INDEPENDENT};
}

vec_mes vecmes_from_x(mes x)
{
    return (vec_mes){.x = x, .y = x, .mode = VEC_MES_FROM_X};
}

vec_mes vecmes_from_y(mes y)
{
    return (vec_mes){.x = y, .y = y, .mode = VEC_MES_FROM_Y};
}

int resmes(dimention dim, mes value)
{
    SDL_Window* win = current_window ? current_window->winptr : NULL;
    int w, h, result;
    if (!win || (dim != WIDTH && dim != HEIGHT) || !SDL_GetWindowSize(win, &w, &h) ||
        !fmgui_resolve_measure(dim == WIDTH ? w : h, value, &result)) return 0;
    return result;
}

vec_i resvecmes(vec_mes value)
{
    SDL_Window* win = current_window ? current_window->winptr : NULL;
    int w, h;
    vec_i result = {0};
    if (!win || !SDL_GetWindowSize(win, &w, &h) ||
        !fmgui_resolve_vec_measure(w, h, value, &result)) return (vec_i){0};
    return result;
}

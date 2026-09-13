#include "internal.h"
#include <SDL3_image/SDL_image.h>

image mkimg(const char* path)
{
    if (!path || !*path) {
        SDL_SetError("Image requires a file path");
        return (image){0};
    }
    return (image){.surface = IMG_Load(path)};
}

void destroyimg(image* img)
{
    if (!img) return;
    fmgui_release_image_cache(img->surface);
    if (img->surface) SDL_DestroySurface(img->surface);
    *img = (image){0};
}

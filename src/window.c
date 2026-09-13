#include "internal.h"

window* current_window = NULL;

static void release_window(window* win)
{
    if (!win) return;
    fmgui_release_render_cache(win->renptr);
    if (win->renptr) SDL_DestroyRenderer(win->renptr);
    if (win->winptr) SDL_DestroyWindow(win->winptr);
    if (win->owns_sdl) SDL_QuitSubSystem(SDL_INIT_VIDEO);
    *win = (window){0};
}

window mkwin(void)
{
    window win = {0};
    win.root.is_winrect = true;
    win.title = "Window";
    win.size = (vec_i){640, 480};
    win.bg = (rgba){0, 0, 0, 255};
    if (!SDL_InitSubSystem(SDL_INIT_VIDEO)) return win;
    win.owns_sdl = true;
    if (!SDL_CreateWindowAndRenderer(win.title, win.size.x, win.size.y,
            SDL_WINDOW_BORDERLESS | SDL_WINDOW_RESIZABLE | SDL_WINDOW_TRANSPARENT | SDL_WINDOW_HIGH_PIXEL_DENSITY,
            &win.winptr, &win.renptr) ||
        !SDL_SetRenderDrawBlendMode(win.renptr, SDL_BLENDMODE_BLEND)) {
        release_window(&win);
        return win;
    }
    /* Request one presentation per refresh. Unsupported backends can still
     * create a usable window (for example headless/software renderers). */
    if (!SDL_SetRenderVSync(win.renptr, 1)) SDL_ClearError();
    fmgui_output output;
    if (!fmgui_prepare_output(&win, &output)) {
        release_window(&win);
        return win;
    }
    win.is_open = 1;
    return win;
}

void destroywin(void)
{
    window* win = current_window;
    current_window = NULL;
    release_window(win);
}

bool updwin(void)
{
    window* win = current_window;
    if (!win || !win->is_open || !win->winptr || !win->renptr) return false;
    win->previous_mouse_down = win->mouse_down;
    win->previous_mouse_valid = win->mouse_valid;
    win->previous_mouse_x = win->mouse_x;
    win->previous_mouse_y = win->mouse_y;
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_RENDER_DEVICE_RESET) {
            SDL_Window* target = SDL_GetWindowFromID(event.render.windowID);
            if (target) fmgui_release_render_cache(SDL_GetRenderer(target));
        }
        if (event.type == SDL_EVENT_QUIT) {
            int count = 0;
            SDL_Window** windows = SDL_GetWindows(&count);
            for (int i = 0; windows && i < count; ++i)
                SDL_SetBooleanProperty(SDL_GetWindowProperties(windows[i]), "fmgui.closed", true);
            SDL_free(windows);
            win->is_open = 0;
        } else if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED) {
            SDL_Window* target = SDL_GetWindowFromID(event.window.windowID);
            if (target) SDL_SetBooleanProperty(SDL_GetWindowProperties(target), "fmgui.closed", true);
        }
    }
    if (SDL_GetBooleanProperty(SDL_GetWindowProperties(win->winptr), "fmgui.closed", false))
        win->is_open = 0;
    fmgui_output output;
    if (!fmgui_prepare_output(win, &output)) return false;
    SDL_MouseButtonFlags buttons = SDL_GetMouseState(&win->mouse_x, &win->mouse_y);
    win->mouse_valid = SDL_GetMouseFocus() == win->winptr;
    win->mouse_down = (buttons & (SDL_BUTTON_MASK(SDL_BUTTON_LEFT) |
                                  SDL_BUTTON_MASK(SDL_BUTTON_RIGHT))) != 0;
    return win->is_open != 0;
}

void modwinbg(rgba bg)
{
    window* win = current_window;
    if (win) win->bg = bg;
}

bool modwin(const char* title, vec_i size)
{
    window* win = current_window;
    if (!win || !win->is_open || !win->winptr || !title || size.x <= 0 || size.y <= 0)
        return SDL_SetError("modwin requires a current window, title, and positive size");
    if (!SDL_SetWindowSize(win->winptr, size.x, size.y)) return false;
    /* Query the actual size, which a window manager may constrain. */
    if (!SDL_GetWindowSize(win->winptr, &win->size.x, &win->size.y)) return false;
    if (!SDL_SetWindowTitle(win->winptr, title)) return false;
    win->title = SDL_GetWindowTitle(win->winptr);
    fmgui_output output;
    return fmgui_prepare_output(win, &output);
}

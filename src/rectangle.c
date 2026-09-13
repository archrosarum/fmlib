#include "internal.h"
#include <limits.h>
#include <stddef.h>
#include <math.h>

rect mkrect(rect* parent, vec_mes size, vec_mes pos, vec_mes anchor)
{
    return (rect){.pos = pos, .size = size, .anchor = anchor, .parent = parent};
}

static const rect* next_parent(const rect* rec)
{
    return rec && !rec->is_winrect ? rec->parent : NULL;
}

window* rectwin(const rect* rec)
{
    /* Reject cycles before walking or resolving the chain. */
    const rect* slow = rec;
    const rect* fast = rec;
    while (fast && next_parent(fast)) {
        slow = next_parent(slow);
        fast = next_parent(next_parent(fast));
        if (slow == fast) return NULL;
    }
    if (!rec) return NULL;
    while (!rec->is_winrect && rec->parent) rec = rec->parent;
    if (!rec->is_winrect || rec->parent) return NULL;
    /* Roots are embedded, so mkwin can safely return a window by value
     * without storing a pointer to its temporary local window. */
    return (window*)((char*)rec - offsetof(window, root));
}

bool fmgui_resolve_rect(const rect* rec, SDL_FRect* result, window** owner)
{
    window* win = rectwin(rec);
    int w, h;
    if (!result || !win || !win->winptr ||
        !SDL_GetWindowSize(win->winptr, &w, &h)) return false;
    size_t count = 0;
    for (const rect* p = rec; !p->is_winrect; p = p->parent) ++count;
    /* Ordinary UI trees fit on the stack; keep a heap fallback for deep trees. */
    const rect* local_chain[32];
    if (count > SIZE_MAX / sizeof(*local_chain)) return false;
    const rect** chain = count <= SDL_arraysize(local_chain) ? local_chain :
        SDL_malloc(count * sizeof(*chain));
    if (!chain) return false;
    const rect* p = rec;
    for (size_t i = 0; i < count; ++i, p = p->parent) chain[i] = p;
    double left = 0, top = 0;
    bool valid = true;
    while (count) {
        p = chain[--count];
        vec_i position, size;
        if (!fmgui_resolve_vec_measure(w, h, p->pos, &position) ||
            !fmgui_resolve_vec_measure(w, h, p->size, &size) ||
            size.x < 0 || size.y < 0) { valid = false; break; }
        /* The anchor is a point inside the child: percentage plus raw offset.
         * Positive px shifts the child left/up; negative px shifts it right/down.
         * Keep fractional anchor values until the final render conversion. */
        double anchor_x, anchor_y;
        if (!fmgui_resolve_vec_measure_precise(size.x, size.y, p->anchor,
                &anchor_x, &anchor_y)) { valid = false; break; }
        left += position.x - anchor_x;
        top += position.y - anchor_y;
        if (!isfinite(left) || !isfinite(top) || left < INT_MIN || left > INT_MAX ||
            top < INT_MIN || top > INT_MAX) { valid = false; break; }
        w = size.x;
        h = size.y;
    }
    if (chain != local_chain) SDL_free(chain);
    if (!valid) return false;
    *result = (SDL_FRect){(float)left, (float)top, (float)w, (float)h};
    if (owner) *owner = win;
    return true;
}

bool resolve_rect(const rect* rec, SDL_FRect* result)
{
    return fmgui_resolve_rect(rec, result, NULL);
}

static bool inside(float x, float y, SDL_FRect r)
{
    return x >= r.x && y >= r.y && (double)x < (double)r.x + r.w &&
           (double)y < (double)r.y + r.h;
}

int chkrect(rect* rec, event evt)
{
    SDL_FRect bounds;
    window* win = NULL;
    if (!fmgui_resolve_rect(rec, &bounds, &win) || !win->is_open) return 0;
    bool now = win->mouse_valid && inside(win->mouse_x, win->mouse_y, bounds);
    bool before = win->previous_mouse_valid &&
        inside(win->previous_mouse_x, win->previous_mouse_y, bounds);
    switch (evt) {
        case MOUSE_IN: return now;
        case MOUSE_OUT: return !now;
        case MOUSE_ENTER: return now && !before;
        case MOUSE_LEAVE: return before && !now;
        case MOUSE_DOWN: return now && win->mouse_down;
        case MOUSE_UP: return now && !win->mouse_down;
        case M_PRESS: return now && win->mouse_down && !win->previous_mouse_down;
        case M_RELEASE: return now && !win->mouse_down && win->previous_mouse_down;
        default: return 0;
    }
}

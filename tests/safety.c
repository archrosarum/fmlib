#include "fmgui.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int main(void)
{
    int status = EXIT_FAILURE;
    window win = {0};
    window created = {0};
    rect moving = {.pos = {.x = {0, -100}, .y = {1, 100}}};
    anim motion = mkanim((vec_mes){.x = {1, 100}, .y = {0, -100}}, 2);
    if (!(!updanim(&motion, 1))) {
        fprintf(stderr, "%s:%d: failed: !updanim(&motion, 1)\n", __FILE__, __LINE__);
        goto cleanup;
    }
    if (!(!startanim(NULL, &moving))) {
        fprintf(stderr, "%s:%d: failed: !startanim(NULL, &moving)\n", __FILE__, __LINE__);
        goto cleanup;
    }
    if (!(!startanim(&motion, NULL))) {
        fprintf(stderr, "%s:%d: failed: !startanim(&motion, NULL)\n", __FILE__, __LINE__);
        goto cleanup;
    }
    if (!(startanim(&motion, &moving))) {
        fprintf(stderr, "%s:%d: failed: startanim(&motion, &moving)\n", __FILE__, __LINE__);
        goto cleanup;
    }
    if (!(updanim(&motion, NAN))) {
        fprintf(stderr, "%s:%d: failed: updanim(&motion, NAN)\n", __FILE__, __LINE__);
        goto cleanup;
    }
    if (!(updanim(&motion, -1))) {
        fprintf(stderr, "%s:%d: failed: updanim(&motion, -1)\n", __FILE__, __LINE__);
        goto cleanup;
    }
    if (!(moving.pos.x.px == -100)) {
        fprintf(stderr, "%s:%d: failed: moving.pos.x.px == -100\n", __FILE__, __LINE__);
        goto cleanup;
    }
    if (!(updanim(&motion, 1))) {
        fprintf(stderr, "%s:%d: failed: updanim(&motion, 1)\n", __FILE__, __LINE__);
        goto cleanup;
    }
    if (!(moving.pos.x.pct == .5f && moving.pos.x.px == 0)) {
        fprintf(stderr, "%s:%d: failed: moving.pos.x.pct == .5f && moving.pos.x.px == 0\n", __FILE__, __LINE__);
        goto cleanup;
    }
    if (!(moving.pos.y.pct == .5f && moving.pos.y.px == 0)) {
        fprintf(stderr, "%s:%d: failed: moving.pos.y.pct == .5f && moving.pos.y.px == 0\n", __FILE__, __LINE__);
        goto cleanup;
    }
    if (!(!updanim(&motion, 10))) {
        fprintf(stderr, "%s:%d: failed: !updanim(&motion, 10)\n", __FILE__, __LINE__);
        goto cleanup;
    }
    if (!(moving.pos.x.pct == 1 && moving.pos.x.px == 100)) {
        fprintf(stderr, "%s:%d: failed: moving.pos.x.pct == 1 && moving.pos.x.px == 100\n", __FILE__, __LINE__);
        goto cleanup;
    }
    if (!(moving.pos.y.pct == 0 && moving.pos.y.px == -100)) {
        fprintf(stderr, "%s:%d: failed: moving.pos.y.pct == 0 && moving.pos.y.px == -100\n", __FILE__, __LINE__);
        goto cleanup;
    }
    if (!(!updanim(&motion, 1))) {
        fprintf(stderr, "%s:%d: failed: !updanim(&motion, 1)\n", __FILE__, __LINE__);
        goto cleanup;
    }
    motion.goal = (vec_mes){.x = {0, 0}, .y = {0, 0}};
    if (!(startanim(&motion, &moving))) {
        fprintf(stderr, "%s:%d: failed: startanim(&motion, &moving)\n", __FILE__, __LINE__);
        goto cleanup;
    }
    if (!(updanim(&motion, 1))) {
        fprintf(stderr, "%s:%d: failed: updanim(&motion, 1)\n", __FILE__, __LINE__);
        goto cleanup;
    }
    if (!(startanim(&motion, &moving))) {
        fprintf(stderr, "%s:%d: failed: startanim(&motion, &moving)\n", __FILE__, __LINE__);
        goto cleanup;
    }
    if (!(updanim(&motion, 1))) {
        fprintf(stderr, "%s:%d: failed: updanim(&motion, 1)\n", __FILE__, __LINE__);
        goto cleanup;
    }
    if (!(moving.pos.x.px == 25)) {
        fprintf(stderr, "%s:%d: failed: moving.pos.x.px == 25\n", __FILE__, __LINE__);
        goto cleanup;
    }
    rect other = {0};
    motion.seconds = 0;
    motion.goal.x.px = 80;
    if (!(startanim(&motion, &other))) {
        fprintf(stderr, "%s:%d: failed: startanim(&motion, &other)\n", __FILE__, __LINE__);
        goto cleanup;
    }
    if (!(other.pos.x.px == 80 && moving.pos.x.px == 25)) {
        fprintf(stderr, "%s:%d: failed: other.pos.x.px == 80 && moving.pos.x.px == 25\n", __FILE__, __LINE__);
        goto cleanup;
    }
    if (!(!updanim(&motion, 1))) {
        fprintf(stderr, "%s:%d: failed: !updanim(&motion, 1)\n", __FILE__, __LINE__);
        goto cleanup;
    }
    rect constrained = {.pos = vecmes_from_x(newmes(.25f, 4))};
    anim constrained_motion = mkanim(vecmes_from_x(newmes(.75f, 12)), 2);
    if (!(startanim(&constrained_motion, &constrained) &&
            updanim(&constrained_motion, 1))) {
        fprintf(stderr, "%s:%d: failed: matched vec_mes animation\n", __FILE__, __LINE__);
        goto cleanup;
    }
    if (!(constrained.pos.mode == VEC_MES_FROM_X && constrained.pos.x.pct == .5f &&
            constrained.pos.x.px == 8)) {
        fprintf(stderr, "%s:%d: failed: matched vec_mes interpolation\n", __FILE__, __LINE__);
        goto cleanup;
    }
    constrained_motion.goal = vecmes_from_y(newmes(.5f, 0));
    if (!(!startanim(&constrained_motion, &constrained))) {
        fprintf(stderr, "%s:%d: failed: reject mismatched vec_mes animation modes\n",
            __FILE__, __LINE__);
        goto cleanup;
    }
    motion.seconds = -1;
    if (!(!startanim(&motion, &other))) {
        fprintf(stderr, "%s:%d: failed: !startanim(&motion, &other)\n", __FILE__, __LINE__);
        goto cleanup;
    }
    motion.seconds = INFINITY;
    if (!(!startanim(&motion, &other))) {
        fprintf(stderr, "%s:%d: failed: !startanim(&motion, &other)\n", __FILE__, __LINE__);
        goto cleanup;
    }
    motion.seconds = 1;
    motion.goal.x.pct = NAN;
    if (!(!startanim(&motion, &other))) {
        fprintf(stderr, "%s:%d: failed: !startanim(&motion, &other)\n", __FILE__, __LINE__);
        goto cleanup;
    }
    if (!(!updwin())) {
        fprintf(stderr, "%s:%d: failed: !updwin()\n", __FILE__, __LINE__);
        goto cleanup;
    }
    if (!(!clrwin())) {
        fprintf(stderr, "%s:%d: failed: !clrwin()\n", __FILE__, __LINE__);
        goto cleanup;
    }
    if (!(!preswin())) {
        fprintf(stderr, "%s:%d: failed: !preswin()\n", __FILE__, __LINE__);
        goto cleanup;
    }
    if (!(!rendrect(NULL, (rgba){0}))) {
        fprintf(stderr, "%s:%d: failed: !rendrect(NULL, (rgba){0})\n", __FILE__, __LINE__);
        goto cleanup;
    }
    if (!(!chkrect(NULL, M_PRESS))) {
        fprintf(stderr, "%s:%d: failed: !chkrect(NULL, M_PRESS)\n", __FILE__, __LINE__);
        goto cleanup;
    }
    current_window = NULL;    destroywin();
    modwinbg((rgba){0});
    if (!(resmes(WIDTH, newmes(1, 0)) == 0)) {
        fprintf(stderr, "%s:%d: failed: resmes(WIDTH, newmes(1, 0)) == 0\n", __FILE__, __LINE__);
        goto cleanup;
    }
    window empty = {0};
    current_window = &empty;    destroywin();
    current_window = &empty;    destroywin();
    if (!(SDL_Init(SDL_INIT_VIDEO))) {
        fprintf(stderr, "%s:%d: failed: SDL_Init(SDL_INIT_VIDEO)\n", __FILE__, __LINE__);
        goto cleanup;
    }
    current_window = &win;
    win.root.is_winrect = true;
    win.winptr = SDL_CreateWindow("test", 100, 100, SDL_WINDOW_HIDDEN);
    if (!(win.winptr)) {
        fprintf(stderr, "%s:%d: failed: win.winptr\n", __FILE__, __LINE__);
        goto cleanup;
    }
    win.is_open = 1;
    rect rec = mkrect(&win.root, newvecmes(newmes(0, 20), newmes(0, 20)),
        newvecmes(newmes(0, 20), newmes(0, 20)),
        newvecmes(newmes(.5f, 2), newmes(.5f, 2)));
    SDL_FRect bounds;
    if (!(resolve_rect(&rec, &bounds))) {
        fprintf(stderr, "%s:%d: failed: resolve_rect(&rec, &bounds)\n", __FILE__, __LINE__);
        goto cleanup;
    }
    if (!(bounds.x == 8 && bounds.y == 8 && bounds.w == 20)) {
        fprintf(stderr, "%s:%d: failed: bounds.x == 8 && bounds.y == 8 && bounds.w == 20\n", __FILE__, __LINE__);
        goto cleanup;
    }
    if (!(rectwin(&rec) == &win)) {
        fprintf(stderr, "%s:%d: failed: rectwin(&rec) == &win\n", __FILE__, __LINE__);
        goto cleanup;
    }
    rect child = mkrect(&rec, (vec_mes){.x = {.5f, 0}, .y = {.5f, 0}},
        (vec_mes){.x = {.5f, 2}, .y = {.5f, 3}},
        (vec_mes){.x = {.5f, 1}, .y = {.5f, 2}});
    if (!(resolve_rect(&child, &bounds))) {
        fprintf(stderr, "%s:%d: failed: resolve_rect(&child, &bounds)\n", __FILE__, __LINE__);
        goto cleanup;
    }
    if (!(bounds.x == 14 && bounds.y == 14 && bounds.w == 10 && bounds.h == 10)) {
        fprintf(stderr, "%s:%d: failed: bounds.x == 14 && bounds.y == 14 && bounds.w == 10 && bounds.h == 10\n", __FILE__, __LINE__);
        goto cleanup;
    }
    rect grandchild = mkrect(&child, (vec_mes){.x = {.5f, 0}, .y = {1, 0}},
        (vec_mes){.x = {1, 0}, .y = {0, 0}}, (vec_mes){0});
    if (!(resolve_rect(&grandchild, &bounds))) {
        fprintf(stderr, "%s:%d: failed: resolve_rect(&grandchild, &bounds)\n", __FILE__, __LINE__);
        goto cleanup;
    }
    if (!(bounds.x == 24 && bounds.y == 14 && bounds.w == 5 && bounds.h == 10)) {
        fprintf(stderr, "%s:%d: failed: bounds.x == 24 && bounds.y == 14 && bounds.w == 5 && bounds.h == 10\n", __FILE__, __LINE__);
        goto cleanup;
    }
    child.parent = &grandchild;
    if (!(!resolve_rect(&child, &bounds))) {
        fprintf(stderr, "%s:%d: failed: !resolve_rect(&child, &bounds)\n", __FILE__, __LINE__);
        goto cleanup;
    }
    if (!(!rectwin(&grandchild))) {
        fprintf(stderr, "%s:%d: failed: !rectwin(&grandchild)\n", __FILE__, __LINE__);
        goto cleanup;
    }
    child.parent = NULL;
    if (!(!resolve_rect(&child, &bounds))) {
        fprintf(stderr, "%s:%d: failed: !resolve_rect(&child, &bounds)\n", __FILE__, __LINE__);
        goto cleanup;
    }
    child.parent = &child;
    if (!(!resolve_rect(&child, &bounds))) {
        fprintf(stderr, "%s:%d: failed: !resolve_rect(&child, &bounds)\n", __FILE__, __LINE__);
        goto cleanup;
    }
    child.parent = &win.root;
    if (!(resolve_rect(&child, &bounds))) {
        fprintf(stderr, "%s:%d: failed: resolve_rect(&child, &bounds)\n", __FILE__, __LINE__);
        goto cleanup;
    }
    if (!(bounds.w == 50)) {
        fprintf(stderr, "%s:%d: failed: bounds.w == 50\n", __FILE__, __LINE__);
        goto cleanup;
    }
    if (!(SDL_SetWindowSize(win.winptr, 200, 160))) {
        fprintf(stderr, "%s:%d: failed: SDL_SetWindowSize(win.winptr, 200, 160)\n", __FILE__, __LINE__);
        goto cleanup;
    }
    if (!(resolve_rect(&win.root, &bounds))) {
        fprintf(stderr, "%s:%d: failed: resolve_rect(&win.root, &bounds)\n", __FILE__, __LINE__);
        goto cleanup;
    }
    if (!(bounds.x == 0 && bounds.y == 0 && bounds.w == 200 && bounds.h == 160)) {
        fprintf(stderr, "%s:%d: failed: bounds.x == 0 && bounds.y == 0 && bounds.w == 200 && bounds.h == 160\n", __FILE__, __LINE__);
        goto cleanup;
    }
    if (!(resolve_rect(&child, &bounds))) {
        fprintf(stderr, "%s:%d: failed: resolve_rect(&child, &bounds)\n", __FILE__, __LINE__);
        goto cleanup;
    }
    if (!(bounds.x == 51 && bounds.y == 41 && bounds.w == 100 && bounds.h == 80)) {
        fprintf(stderr, "%s:%d: failed: bounds.x == 51 && bounds.y == 41 && bounds.w == 100 && bounds.h == 80\n", __FILE__, __LINE__);
        goto cleanup;
    }
    if (!(SDL_SetWindowSize(win.winptr, 100, 100))) {
        fprintf(stderr, "%s:%d: failed: SDL_SetWindowSize(win.winptr, 100, 100)\n", __FILE__, __LINE__);
        goto cleanup;
    }
    win.mouse_valid = true;
    win.mouse_x = win.mouse_y = 8;
    win.mouse_down = true;
    if (!(chkrect(&rec, MOUSE_ENTER))) {
        fprintf(stderr, "%s:%d: failed: chkrect(&rec, MOUSE_ENTER)\n", __FILE__, __LINE__);
        goto cleanup;
    }
    if (!(chkrect(&rec, M_PRESS))) {
        fprintf(stderr, "%s:%d: failed: chkrect(&rec, M_PRESS)\n", __FILE__, __LINE__);
        goto cleanup;
    }
    if (!(!chkrect(&rec, M_RELEASE))) {
        fprintf(stderr, "%s:%d: failed: !chkrect(&rec, M_RELEASE)\n", __FILE__, __LINE__);
        goto cleanup;
    }
    if (!(chkrect(&rec, M_PRESS))) {
        fprintf(stderr, "%s:%d: failed: chkrect(&rec, M_PRESS)\n", __FILE__, __LINE__);
        goto cleanup;
    }
    win.previous_mouse_valid = true;
    win.previous_mouse_x = win.previous_mouse_y = 8;
    win.previous_mouse_down = true;
    win.mouse_down = false;
    if (!(!chkrect(&rec, M_PRESS))) {
        fprintf(stderr, "%s:%d: failed: !chkrect(&rec, M_PRESS)\n", __FILE__, __LINE__);
        goto cleanup;
    }
    if (!(chkrect(&rec, M_RELEASE))) {
        fprintf(stderr, "%s:%d: failed: chkrect(&rec, M_RELEASE)\n", __FILE__, __LINE__);
        goto cleanup;
    }
    if (!(chkrect(&rec, M_RELEASE))) {
        fprintf(stderr, "%s:%d: failed: chkrect(&rec, M_RELEASE)\n", __FILE__, __LINE__);
        goto cleanup;
    }
    win.mouse_x = 28;
    if (!(!chkrect(&rec, MOUSE_IN))) {
        fprintf(stderr, "%s:%d: failed: !chkrect(&rec, MOUSE_IN)\n", __FILE__, __LINE__);
        goto cleanup;
    }
    if (!(chkrect(&rec, MOUSE_LEAVE))) {
        fprintf(stderr, "%s:%d: failed: chkrect(&rec, MOUSE_LEAVE)\n", __FILE__, __LINE__);
        goto cleanup;
    }
    rec.size.x.pct = NAN;
    if (!(!resolve_rect(&rec, &bounds))) {
        fprintf(stderr, "%s:%d: failed: !resolve_rect(&rec, &bounds)\n", __FILE__, __LINE__);
        goto cleanup;
    }
    if (!(!chkrect(&rec, MOUSE_IN))) {
        fprintf(stderr, "%s:%d: failed: !chkrect(&rec, MOUSE_IN)\n", __FILE__, __LINE__);
        goto cleanup;
    }
    rec.size.x.pct = INFINITY;
    if (!(!resolve_rect(&rec, &bounds))) {
        fprintf(stderr, "%s:%d: failed: !resolve_rect(&rec, &bounds)\n", __FILE__, __LINE__);
        goto cleanup;
    }
    if (!(resmes((dimention)99, newmes(1, 0)) == 0)) {
        fprintf(stderr, "%s:%d: failed: resmes((dimention)99, newmes(1, 0)) == 0\n", __FILE__, __LINE__);
        goto cleanup;
    }
    if (!(resmes(WIDTH, newmes(INFINITY, 0)) == 0)) {
        fprintf(stderr, "%s:%d: failed: resmes(WIDTH, newmes(INFINITY, 0)) == 0\n", __FILE__, __LINE__);
        goto cleanup;
    }
    vec_mes invalid_vec = newvecmes(newmes(0, 1), newmes(0, 2));
    invalid_vec.mode = (vec_mes_mode)99;
    vec_i invalid_result = resvecmes(invalid_vec);
    if (!(invalid_result.x == 0 && invalid_result.y == 0)) {
        fprintf(stderr, "%s:%d: failed: invalid vec_mes mode\n", __FILE__, __LINE__);
        goto cleanup;
    }
    rec.size = invalid_vec;
    if (!(!resolve_rect(&rec, &bounds))) {
        fprintf(stderr, "%s:%d: failed: invalid rectangle vec_mes mode\n", __FILE__, __LINE__);
        goto cleanup;
    }
    current_window = &win;    destroywin();
    current_window = &win;    destroywin();
    SDL_Quit();
    created = mkwin();
    if (!(created.is_open && created.root.is_winrect)) {
        fprintf(stderr, "%s:%d: failed: created.is_open && created.root.is_winrect\n", __FILE__, __LINE__);
        goto cleanup;
    }
    if (!(rectwin(&created.root) == &created)) {
        fprintf(stderr, "%s:%d: failed: rectwin(&created.root) == &created\n", __FILE__, __LINE__);
        goto cleanup;
    }
    if (!(resolve_rect(&created.root, &bounds))) {
        fprintf(stderr, "%s:%d: failed: resolve_rect(&created.root, &bounds)\n", __FILE__, __LINE__);
        goto cleanup;
    }
    if (!(bounds.w == created.size.x && bounds.h == created.size.y)) {
        fprintf(stderr, "%s:%d: failed: bounds.w == created.size.x && bounds.h == created.size.y\n", __FILE__, __LINE__);
        goto cleanup;
    }
    current_window = &created;    destroywin();
    status = EXIT_SUCCESS;

cleanup:
    current_window = &created;    destroywin();
    current_window = &win;    destroywin();
    SDL_Quit();
    return status;
}

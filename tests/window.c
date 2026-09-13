#include "fmgui.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
    int status = EXIT_FAILURE;
    window first = mkwin();
    window second = {0};
    vec_i size = {320, 240};
    SDL_FRect bounds;

    if (!first.is_open || current_window != NULL) goto cleanup;
    if (updwin() || clrwin() || preswin() || modwin("Missing", size)) goto cleanup;
    current_window = &first;
    char title[] = "First";
    if (!modwin(title, size)) goto cleanup;
    title[0] = 'X';
    if (strcmp(first.title, "First") || strcmp(SDL_GetWindowTitle(first.winptr), "First") ||
        first.size.x != 320 || first.size.y != 240) goto cleanup;
    if (modwin(NULL, size) || modwin("Bad", (vec_i){0, 240}) ||
        strcmp(first.title, "First")) goto cleanup;
    if (resmes(WIDTH, newmes(.5f, 3)) != 163) goto cleanup;
    vec_i resolved = resvecmes((vec_mes){.x = {.5f, 0}, .y = {.5f, 0}});
    if (resolved.x != 160 || resolved.y != 120) goto cleanup;
    resolved = resvecmes(vecmes_from_x(newmes(.5f, 3)));
    if (resolved.x != 163 || resolved.y != 163) goto cleanup;
    resolved = resvecmes(vecmes_from_y(newmes(.5f, -2)));
    if (resolved.x != 118 || resolved.y != 118) goto cleanup;
    resolved = resvecmes(vecmes_from_x(newmes(0, 37)));
    if (resolved.x != 37 || resolved.y != 37) goto cleanup;
    modwinbg((rgba){20, 30, 40, 255});

    rect parent = mkrect(&first.root, (vec_mes){.x = {0, 100}, .y = {0, 80}},
        (vec_mes){.x = {0, 40}, .y = {0, 50}}, (vec_mes){0});
    rect child = mkrect(&parent, (vec_mes){.x = {0, 20}, .y = {0, 10}},
        (vec_mes){.x = {.5f, 0}, .y = {.5f, 0}},
        (vec_mes){.x = {0, 7}, .y = {0, -3}});
    if (!resolve_rect(&child, &bounds) || bounds.x != 83 || bounds.y != 93) goto cleanup;
    child.anchor = (vec_mes){.x = {.5f, 7}, .y = {.5f, -3}};
    if (!resolve_rect(&child, &bounds) || bounds.x != 73 || bounds.y != 88) goto cleanup;
    rect square = mkrect(&parent, vecmes_from_x(newmes(.5f, 3)),
        vecmes_from_y(newmes(.5f, 0)), (vec_mes){0});
    if (!resolve_rect(&square, &bounds) || bounds.x != 80 || bounds.y != 90 ||
        bounds.w != 53 || bounds.h != 53) goto cleanup;
    first.mouse_valid = true;
    first.mouse_x = 73;
    first.mouse_y = 88;
    if (!chkrect(&child, MOUSE_IN)) goto cleanup;

    second = mkwin();
    if (!second.is_open || current_window != &first) goto cleanup;
    current_window = &second;
    if (!modwin("Second", (vec_i){500, 400}) || resmes(WIDTH, newmes(.5f, 3)) != 253)
        goto cleanup;
    modwinbg((rgba){60, 70, 80, 255});
    if (first.bg.r != 20 || second.bg.r != 60 || first.size.x != 320 ||
        !updwin() || !clrwin() || !preswin()) goto cleanup;
    /* Rectangles remain rooted in their original window regardless of selection. */
    if (rectwin(&child) != &first || !resolve_rect(&child, &bounds) || bounds.x != 73 ||
        !chkrect(&child, MOUSE_IN) || !rendrect(&child, (rgba){255, 0, 0, 255}) ||
        current_window != &second) goto cleanup;
    destroywin();
    if (current_window != NULL || second.winptr || !first.is_open || clrwin()) goto cleanup;
    destroywin();
    current_window = &first;
    if (resmes(WIDTH, newmes(.5f, 3)) != 163 || !clrwin() || !preswin()) goto cleanup;
    status = EXIT_SUCCESS;

cleanup:
    if (status != EXIT_SUCCESS) fprintf(stderr, "Window selection regression failed: %s\n", SDL_GetError());
    current_window = &second;
    destroywin();
    current_window = &first;
    destroywin();
    return status;
}

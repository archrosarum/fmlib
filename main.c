#include "fmgui.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[])
{
    window win = mkwin();
    current_window = &win;
    modwinbg((rgba){255, 255, 255, 255});

    /* reze */
    rect reze_hitbox = mkrect(
        &win.root,
        vecmes_from_x((mes){0.5, 0}),
        (vec_mes){.x = {0, 0}, .y = {1, 0}},
        (vec_mes){.x = {0, 0}, .y = {1, 0}}
    );
    image reze_img = mkimg(
        "reze.png"
    );

    /* hello */
    rect hello_hitbox = mkrect(
        &win.root,
        (vec_mes){.x = {0.5, 0}, .y = {0.5, 0}},
        (vec_mes){.x = {0.5, 0}, .y = {0.3, 0}},
        (vec_mes){.x = {0.5, 0}, .y = {0.5, 0}}
    );
    text hello_txt = mktext(
        "Meet the beautiful Reze <3"
    );
    hello_txt.justify = TEXT_CENTER;
    hello_txt.font_size = 28;

    while(win.is_open)
    {
        updwin();

        clrwin();
        rendimg(&reze_hitbox, &reze_img);
        rendtxt(&hello_hitbox, &hello_txt);
        preswin();
    }
}

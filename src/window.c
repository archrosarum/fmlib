#include "optic.h"


window mkwin()
{
	/* Create window structure and copy arguments into it */
	window new_window;

	new_window.is_open = 1;
	new_window.title = "Window";
	new_window.size = (vec_i){640, 480};
	new_window.bg = (rgba){0, 0, 0, 255};

	/* Create a new window internally */
	SDL_Init(SDL_INIT_VIDEO);

	SDL_CreateWindowAndRenderer(
		new_window.title,
		new_window.size.x,
		new_window.size.y,
		SDL_WINDOW_RESIZABLE | SDL_WINDOW_TRANSPARENT,
		&new_window.winptr,
		&new_window.renptr
	);

	SDL_SetRenderDrawBlendMode(new_window.renptr, SDL_BLENDMODE_BLEND);

	/* Return once fully populated */
	return new_window;
}

void updwin(window* win)
{
	/* Iterate internal events and handle them */
	SDL_Event event;
	SDL_WaitEvent(&event);

	switch (event.type)
	{
		/* Window was closed */
		case SDL_EVENT_QUIT:
		{
			win->is_open = 0;
			break;
		}
		/* Window was resized */
		case SDL_EVENT_WINDOW_RESIZED:
		{
			win->size.x = event.window.data1;
    		win->size.y = event.window.data2;
			break;
		}
	}
}

void modwinbg(window* win, rgba bg)
{
	/* Edit the window structure and call internal changes */
	win->bg = bg;
}

void clrwin(window* win)
{
	SDL_SetRenderDrawColor(win->renptr, win->bg.r, win->bg.g, win->bg.b, win->bg.a);
	SDL_RenderClear(win->renptr);
}

void preswin(window* win)
{
	SDL_RenderPresent(win->renptr);
}
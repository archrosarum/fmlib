#include "optic.h"


void rendrect(rect* rec, rgba color)
{
	window* win = rec->win;

	vec_mes pos = rec->pos;
	vec_mes size = rec->size;
	vec_mes anchor = rec->anchor;

	/* Compute geometry using resolved values */
	SDL_FRect rect = 
	{
		/* Resolve position and apply resolved anchor */
		resmes(win->winptr, WIDTH, pos.x) - resmes(win->winptr, WIDTH, size.x) * (anchor.x.pct),
		resmes(win->winptr, HEIGHT, pos.y) - resmes(win->winptr, HEIGHT, size.y) * (anchor.y.pct),

		/* Resolve size */
		resmes(win->winptr, WIDTH, size.x),
		resmes(win->winptr, HEIGHT, size.y),
	};

	SDL_SetRenderDrawColor(win->renptr, color.r, color.g, color.b, color.a);

	SDL_RenderFillRect(win->renptr, &rect);
}
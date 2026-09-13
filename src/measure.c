#include "optic.h"


mes newmes(float pct, int px)
{
	/* Package arguments into a measure vector struct and return */
	mes new_mes;
	new_mes.pct = pct;
	new_mes.px = px;

	return new_mes;
}

int resmes(SDL_Window* win, dimention dim, mes mes)
{
	/* Get the appropriate window pixel dimention */
	int winpx;
	if (dim == WIDTH)
	{
		SDL_GetWindowSize(win, &winpx, NULL);
	}
	else if (dim == HEIGHT)
	{
		SDL_GetWindowSize(win, NULL, &winpx);
	}

	/* Compute and return the resolved measure */
	int res = (winpx * mes.pct) + mes.px;
	return res;
}


vec_mes newvecmes(mes x, mes y)
{
	/* Package arguments into a measure vector struct and return */
	vec_mes new_vec_mes;
	new_vec_mes.x = x;
	new_vec_mes.y = y;

	return new_vec_mes;
}

vec_i resvecmes(SDL_Window* win, vec_mes vec_mes)
{
	/* Get window pixel dimentions */
	int winpx_w, winpx_h;
	SDL_GetWindowSize(win, &winpx_w, &winpx_h);

	/* Compute and return the resolved measure vector */
	vec_i res;
	res.x = (winpx_w * vec_mes.x.pct) + vec_mes.x.px;
	res.y = (winpx_h * vec_mes.y.pct) + vec_mes.y.px;

	return res;
}

rect mkrect(window* win, vec_mes size, vec_mes pos, vec_mes anchor)
{
	rect new_rect =
	{
		pos,
		size,
		anchor,
		0,
		0,
		win
	};

	return new_rect;
}
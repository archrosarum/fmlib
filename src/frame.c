#include "optic.h"

/* Abstraction to compute whether a resolved coordinate is inside a resolved rectangle. */
static int inrect(vec_i pos, vec_i tl, vec_i br)
{
	/* Four inequalities determine if the given coordinate is inside the rectanlge */
	if ((pos.x > tl.x && pos.x < br.x) && (pos.y < br.y && pos.y > tl.y))
		return true;
	else
		return false;
}

int chkrect(rect* rec, event evt)
{
	window* win = rec->win;

	/* Resolve the position and size of the rectangle based on the window */
	vec_i res_size = resvecmes(win->winptr, rec->size);

	vec_i res_pos = resvecmes(win->winptr, rec->pos);
	res_pos.x -= res_size.x * rec->anchor.x.pct;
	res_pos.y -= res_size.y * rec->anchor.y.pct;

	/* Calculate corners of the resolved rectangle */
	vec_i tl = res_pos; // TOP LEFT

	vec_i tr = res_pos; // TOP RIGHT
	tr.x += res_size.x;

	vec_i bl = res_pos; // BOTTOM LEFT
	bl.y += res_size.y;

	vec_i br = res_pos; // BOTTOM RIGHT
	br.x += res_size.x;
	br.y += res_size.y;

	/* Get mouse values from internal calls */
	float mx, my;
	SDL_MouseButtonFlags mflags = SDL_GetMouseState(&mx, &my);
	mx = (int)mx;
	my = (int)my;
	vec_i mpos = {mx, my};

	int is_mdownl = mflags & SDL_BUTTON_MASK(SDL_BUTTON_LEFT);
	int is_mdownr = mflags & SDL_BUTTON_MASK(SDL_BUTTON_RIGHT);
	int is_mdown = is_mdownl || is_mdownr;

	/* Determine if provided event has happened within the computed rectangle */
	int check;

	switch (evt)
	{
		/* Check if the mouse is inside this rectangle */
		case MOUSE_IN:
			if (inrect(mpos, tl, br))
				check = 1;
			else
				check = 0;
			break;
		/* Check if the mouse is outside this rectangle */
		case MOUSE_OUT:
			if (!inrect(mpos, tl, br))
				check = 1;
			else
				check = 0;
			break;
		/* Check if the mouse is down inside this rectangle */
		case MOUSE_DOWN:
			if (inrect(mpos, tl, br) && is_mdown)
				check = 1;
			else
				check = 0;
			break;
		/* Check if the mouse is up inside this rectangle */
		case MOUSE_UP:
			if (inrect(mpos, tl, br) && !is_mdown)
				check = 1;
			else
				check = 0;
			break;
		case M_PRESS:
			if (inrect(mpos, tl, br) && !rec->m_down && is_mdown)
				check = 1;
			else 
				check = 0;
			break;
		case M_RELEASE:
			if (inrect(mpos, tl, br) && rec->m_down && !is_mdown)
				check = 1;
			else 
				check = 0;
			break;
		default:
			check = 0;
	}

	/* Update rectangles states */
	if (evt == M_PRESS || evt == M_RELEASE)
	{
		rec->m_down = is_mdown;
	}

	return check;
}
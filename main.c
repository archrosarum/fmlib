#include "optic.h"
#include <stdio.h>



int main()
{
	

	window win = mkwin();
	modwinbg(&win, (rgba){0, 0, 0, 128});

	rect box = mkrect(
		&win,
		(vec_mes){(mes){0, 128}, (mes){0, 32}},
		(vec_mes){(mes){0, 8}, (mes){0, 8}},
		(vec_mes){(mes){0, 0}, (mes){0, 0}}
	);

	while (win.is_open)
	{
		updwin(&win);

		clrwin(&win);

		/* Button behavior */
		if (chkrect(&box, MOUSE_DOWN)) {
			rendrect(&box, (rgba){200, 200, 200, 255});
		} else {
			rendrect(&box, (rgba){255, 255, 255, 255});
		}
		if (chkrect(&box, M_PRESS)) {
			printf("Press!\n");
			fflush(stdout);
		}

		

		preswin(&win);
	}

	return 0;
}

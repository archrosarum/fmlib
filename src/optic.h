#ifndef OPTIC_H
#define OPTIC_H

#include <SDL3/SDL.h>

/* 2D Vector for integers */
typedef struct
{
	int x;
	int y;
}
vec_i;




/* 2D Vector for floats */
typedef struct {
	float x;
	float y;
} vec2f;


/* RGBA color */
typedef struct {
	unsigned char r;
	unsigned char g;
	unsigned char b;
	unsigned char a;
} rgba;


/* Container for all data related to a specific window. */
typedef struct {
	unsigned char	is_open;	// Tracks if a window is open or has been closed.
	char*			title;		// Visible title of the window.	
	vec_i 			size;		// Dimentions of the window.
	rgba  			bg;			// Color of the window background.

	SDL_Window* 	winptr;		// Internal window pointer.
	SDL_Renderer* 	renptr;		// Internal renderer pointer.
} window;

window 	mkwin();
void 	updwin(window* win);
void 	clrwin(window* win);
void 	preswin(window* win);
void    modwinbg(window* win, rgba bg);


/* frame.c */

/* Measures are computed as the sum of the resolved percentage of
 * an arbitrary frame's width or height and the pixel offset. */
typedef struct 
{
	float 	pct;	// Relative percentage
	int 	px;		// Pixel offset
}
mes;

/* Used as an argument to specify whether a measurement should be
 * resolved based on the width or height of a window. */
typedef enum
{
	WIDTH,
	HEIGHT,
}
dimention;

/* Two dimentional vector for measures. */
typedef struct
{
	mes x;
	mes y;
}
vec_mes;


/* Resolves a measure into pixels based on the provided window and dimention. */
int resmes(SDL_Window* win, dimention dim, mes mes);

/* Factory function to convieniently create a measure. */
mes newmes(float pct, int px);


/* Resolves a measure vector into pixels based on the provided window. */
vec_i resvecmes(SDL_Window* win, vec_mes vec_mes);

/* Factory function to convieniently create a measure vector. */
vec_mes newvecmes(mes x, mes y);


typedef struct {
	vec_mes pos;
	vec_mes size;
	vec_mes anchor;
	int 	m_down;
	int 	m_touch;
	window* win;
} rect;

rect mkrect(window* win, vec_mes size, vec_mes pos, vec_mes anchor);

/* Render a rectangle to a window */
void rendrect(rect* rec, rgba color);




typedef enum {
	MOUSE_IN,
	MOUSE_OUT,
	MOUSE_ENTER,
	MOUSE_LEAVE,
	MOUSE_DOWN,
	MOUSE_UP,
	M_PRESS,
	M_RELEASE,
} event;

/* Check within a rect of a window if an event has happened and return if so. */
int chkrect(rect* rec, event evt);


#endif
#ifndef FMGUI_H
#define FMGUI_H

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


/* Measurements */

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

/* Controls whether vector components resolve independently or one resolved
 * component is copied to the other. */
typedef enum
{
	VEC_MES_INDEPENDENT = 0,
	VEC_MES_FROM_X,
	VEC_MES_FROM_Y,
}
vec_mes_mode;

/* Two dimentional vector for measures. A zero-initialized mode is independent. */
typedef struct
{
	mes x;
	mes y;
	vec_mes_mode mode;
}
vec_mes;


/* Resolves a measure into pixels based on the current window and dimention. */
int resmes(dimention dim, mes mes);

/* Factory function to convieniently create a measure. */
mes newmes(float pct, int px);


/* Resolves a measure vector into pixels based on the current window. */
vec_i resvecmes(vec_mes vec_mes);

/* Factory function to convieniently create a measure vector. */
vec_mes newvecmes(mes x, mes y);

/* Resolve the supplied component against its corresponding dimension, then
 * use that resolved value for both components. */
vec_mes vecmes_from_x(mes x);
vec_mes vecmes_from_y(mes y);


typedef struct rect {
    vec_mes pos;
    vec_mes size;
    vec_mes anchor;
    int m_down;
    int m_touch;
    struct rect* parent;
    bool is_winrect; /* Internal: only the window-owned root sets this. */
} rect;

/* A special root rectangle, embedded in its owning window. */
typedef rect winrect;

/* Container for all data related to a specific window. */
typedef struct {
	unsigned char	is_open;	// Tracks if a window is open or has been closed.
	const char* title;		// Visible title of the window.	
	vec_i 			size;		// Dimentions of the window.
	rgba  			bg;			// Color of the window background.

	SDL_Window* 	winptr;		// Internal window pointer.
	SDL_Renderer* 	renptr;		// Internal renderer pointer.
    bool mouse_down, previous_mouse_down;
    bool mouse_valid, previous_mouse_valid;
    float mouse_x, mouse_y, previous_mouse_x, previous_mouse_y;
    bool owns_sdl;
    winrect root;
} window;

/* Set this after mkwin: current_window = &win. Main-thread use only.
 * Creation does not change selection; destruction clears it. Rectangle-based
 * operations continue to use the window belonging to the rectangle's root. */
extern window* current_window;

window 	mkwin(void);
void destroywin(void);
bool 	updwin(void);
bool 	clrwin(void);
bool 	preswin(void);
void    modwinbg(rgba bg);
/* SDL copies title; window.title borrows SDL-owned storage until changed/destroyed. */
bool modwin(const char* title, vec_i size);


bool resolve_rect(const rect* rec, SDL_FRect* result);
/* Return the window at the root of a valid parent chain, or NULL. */
window* rectwin(const rect* rec);
rect mkrect(rect* parent, vec_mes size, vec_mes pos, vec_mes anchor);

/* Linear position animation. Initialize with mkanim; runtime fields are internal.
 * One anim drives one rectangle at a time. The rectangle must remain alive at
 * the same address until completion or restart. */
typedef struct {
    vec_mes goal;
    double seconds;
    vec_mes start;
    double elapsed;
    rect* target;
} anim;

anim mkanim(vec_mes goal, double seconds);
/* Capture the current position and (re)start. Zero seconds moves immediately.
 * Invalid inputs return false without changing the animation or rectangle. */
bool startanim(anim* animation, rect* rec);
/* Advance by delta seconds; return true while still running, false otherwise.
 * Negative/nonfinite deltas are ignored. Call before hit testing and drawing. */
bool updanim(anim* animation, double delta_seconds);

/* Render a rectangle to a window */
bool rendrect(rect* rec, rgba color);

typedef enum {
    TEXT_LEFT,
    TEXT_RIGHT,
    TEXT_CENTER,
} text_align;

typedef struct {
    const char* content; /* Borrowed, null-terminated UTF-8 string. */
    rgba color;
    float font_size;    /* Point size, greater than zero. */
    const char* font;   /* Borrowed font-file path; NULL selects Times New Roman. */
    text_align justify;
} text;

/* Defaults: black, 16pt Times New Roman, left aligned. */
text mktext(const char* content);
/* Top-aligned text, wrapped to the rectangle width and clipped to its bounds.
 * justify aligns each line left, center, or right inside the rectangle.
 * Returns false on failure (SDL_GetError); empty text/rectangles draw nothing. */
bool rendtxt(const rect* rec, const text* txt);

/* Owns decoded pixels, with no rectangle or renderer association. */
typedef struct {
    SDL_Surface* surface;
} image;

/* Load once; a NULL surface indicates failure (SDL_GetError). Do not copy
 * owning images. The loaded image can be drawn on any rectangle/window. */
image mkimg(const char* path);
void destroyimg(image* img);
/* Stretch the entire image to fill the rectangle, with alpha blending. */
bool rendimg(const rect* rec, const image* img);



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

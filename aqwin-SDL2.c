/* window-sdl.c - implement the Aqwin API using SDL */
#include <GL/gl.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include "aqwin.h"
#include "aqwin-impl.h"

struct Aqwin {
	SDL_Window *sdlwin;
	SDL_GLContext context;
};

Aqwin *aqwin_create(aqwin_param *p) {
	int w,h,bpp,x,y;
	int flags;
	Aqwin *win;
	const GLubyte *version;
	const char *geom, *title;
	
	title = "GL Window";
	x = y = SDL_WINDOWPOS_UNDEFINED;
	w = 640;
	h = 480;
	bpp = 24;
	flags = SDL_WINDOW_OPENGL;
	
	if(aqwin_params(p, "geom", &geom)) {
		aqwin_parsegeom(geom, &w, &h, &x, &y);
	}
	aqwin_params(p, "title", &title);
	aqwin_parami(p, "depth", &bpp);
	if (aqwin_params(p, "resize", NULL)) {
		flags |= SDL_WINDOW_RESIZABLE;
	}
	if (aqwin_params(p, "fullscreen", NULL)) {
		flags |= SDL_WINDOW_FULLSCREEN;
	}
	
	if ((win = malloc(sizeof(*win))) == NULL) {
		goto e0_nomem;
	}
	memset(win, 0, sizeof *win);

	if (!SDL_WasInit(SDL_INIT_VIDEO) && SDL_Init(SDL_INIT_VIDEO)) {
		complain("Init SDL failed: %s\n", SDL_GetError());
		goto e1_sdl_init;
	}
	
	SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 8 );
	SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 8 );
	SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE,  8 );
	SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 24 );
	
	win->sdlwin = SDL_CreateWindow(title, x, y, w, h, flags);
	
	if (win->sdlwin == NULL) {
		complain("Open window failed: %s\n", SDL_GetError());
		goto e2_sdl_win;
	}
	
	win->context = SDL_GL_CreateContext(win->sdlwin);
	if (win->context == NULL) {
		complain("Get GL context failed: %s\n", SDL_GetError());
		goto e3_sdl_cxt;
	}
	if ((version = glGetString(GL_VERSION)) == NULL) {
		complain("Could not determine openGL version\n");
		goto e3_sdl_cxt;
	}
	SDL_GL_SetSwapInterval(1);
	
	if (SDL_GL_MakeCurrent(win->sdlwin, win->context)) {
		complain("Could not make context current: %s\n", SDL_GetError());
		goto e4_sdl_current;
	}
	complain("Initialized %s\n", version);
	
	return win;

e4_sdl_current:
	SDL_GL_DeleteContext(win->context);
e3_sdl_cxt:
	SDL_DestroyWindow(win->sdlwin);
e2_sdl_win:
	SDL_Quit();
e1_sdl_init:
	free(win);	
e0_nomem:
	return NULL;
}
	
void aqwin_swap(Aqwin *win) {
	SDL_GL_SwapWindow(win->sdlwin);
}

void aqwin_close(Aqwin *win) {
	SDL_GL_DeleteContext(win->context);
	SDL_DestroyWindow(win->sdlwin);
	SDL_Quit();
	free(win);
}

void aqwin_geom(Aqwin *win, int *x, int *y, int *w, int *h) {
	if (w && h) SDL_GetWindowSize(win->sdlwin, w,h);
	if (x && y) SDL_GetWindowPosition(win->sdlwin, x,y);
}

const char *aqwin_title(Aqwin *win, const char *title) {
	if (title) {
		SDL_SetWindowTitle(win->sdlwin, title);
	}
	return SDL_GetWindowTitle(win->sdlwin);
}

int aqwin_eventfd(Aqwin *win, const char *filter) {
	return -1;
}

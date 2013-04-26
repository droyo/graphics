#include <stdio.h>
#include <stdlib.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "aqwin.h"

void draw_line(Aqwin *);

int main(int argc, char **argv) {
	char s[10];
	Aqwin *w;
	
	if ((w = aqwin_open("title='Demo window' geom=640x480")) == NULL) {
		exit(1);
	}
	puts("Created window");
	draw_line(w);
	
	fgets(s, sizeof s, stdin);
	aqwin_close(w);

	return 0;
}

void draw_line(Aqwin *w) {
	glClearColor( 0.0, 0.0, 0.0, 0.0 );

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	/* cartesian coordinates */
	gluOrtho2D( 0.0, 640, -(480 / 2), 480 / 2 );
	glMatrixMode(GL_MODELVIEW);

	glDisable(GL_DEPTH_TEST);
	glClear (GL_COLOR_BUFFER_BIT);
	glBegin(GL_LINE_STRIP);
		glVertex2f(0.5f, 0.0f);
		glVertex2f(44.0f, 44.0f);
	glEnd();
	aqwin_swap(w);
}

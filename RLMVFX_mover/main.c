#include <stdio.h>
#include <GLUT/glut.h>
#include "chip8.h"

void renderScene(void) {

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glDrawPixels(windowHeight, windowWidth, GL_RGB, GL_UNSIGNED_BYTE_3_3_2, truegfx);
	
	glutSwapBuffers();
}

int main (int argc, char *argv[]) {

	// init GLUT and create window
	glutInit(&argc, argv);
	glutInitWindowPosition(500, 0);
	glutInitWindowSize(windowHeight*scale, windowWidth*scale);
	glutInitDisplayMode( GLUT_RGBA | GLUT_SINGLE );
	glutCreateWindow("RLMVFX_");

	glutDisplayFunc(renderScene);

	glutTimerFunc(1, calculateFrame, 0);
	glPixelZoom(scale, scale);


	glutMainLoop();

}

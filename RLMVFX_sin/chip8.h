#include <stdio.h>
#include <GLUT/glut.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>

#define windowHeight 256 
#define windowWidth 256 
float scale = 2.0f;
int nCycles = 0;
int mCycles = 0;
int trueCycles = 0;
unsigned char truegfx[windowHeight * windowWidth]; 	/* My real video buffer. Used to turn gfx[] into whatever I want. */

int calculateFrame() {

	int temp;
	float dScale = 1.0f;

	trueCycles++;	
	trueCycles = trueCycles % 360;

	nCycles = (sin( (trueCycles * 3.14159f) / 180) * 180) / 3.14159f;
	mCycles = (cos( (trueCycles * 3.14159f) / 180) * 180) / 3.14159f;

	// Fill it in
	for (int yline = 0; yline < windowHeight; yline++) {
		for (int xline = 0; xline < windowWidth; xline++) {

			// You can add cool filters here!!!
			truegfx[xline + (yline*windowWidth)] = 
				sin(((double)nCycles + xline)/10)*5 + 
				sin(((double)mCycles + yline)/10)*5 +
				cos(((double)nCycles)/30)*4;
		}
	}	

	glutPostRedisplay(); 

	glutTimerFunc(10, calculateFrame, 0);

	return 1;
}

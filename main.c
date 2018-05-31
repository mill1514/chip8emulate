#include <stdio.h>
#include <GLUT/glut.h>
#include "chip8.h"

void renderScene(void) {

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glDrawPixels(64, 32, GL_RGB, GL_UNSIGNED_BYTE_3_3_2, truegfx);
	
	glutSwapBuffers();
}

void keyFuncDown(unsigned char keycode, int x, int y) {

	//printf("REGISTERED KEYPRESS\n");

	switch (keycode) {

		case 'x':
			key[0x0] = 1;
			break;
		case '1':
			key[0x1] = 1;
			break;
		case '2':
			key[0x2] = 1;
			break;
		case '3':
			key[0x3] = 1;
			break;
		case 'q':
			key[0x4] = 1;
			break;
		case 'w':
			key[0x5] = 1;
			break;
		case 'e':
			key[0x6] = 1;
			break;
		case 'a':
			key[0x7] = 1;
			break;
		case 's':
			key[0x8] = 1;
			break;
		case 'd':
			key[0x9] = 1;
			break;
		case 'z':
			key[0xA] = 1;
			break;
		case 'c':
			key[0xB] = 1;
			break;
		case '4':
			key[0xC] = 1;
			break;
		case 'r':
			key[0xD] = 1;
			break;
		case 'f':
			key[0xE] = 1;
			break;
		case 'v':
			key[0xF] = 1;
			break;
		default:
			return;
	}
}

void keyFuncUp(unsigned char keycode, int x, int y) {

	switch (keycode) {

		case 'x':
			key[0] = 0;
			break;
		case '1':
			key[1] = 0;
			break;
		case '2':
			key[2] = 0;
			break;
		case '3':
			key[3] = 0;
			break;
		case 'q':
			key[0x4] = 0;
			break;
		case 'w':
			key[0x5] = 0;
			break;
		case 'e':
			key[0x6] = 0;
			break;
		case 'a':
			key[0x7] = 0;
			break;
		case 's':
			key[0x8] = 0;
			break;
		case 'd':
			key[0x9] = 0;
			break;
		case 'z':
			key[0xA] = 0;
			break;
		case 'c':
			key[0xB] = 0;
			break;
		case '4':
			key[0xC] = 0;
			break;
		case 'r':
			key[0xD] = 0;
			break;
		case 'f':
			key[0xE] = 0;
			break;
		case 'v':
			key[0xF] = 0;
			break;
		default:
			break;
	}
}


int main (int argc, char *argv[]) {

	int scale = 8.0f;
	
	// init GLUT and create window
	glutInit(&argc, argv);
	glutInitWindowPosition(500, 0);
	glutInitWindowSize(64 * (int)scale, 32 * (int)scale);
	glutInitDisplayMode( GLUT_RGBA | GLUT_SINGLE );
	glutCreateWindow("Chip-8 Emulate");
	glutDisplayFunc(renderScene);
	glutKeyboardFunc(keyFuncDown);
	glutKeyboardUpFunc(keyFuncUp);
	glutTimerFunc(17, emulateCycle, 0);
	glPixelZoom(scale, scale);

	// Load up the game, son
	initialize();

	if (argc > 1) {
		loadROM(argv[1]);
	} else {
		printf("Usage: %s <romname>\n", argv[0]);
		exit(0);
	}

	glutMainLoop();

}

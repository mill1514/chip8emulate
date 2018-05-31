chate: main.c chip8.h
	gcc -o chate.out main.c chip8.h -framework GLUT -framework OpenGL -Wno-deprecated-declarations

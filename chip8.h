#include <stdio.h>
#include <GLUT/glut.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>

unsigned short 	opcode;		/* The operation to be performed in this cycle. */
unsigned char 	memory[4096];	/* 4k of ram */
unsigned char 	V[16];		/* 16 Registers. The last one is VF; it is a flag. */
unsigned short 	I;		/* Index register. Used for addresses. */
unsigned short 	pc;		/* Program counter. Points to current opcode. */
unsigned char 	gfx[64*32];	/* Video Buffer. Holds pixels of screen. 0/1. */
unsigned char 	truegfx[64*32]; /* My real video buffer. Used to turn gfx[] into whatever I want. */
unsigned char 	delay_timer;	/* timer register. Decrements at 60hz (once a cycle)*/
unsigned char 	sound_timer;	/* Same as delay timer. */
unsigned short 	stack[16];	/* Used to store return pc */
unsigned short 	sp;		/* points to current place on stack*/
unsigned char 	key[16];	/* Stores input key statuses*/

unsigned char chip8_fontset[80] =
{ 
  0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
  0x20, 0x60, 0x20, 0x20, 0x70, // 1
  0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
  0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
  0x90, 0x90, 0xF0, 0x10, 0x10, // 4
  0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
  0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
  0xF0, 0x10, 0x20, 0x40, 0x40, // 7
  0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
  0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
  0xF0, 0x90, 0xF0, 0x90, 0x90, // A
  0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
  0xF0, 0x80, 0x80, 0x80, 0xF0, // C
  0xE0, 0x90, 0x90, 0x90, 0xE0, // D
  0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
  0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

void initialize() {

	pc	= 0x200; 		// PC starts at 0x200 
	opcode 	= 0;
	I	= 0;
	sp 	= 0;
	
	for (int i = 0; i < 80; i++) 	// Put fontset in the beginning of RAM
		memory[i] = chip8_fontset[i];

	srand(time(NULL)); 		// Seed the random number gen
}

void loadROM(char * name) {

	FILE * fp;
	unsigned char tempROMstorage[4096];

	char fullpath[50] = "./c8games/";
	strcat(fullpath, name);
	fp = fopen(fullpath, "rb");	

	fread( tempROMstorage, sizeof(char), 4096, fp );
	for (int i = 0; i < 4096 - 0x200; i++) {
		memory[i + 0x200] = tempROMstorage[i];
	}

}

int emulateCycle() {

	/* Fetch the opcode	*/
	opcode = memory[pc] << 8 | memory[pc+1];
	
	//printf("PC: 0x%x\n", pc);
	//printf("OPCODE: 0x%x\n\n", opcode);

	/* Increment timers */
	if (delay_timer > 0) 
		delay_timer--;

	if (sound_timer > 0) 
		sound_timer--;

	// Debugging help
	if (pc < 0x200) {
		printf("Program counter has gone wonky, stopping...\n");
		exit(0);
	}

	int temp;
	int x;
	int y;

	switch (opcode & 0xF000) {

		case 0x0000: /* 0x0000			*/

			switch (opcode & 0x00FF) {
			
				case 0x00E0: /* 0x00E0: disp_clear(): clears the screen */
					
					for (int i = 0; i < 2048; i++) {
						gfx[i] = 0;
						truegfx[i] = 0;
					}

					glutPostRedisplay(); 

					pc += 2;
					break;


				case 0x00EE: /* 0x0NNN: return from subroutine	*/
					
					pc = stack[--sp];

					pc += 2;
					break;
				
				default: /* 0x0NNN: Calls RCA 1802 program at address NNN.
						not necessary. */
					printf("0x0NNN is not implemented...\n");
					break;

			}

			break;

		case 0x1000: /* 0x1NNN: Jump to NNN			*/

			pc = opcode & 0x0FFF;
			break;

		case 0x2000: /* 0x2NNN: Call subroutine at NNN			*/

			stack[sp] = pc;
			sp++;
			pc = opcode & 0x0FFF;

			break;

		case 0x3000: /* 0x3XNN: Skips the next instruction if VX == NN */


			temp = (opcode & 0x0F00) >> 8;

			//printf("Checking if V[%d](=%d) == 0x00FF & 0x%x (=%d):", 
			//	temp, V[temp], opcode, (opcode & 0x00FF));

			if ( V[temp] == (opcode & 0x00FF) ) {
			//	printf("TRUE!\n");
				pc += 2;
			}
		
			
			pc += 2;


			break;

		case 0x4000: /* 0x4XNN: Skips the next instruction if VX != NN */

			temp = (opcode & 0x0F00) >> 8;

			if ( V[temp] != (opcode & 0x00FF) ) 
				pc += 2;
			
			pc += 2;
			break;

		case 0x5000:; /* 0x5XY0: Skips the next instruction if VX == VY */

			x = (opcode & 0x0F00) >> 8;
			y = (opcode & 0x00F0) >> 4;

			if ( V[x] == V[y] ) 
				pc += 2;
			
			pc += 2;
			break;

		case 0x6000: /* 0x6XNN: Set Vx = NN			*/

			temp = (opcode & 0x0F00) >> 8;
			V[temp] = opcode & 0x00FF;
			pc += 2;
			break;

		case 0x7000: /* Adds NN to VX. (Carry flag is not changed) */

			temp = (opcode & 0x0F00) >> 8;
			V[temp] += opcode & 0x00FF;
			pc += 2;
			break;

		case 0x8000: /* Adds NN to VX. (Carry flag is not changed) */

			switch (opcode & 0x000F) {
				
				case 0x0000: /* 0x8XY0: Sets Vx = Vy	*/
					
					x = (opcode & 0x0F00) >> 8;
					y = (opcode & 0x00F0) >> 4;
					
					V[x] = V[y];

					pc += 2;
					break;

				case 0x0001: /* 0x8XY1: Sets Vx equal to Vx OR Vy	*/
					
					x = (opcode & 0x0F00) >> 8;
					y = (opcode & 0x00F0) >> 4;
					
					V[x] = V[x] | V[y];

					pc += 2;
					break;

				case 0x0002: /* 0x8XY2: Sets Vx equal to Vx AND Vy	*/
					
					x = (opcode & 0x0F00) >> 8;
					y = (opcode & 0x00F0) >> 4;
					
					V[x] = V[x] & V[y];

					pc += 2;
					break;

				case 0x0003: /* 0x8XY3: Sets Vx equal to Vx XOR Vy	*/
					
					x = (opcode & 0x0F00) >> 8;
					y = (opcode & 0x00F0) >> 4;
					
					V[x] = V[x] ^ V[y];

					pc += 2;
					break;

				case 0x0004: /* 0x8XY4: Adds Vy to Vx. VF is set to 1 if carry. */
					
					x = (opcode & 0x0F00) >> 8;
					y = (opcode & 0x00F0) >> 4;
					
					if (V[x] + V[y] > 255) 
						V[0xF] = 1;
					
					V[x] += V[y];

					pc += 2;
					break;

				case 0x0005: /* 0x8XY5: Subtracts Vy from Vx. VF is set to 0 if borrow (?) */
					
					x = (opcode & 0x0F00) >> 8;
					y = (opcode & 0x00F0) >> 4;
									
					if(V[y] > V[x]) 
						V[0xF] = 0; // there is a borrow
					else 
						V[0xF] = 1;		

					V[x] -= V[y];

					pc += 2;
					break;

				case 0x0006: /* 0x8XY6: Shift Vy right by 1 and store in Vx.
						VF is set to the least significant bit 
						before the shift. 	*/
					
					x = (opcode & 0x0F00) >> 8;
					y = (opcode & 0x00F0) >> 4;
					
					V[0xF] = V[x] & 0x1; // Get least significant bit
					
					V[x] = V[y] >> 1;

					pc += 2;
					break;

				case 0x0007: /* 0x8XY7: Sets Vx equal to Vy - Vx. Set VF to 0 if borrow	*/
					
					x = (opcode & 0x0F00) >> 8;
					y = (opcode & 0x00F0) >> 4;
					
					if(V[x] > V[y]) 
						V[0xF] = 0; // there is a borrow
					else 
						V[0xF] = 1;		

					V[x] = V[y] - V[x];

					pc += 2;
					break;

				case 0x000E: /* 0x8XYE: Shifts Vy left by one and copies the result to Vx.
						VF is set to the most significant bit of Vy before shift. */
					
					x = (opcode & 0x0F00) >> 8;
					y = (opcode & 0x00F0) >> 4;
				
					V[0xF] = V[y] & 0x8;
					
					V[y] = V[y] << 1;	
					V[x] = V[y];

					pc += 2;
					break;

				default:
					printf("Unrecognized command! [0x%x]\n", opcode);
					exit(0);
			}

			break;

		case 0x9000: /* 0x9XY0: Skips the next instruction if VX != VY */
			
			temp = V[ (opcode & 0x0F00) >> 8 ];
			if (temp != V[ (opcode & 0x00F0) >> 4] ) {
				pc += 2;
			}

			pc += 2;
			break;

		case 0xA000: /* 0xANNN: Set I to the address NNN	*/
		
			I = opcode & 0x0FFF;
			pc += 2;
			break;

		case 0xB000: /* 0xBNNN: Jump to address NNN plus V0 	*/

			pc = (opcode & 0x0FFF) + V[0];
			break;

		case 0xC000: /* 0xCXNN: Sets Vx to the result of a 
				bitwise AND operation on a random number 
				(0-255) and NN. 	*/
			
			temp = (opcode & 0x0F00) >> 8; // temp = X;
			V[temp] = (rand() % 255) & (opcode & 0x00FF);

			pc += 2;
			break;

		case 0xD000:; /* 0xDXYN: Disp: Draws a sprite at coordinate
				(VX, VY) with width of 8 px and height of
				N px. Each row of 8 px is read as bit coded
				starting from memory location I;  I value 
				does not change after execution. VF is set
				to 1 if any screen pixels are flipped from
				set to unset when the sprite is drawn.	*/


			x = V[ (opcode & 0x0F00) >> 8 ];
			int y = V[ (opcode & 0x00F0) >> 4 ];
			int height = opcode & 0x000F;

			//printf("X,Y,I,h: %d,%d,0x%x,%d\n", x, y, I, height);

			V[0xF] = 0;
			for (int yline = 0; yline < height; yline++) {

				int byte = memory[I+yline];
				for (int xline = 0; xline < 8; xline++) {

					if ((byte & (0x80 >> xline)) != 0) {

						int gfxIndex = ((x+xline) + (64 * (31-(y+yline)))) % (32 * 64);
					
						int memVal = ( (1<<(7-xline)) & byte ) >> (7-xline);

						if ( gfx[gfxIndex] == 1 ) {
							V[0xF] = 1;
						}
					
						gfx[gfxIndex] ^= 1;

						// You can add cool filters here!!!
						truegfx[gfxIndex] = gfx[gfxIndex] * 0xFF;
							//5 * sqrt(pow(31-y-yline, 4) - pow(x+xline, 4)); 
						
		
					}
				}
			}	
	
	
			/* Since this is the only function (other than 
			clear screen) that actually changes gfx[], 
			just redisplay the screen after this function 
			runs.	*/
			glutPostRedisplay(); 

			pc += 2;
			break;

		case 0xE000: /* 0xE000	*/

			switch (opcode & 0x00FF) { 
				
				case 0x009E: /* 	-- 0xEX9E --
						Skips next instruction if key stored 
						in VX is pressed.	*/

					temp = (opcode & 0x0F00) >> 8;
					
					if (key[V[temp]] == 1) {
						pc += 2;
					}

					pc += 2;
					break;
						
				case 0x00A1: /* 	-- 0xEXA1 --
						Skips next instruction if key stored 
						in VX is pressed.	*/

					temp = (opcode & 0x0F00) >> 8;

					if (key[V[temp]] != 1) {
						pc += 2;
					}

					pc += 2;
					break;

				default:
					printf("Unrecognized command! [0x%x]\n", opcode);
					exit(0);

			}

			break;

		case 0xF000:

			switch (opcode & 0x00FF) { /* 0xFXXX */

				case 0x0007: /* 0xFX07
						Sets VX to value of
						delay timer. */
							
					temp = (opcode & 0x0F00) >> 8;
					V[temp] = delay_timer;

					pc += 2;
					break;
						
				case 0x000A: /* 0xFX0A
						A key press is awaited,
						and then stored in VX */
					
					temp = (opcode & 0x0F00) >> 8;
					
					for (int i = 0; i < 16; i++) {

						if (key[i] != 0) {

							//printf("Keypress found!\n");
							V[temp] = i;
							pc += 2;

							break;
						}
					
					}
			
					//printf("NO keypress found...\n");
					
					break; // If no key was found, continue without 
						// incrementing pc to try again 

				case 0x0015: /* 	--0xFX15--
						Sets the delay timer to VX */
					
					temp = (opcode & 0x0F00) >> 8;

					delay_timer = V[temp];
					
					pc += 2;
					break;

				case 0x0018: /* 	--0xFX18--
						Sets the sound timer to VX */
					
					temp = (opcode & 0x0F00) >> 8;

					sound_timer = V[temp];
					
					pc += 2;
					break;
						
				case 0x001E:; /* 	--0xFX1E--
						Adds VX to I. 	*/
					
					temp = (opcode & 0x0F00) >> 8;

					I += V[temp];
						
					pc += 2;
					break;
						
				case 0x0029:; /* 	--0xFX29--
						Sets I to the location of the sprite 
						for the character in VX.
						*/

					temp = (opcode & 0x0F00) >> 8; // Temp = X 
					
					I = V[temp] * 5;						
					
					pc += 2;
					break;

				case 0x0033:; /* 	--0xFR33--
						bcd vr: store the bcd representation
						of register vr at location I,I+1,I+2
						*/
					//printf("BREAK\n");
					//getchar();
					
					temp = (opcode & 0x0F00) >> 8; // Temp = R
					temp = V[temp]; //Temp = V[R]
					
					int hundreds = 0;
					int tens = 0;
					int ones = 0;

					while (temp >= 100) {
						temp -= 100;
						hundreds++;
					}

					while (temp >= 10) {
						temp -= 10;
						tens++;
					}
	
					while (temp >= 1) {
						temp -= 1;
						ones++;
					}

					memory[I] = hundreds;
					memory[I+1] = tens;
					memory[I+2] = ones;

					pc += 2;
					break;
			
				case 0x0055:; /* 	--0xFX55--
						reg_dump: stores V0 to VX in 
						memory starting at address I. 
						I is increased by 1 for each 
						value written.	*/
					
					temp = (opcode & 0x0F00) >> 8;

					for (int i = 0; i <= temp; i++) 
						memory[i+I] = V[i];

					pc += 2;
					break;
					

				case 0x0065:; /* 	--0xFX65--
						reg_load: fills V0 to VX with
						values from memory starting at 
						address I. I is increased by 1
						for each value written.	*/

					temp = (opcode & 0x0F00) >> 8;

					for (int i = 0; i <= temp; i++)
						V[i] = memory[i+I];

					pc += 2;
					break;

					
				default:
					printf("Unrecognized command! [0x%x]\n", opcode);
					exit(0);

			}

			break;

		default:
			printf("Unrecognized command! [0x%x]\n", opcode);
			exit(0);
	}

	glutTimerFunc(5, emulateCycle, 0);

	return 1;
}

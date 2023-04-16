
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <stdio.h>

#define LEDS (volatile uint32_t *) 30016
volatile uint32_t* display =(volatile uint32_t*)30032;
#define SCREEN_SIZE 2048

uint8_t ram[4096]; //ram for chip8
uint16_t rom_load_addres = 0x200; //chip-8 program start from that adress
uint16_t v_registers[16]; // v registers
uint16_t i_register = 0; // memory register,stores memory adress,last 12 bit used
uint16_t delay_register = 0;
uint16_t sound_register = 0; // When these registers are non-zero, they are automatically decremented at a rate of 60Hz
uint16_t pc_register = 0; // stores currently executing adress
uint16_t stack[16]; /* used to store the address that the interpreter shoud return to when finished with a subroutine.
 Chip-8 allows for up to 16 levels of nested subroutines.(16 nested returns)*/
uint16_t *stack_pointer = stack; //store index of stack
bool draw_flag = false;
uint8_t keyboard[16];
//uint8_t display[2048];
uint16_t opcode = 0;
uint8_t rom_index = 0;

uint8_t fonts[] = { 0xF0, 0x90, 0x90, 0x90, 0xF0, 0x20, 0x60, 0x20, 0x20, 0x70,
		0xF0, 0x10, 0xF0, 0x80, 0xF0, 0xF0, 0x10, 0xF0, 0x10, 0xF0, 0x90, 0x90,
		0xF0, 0x10, 0x10, 0xF0, 0x80, 0xF0, 0x10, 0xF0, 0xF0, 0x80, 0xF0, 0x90,
		0xF0, 0xF0, 0x10, 0x20, 0x40, 0x40, 0xF0, 0x90, 0xF0, 0x90, 0xF0, 0xF0,
		0x90, 0xF0, 0x10, 0xF0, 0xF0, 0x90, 0xF0, 0x90, 0x90, 0xE0, 0x90, 0xE0,
		0x90, 0xE0, 0xF0, 0x80, 0x80, 0x80, 0xF0, 0xE0, 0x90, 0x90, 0x90, 0xE0,
		0xF0, 0x80, 0xF0, 0x80, 0xF0, 0xF0, 0x80, 0xF0, 0x80, 0x80 };

//chip8 picture
uint8_t intro[] = { 106, 2, 107, 12, 108, 63, 109, 12, 162, 234, 218, 182, 220,
		214, 110, 0, 34, 212, 102, 3, 104, 2, 96, 96, 240, 21, 240, 7, 48, 0,
		18, 26, 199, 23, 119, 8, 105, 255, 162, 240, 214, 113, 162, 234, 218,
		182, 220, 214, 96, 1, 224, 161, 123, 254, 96, 4, 224, 161, 123, 2, 96,
		31, 139, 2, 218, 182, 141, 112, 192, 10, 125, 254, 64, 0, 125, 2, 96, 0,
		96, 31, 141, 2, 220, 214, 162, 240, 214, 113, 134, 132, 135, 148, 96,
		63, 134, 2, 97, 31, 135, 18, 70, 2, 18, 120, 70, 63, 18, 130, 71, 31,
		105, 255, 71, 0, 105, 1, 214, 113, 18, 42, 104, 2, 99, 1, 128, 112, 128,
		181, 18, 138, 104, 254, 99, 10, 128, 112, 128, 213, 63, 1, 18, 162, 97,
		2, 128, 21, 63, 1, 18, 186, 128, 21, 63, 1, 18, 200, 128, 21, 63, 1, 18,
		194, 96, 32, 240, 24, 34, 212, 142, 52, 34, 212, 102, 62, 51, 1, 102, 3,
		104, 254, 51, 1, 104, 2, 18, 22, 121, 255, 73, 254, 105, 255, 18, 200,
		121, 1, 73, 2, 105, 1, 96, 4, 240, 24, 118, 1, 70, 64, 118, 254, 18,
		108, 162, 242, 254, 51, 242, 101, 241, 41, 100, 20, 101, 0, 212, 85,
		116, 21, 242, 41, 212, 85, 0, 238, 128, 128, 128, 128, 128, 128, 128, 0,
		0, 0, 0, 0 };



uint8_t* roms[] = { intro};
int sizes[] = { sizeof(intro)};
bool active = true;
/*chip-8 keyboard layout
 # 1, 2, 3, 0xC,
 # 4, 5, 6, 0xD,
 # 7, 8, 9, 0xE,
 # 0xA, 0, 0xB, 0xF
 */



void clear_screen()
{
	for(int i=0;i<2048;i++)
		*(display+i)=0;
}
uint32_t lfsr = 0b11010011; //for pseudo random number
uint32_t rand_number()
{

	uint32_t bit = (lfsr>>0 ^ lfsr >>5 ^ lfsr >> 3)&1;
    lfsr = (lfsr >> 1) | (bit << 7);
    return lfsr;
}


void init_fonts() {
	for (int i = 0; i < 0x50; i++) {
		ram[i] = fonts[i];
	}
}

void load_rom(uint8_t rom[], int size) {

	for (int i = 0; i < size; i++) {
		ram[i + rom_load_addres] = rom[i];

	}

}

void del_sound_timer() {
	if (delay_register) {
		delay_register -= 1;
	}

}
void _00() {

// 00E0 - CLS Clear the display.
	clear_screen();
	draw_flag = true;
}

void _0E() {
	/*``
	 00EE - RET
	 Return from a subroutine.
	 The interpreter sets the program counter to the address at the top of the stack, then subtracts 1 from the stack pointer.'''
	 */
	pc_register = *(stack_pointer);
	stack_pointer--;
}

void _1() {
	/* 1nnn - JP addr
	 jump to location nnn.
	 The interpreter sets the program counter to nnn.
	 */

	pc_register = opcode & 0x0FFF;

}

void _2() {
	/*2nnn - CALL addr
	 Call subroutine at nnn.
	 The interpreter increments the stack pointer, then puts the current PC on the top of the stack. The PC is then set to nnn.
	 */

//    self.stack.append(self.pc_register)
	stack_pointer++;
	*(stack_pointer) = pc_register;
	pc_register = opcode & 0x0FFF;

}

void _3() {
	/*3xkk - SE Vx, byte
	 Skip next instruction if Vx = kk.
	 The interpreter compares register Vx to kk, and if they are equal, increments the program counter by 2.
	 */

	uint8_t x = (opcode & 0x0F00) >> 8;
	uint8_t kk = opcode & 0x00FF;

	if (v_registers[x] == kk)
		pc_register += 2;

}

void _4() {

	/*4xkk - SNE Vx, byte
	 Skip next instruction if Vx != kk.
	 The interpreter compares register Vx to kk, and if they are not equal, increments the program counter by 2.*/

	uint8_t x = (opcode & 0x0F00) >> 8;
	uint8_t kk = opcode & 0x00FF;
	if (v_registers[x] != kk)
		pc_register += 2;
}

void _5() {
	/*5xy0 - SE Vx, Vy
	 Skip next instruction if Vx = Vy.
	 The interpreter compares register Vx to register Vy, and if they are equal, increments the program counter by 2.
	 */
	uint8_t x = (opcode & 0x0F00) >> 8;
	uint8_t y = (opcode & 0x00F0) >> 4;
	if (v_registers[x] == v_registers[y])
		pc_register += 2;

}

void _6() {
	/*6xkk - LD Vx, byte
	 Set Vx = kk.
	 The interpreter puts the value kk into register Vx.*/

	uint8_t x = (opcode & 0x0F00) >> 8;
	uint8_t kk = opcode & 0x00FF;
	v_registers[x] = kk;
	v_registers[x] &= 0xff;
}

void _7() {
	/*
	 7xkk - ADD Vx, byte
	 Set Vx = Vx + kk.
	 Adds the value kk to the value of register Vx, then stores the result in Vx.
	 */
	uint8_t x = (opcode & 0x0F00) >> 8;
	uint8_t kk = opcode & 0x00FF;
	v_registers[x] = v_registers[x] + kk;
	v_registers[x] &= 0xff;

}

void _80() {
	/*8xy0 - LD Vx, Vy
	 Set Vx = Vy.
	 Stores the value of register Vy in register Vx.*/

	uint8_t x = (opcode & 0x0F00) >> 8;
	uint8_t y = (opcode & 0x00F0) >> 4;
	v_registers[x] = v_registers[y];
	v_registers[x] &= 0xff;

}

void _81() {
	/*8xy1 - OR Vx, Vy
	 Set Vx = Vx OR Vy.
	 Performs a bitwise OR on the values of Vx and Vy, then stores the result in Vx.
	 A bitwise OR compares the corrseponding bits from two values, and if either bit is 1, then the same bit in the result is also 1. Otherwise, it is 0.
	 */

	uint8_t x = (opcode & 0x0F00) >> 8;
	uint8_t y = (opcode & 0x00F0) >> 4;
	v_registers[x] = (v_registers[x] | v_registers[y]);
	v_registers[x] &= 0xff;

}

void _82() {
	/*8xy2 - AND Vx, Vy
	 Set Vx = Vx AND Vy.
	 Performs a bitwise AND on the values of Vx and Vy, then stores the result in Vx.
	 A bitwise AND compares the corrseponding bits from two values, and if both bits are 1, then the same bit in the result is also 1. Otherwise, it is 0.
	 */

	uint8_t x = (opcode & 0x0F00) >> 8;
	uint8_t y = (opcode & 0x00F0) >> 4;
	v_registers[x] = (v_registers[x] & v_registers[y]);
	v_registers[x] &= 0xff;
}

void _83() {
	/*8xy3 - XOR Vx, Vy
	 Set Vx = Vx XOR Vy.
	 Performs a bitwise exclusive OR on the values of Vx and Vy, then stores the result in Vx.
	 An exclusive OR compares the corrseponding bits from two values, and if the bits are not both the same,
	 then the corresponding bit in the result is set to 1. Otherwise, it is 0.
	 */

	uint8_t x = (opcode & 0x0F00) >> 8;
	uint8_t y = (opcode & 0x00F0) >> 4;
	v_registers[x] = (v_registers[x] ^ v_registers[y]);
	v_registers[x] &= 0xff;

}

void _84() {
	/*8xy4 - ADD Vx, Vy
	 Set Vx = Vx + Vy, set VF = carry.
	 The values of Vx and Vy are added together. If the result is greater than 8 bits (i.e., > 255,)
	 VF is set to 1, otherwise 0. Only the lowest 8 bits of the result are kept, and stored in Vx.
	 */

	uint8_t x = (opcode & 0x0F00) >> 8;
	uint8_t y = (opcode & 0x00F0) >> 4;
	if (v_registers[x] + v_registers[y] > 255) {
		v_registers[15] = 1;
	} else {
		v_registers[15] = 0;
	}

	v_registers[x] = (v_registers[x] + v_registers[y]);
	v_registers[x] &= 0xff;
}

void _85() {

	/*8xy5 - SUB Vx, Vy
	 Set Vx = Vx - Vy, set VF = NOT borrow.
	 If Vx > Vy, then VF is set to 1, otherwise 0. Then Vy is subtracted from Vx, and the results stored in Vx.
	 */

	uint8_t x = (opcode & 0x0F00) >> 8;
	uint8_t y = (opcode & 0x00F0) >> 4;

	if (v_registers[x] > v_registers[y]) {
		v_registers[15] = 1;
	} else {
		v_registers[15] = 0;
	}

	v_registers[x] = (v_registers[x] - v_registers[y]);
	v_registers[x] &= 0xff;
}

void _86() {
	/*8xy6 - SHR Vx {, Vy} Set Vx = Vx SHR 1.
	 If the least-significant bit of Vx is 1, then VF is set to 1, otherwise 0. Then Vx is divided by 2.
	 */

	uint8_t x = (opcode & 0x0F00) >> 8;
  uint8_t y = (opcode & 0x00F0) >> 4;
	v_registers[15] = v_registers[x] & 1;

	v_registers[x] = v_registers[y] >> 1;
	v_registers[x] &= 0xff;

}

void _87() {
	/*8xy7 - SUBN Vx, Vy
	 Set Vx = Vy - Vx, set VF = NOT borrow.
	 If Vy > Vx, then VF is set to 1, otherwise 0. Then Vx is subtracted from Vy, and the results stored in Vx.
	 */

	uint8_t x = (opcode & 0x0F00) >> 8;
	uint8_t y = (opcode & 0x00F0) >> 4;

	if (v_registers[y] > v_registers[x]) {
		v_registers[15] = 1;
	} else {
		v_registers[15] = 0;
	}

	v_registers[x] = v_registers[y] - v_registers[x];
	v_registers[x] &= 0xff;

}

void _8E() {
	/*8xyE - SHL Vx {, Vy}
	 Set Vx = Vx SHL 1.
	 If the most-significant bit of Vx is 1, then VF is set to 1, otherwise to 0. Then Vx is multiplied by 2.
	 */

	uint8_t x = (opcode & 0x0F00) >> 8;
	uint8_t y = (opcode & 0x00F0) >> 4;

	v_registers[15] = v_registers[x] >> 7;

	v_registers[x] = (v_registers[y] << 1);
	v_registers[x] &= 0xff;

}

void _9() {
	/*9xy0 - SNE Vx, Vy
	 Skip next instruction if Vx != Vy.
	 The values of Vx and Vy are compared, and if they are not equal, the program counter is increased by 2.*/

	uint8_t x = (opcode & 0x0F00) >> 8;
	uint8_t y = (opcode & 0x00F0) >> 4;

	if (x != y) {
		pc_register += 2;
	}

}

void _A() {
	/*Annn - LD I, addr
	 Set I = nnn.
	 The value of register I is set to nnn.*/

	uint16_t nnn = opcode & 0x0FFF;
	i_register = nnn;

}

void _B() {
	/*Bnnn - JP V0, addr
	 Jump to location nnn + V0.
	 The program counter is set to nnn plus the value of V0.*/

	uint16_t nnn = opcode & 0x0FFF;
	pc_register = nnn + v_registers[0];
}

void _C() {
	/*Cxkk - RND Vx, byte
	 Set Vx = random byte AND kk.
	 The interpreter generates a random number from 0 to 255,
	 which is then ANDed with the value kk.
	 The results are stored in Vx. See instruction 8xy2 for more information on AND.*/

	uint8_t x = (opcode & 0x0F00) >> 8;
	uint8_t kk = opcode & 0x00FF;
	uint16_t randn = rand_number();

	v_registers[x] = kk & randn;
	v_registers[x] &= 0xff;
}

void _D() {
	/*Dxyn - DRW Vx, Vy, nibble
	 Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision.

	 The interpreter reads n bytes from memory, starting at the address stored in I.
	 These bytes are then displayed as sprites on screen at coordinates (Vx, Vy).
	 Sprites are XORed onto the existing screen.
	 If this causes any pixels to be erased, VF is set to 1, otherwise it is set to 0.
	 If the sprite is positioned so part of it is outside the coordinates of the display, it wraps around to the opposite side of the screen.
	 See instruction 8xy3 for more information on XOR, and section 2.4, Display, for more information on the Chip-8 screen and sprites.'''*/
	// for(int i=0;i<15;i++){
	//     	*LEDS=i;
	// 			for(int j=0;j<19000000;j++){}
	// 		}

	
	int vx = v_registers[(opcode & 0x0F00) >> 8];
	int vy = v_registers[(opcode & 0x00F0) >> 4];
	int n = opcode & 0x000F;
	v_registers[0xF] = 0;
	for (int y = 0; y < n; y++) {
		uint16_t pixel = ram[i_register + y];
		for (uint16_t x = 0; x < 8; x++) {
			if ((pixel & (0x80 >> x)) && x + vx + ((y + vy) * 64) < 2048) {
				


				if(*(display+x + vx + ((y + vy) * 64)))
				{

					v_registers[0xF] = 1;
				}
			
				*(display+x + vx + ((y + vy) * 64)) ^= 1;

			}

		}

	}

	draw_flag = true;

}

void _EE() {
	/*'''

	 Ex9E - SKP Vx
	 Skip next instruction if key with the value of Vx is pressed.
	 Checks the keyboard, and if the key corresponding to the value of Vx is
	 currently in the down position, PC is increased by 2.'''*/

	uint8_t x = (opcode & 0x0F00) >> 8;

	if (keyboard[v_registers[x] & 0xf] == 1) {
		pc_register += 2;
	}

}

void _E1() {
	/*'''ExA1 - SKNP Vx
	 Skip next instruction if key with the value of Vx is not pressed.
	 Checks the keyboard, and
	 if the key corresponding to the value of Vx is currently in the up position,
	 PC is increased by 2.
	 '''*/

	uint8_t x = (opcode & 0x0F00) >> 8;

	if (keyboard[v_registers[x] & 0xf] == 0) {

		pc_register += 2;
	}

}

void _F07() {
	/*'''
	 Fx07 - LD Vx, DT
	 Set Vx = delay timer value.
	 The value of DT is placed into Vx'''*/

	uint8_t x = (opcode & 0x0F00) >> 8;

	v_registers[x] = delay_register;
}

//not need

void _F0A() {

	//not implemented
	/*
	 '''Fx0A - LD Vx, K
	 Wait for a key press, store the value of the key in Vx.
	 All execution stops until a key is pressed, then the value of that key is stored in Vx.'''

	 x = (self.opcode & 0x0F00) >> 8
	 cur_state=self.keyboard
	 while True:
	 for i in self.keyboard:
	 if i!=0 and not i in cur_state:
	 self.v_registers[x]=i
	 break
	 */

}

void _F15() {
	/*'''
	 Set delay timer = Vx.

	 DT is set equal to the value of Vx.
	 '''
	 */

	uint8_t x = (opcode & 0x0F00) >> 8;
	delay_register = v_registers[x];

}

void _F18() {

	/*'''
	 Set sound timer = Vx.

	 ST is set equal to the value of Vx.

	 '''*/

	uint8_t x = (opcode & 0x0F00) >> 8;
	sound_register = v_registers[x];
}

void _F1E() {
	/*'''
	 Set I = I + Vx.

	 The values of I and Vx are added, and the results are stored in I.'''*/

	uint8_t x = (opcode & 0x0F00) >> 8;
	i_register = i_register + v_registers[x];

}

void _F29() {

	/*'''

	 Set I = location of sprite for digit Vx.

	 The value of I is set to the location for the hexadecimal sprite corresponding to the value of Vx.
	 See section 2.4, Display, for more information on the Chip-8 hexadecimal font.

	 '''*/
	uint8_t x = (opcode & 0x0F00) >> 8;

	i_register = v_registers[x] * 5;

}

void _F33() {

	/*'''Store BCD representation of Vx in memory locations I, I+1, and I+2.

	 The interpreter takes the decimal value of Vx, and places the hundreds digit in memory at location in I
	 , the tens digit at location I+1, and the ones digit at location I+2.
	 '''*/

	uint32_t x = v_registers[(opcode & 0x0F00) >> 8];


	uint32_t hundred = ((((x * 0x47AE) >> 16) + x) >> 1) >> 6; //div by 100
	uint32_t ten = (((x-(hundred*100)) * 0xCD) >> 8) >> 3;// div by 10;
	uint32_t one = x-(hundred*100) - (ten * 10);
	ram[i_register] = hundred;
	ram[i_register + 1] = ten;
	ram[i_register + 2] = one;

}

void _F55() {

	/*'''
	 Store registers V0 through Vx in memory starting at location I.

	 The interpreter copies the values of registers V0 through Vx into memory, starting at the address in I.

	 '''*/

	uint8_t x = (opcode & 0x0F00) >> 8;
	for (int i = 0; i <= x; i++) {
		ram[i_register + i] = v_registers[i];
	}

}

void _F65() {

	/* '''

	 Read registers V0 through Vx from memory starting at location I.

	 The interpreter reads values from memory starting at location I into registers V0 through Vx.
	 '''*/

	uint8_t x = (opcode & 0x0F00) >> 8;
	for (int i = 0; i <= x; i++) {
		v_registers[i] = ram[i_register + i];
	}

}

struct {
	uint32_t name;
	void (*func)(void);
} funcs[] = { { 0x00, _00 }, { 0x0E, _0E }, { 0x1, _1 }, { 0X2, _2 },
		{ 0x3, _3 }, { 0x4, _4 }, { 0x5, _5 }, { 0x6, _6 }, { 0x7, _7 }, { 0x80,
				_80 }, { 0x81, _81 }, { 0x82, _82 }, { 0x83, _83 },
		{ 0x84, _84 }, { 0x85, _85 }, { 0x86, _86 }, { 0x87, _87 },
		{ 0x8E, _8E }, { 0x9, _9 }, {0xA, _A }, { 0xB, _B }, { 0xC, _C }, {
				0xD, _D }, { 0xEE, _EE }, { 0xE1, _E1 }, { 0xF07, _F07 }, {
				0xF0A, _F0A }, { 0xF15, _F15 }, { 0x18, _F18 },
		{ 0xF1E, _F1E }, { 0xF29, _F29 }, { 0xF33, _F33 }, { 0xF55, _F55 }, {
				0xF65, _F65 }

};

int call_function(uint32_t name) {
	int i;

	
	for (i = 0; i < 34; i++) {
		if (funcs[i].name == name) {

			 
			funcs[i].func();
			return 0;
		}
	}

	return -1;
}

void execop(uint32_t opcode) {


	uint32_t f = (opcode & 0xF000) >> 12;
	if (f == 8 || f == 0 || f == 14) {

		uint32_t l = opcode & 0x000F;
		call_function( f<<4|l);

	} else if (f == 15) {

		uint32_t l = opcode & 0x000F;
		uint32_t ll = (opcode & 0x00F0) >> 4;
		call_function(((f<<4 |ll)<<4)|l);

	} else {

		call_function(f);

	}

}

void main() {
	init_fonts();
	load_rom(roms[rom_index], sizes[rom_index]);
	pc_register = rom_load_addres;

	while (true) {
		if (active) {
			del_sound_timer();
			opcode = ram[pc_register] << 8 | ram[pc_register + 1];
			pc_register = (pc_register + 2);
			execop(opcode);
			for(uint32_t i =0;i<1000;i++){}
 
			
			
		} 
		// else {
		// 	bzero(ram, sizeof(ram));
		// 	bzero(v_registers, sizeof(v_registers));
		// 	bzero(stack, sizeof(stack));
		// 	bzero(keyboard, sizeof(keyboard));
		// 	bzero(display, sizeof(display));
		// 	*stack_pointer = stack;
		// 	i_register = 0;
		// 	delay_register = 0;
		// 	sound_register = 0;
		// 	pc_register = 0;
		// 	opcode = 0;
		// 	pc_register = rom_load_addres;

		// 	init_fonts();

		// 	bool draw_flag = false;

		// 	load_rom(roms[rom_index], sizes[rom_index]);
			
		// 	active = true;

		// }

	}

}


#ifndef chip_8
#define chip_8

#include <stdint.h>
#include <SDL.h>

/* Specifications for our Chip-8 emu/interpreter */

// 4kb of memory 
uint8_t memory[4096]; 
//Chip-8 display is 64 by 32 px
uint8_t display[64 * 32]; 
// Opcode/instruction, fetched from rom and gives info on what to do
uint16_t opcode; 
// Program counter, tells us where to fetch the next instruction
uint16_t PC; 
// Index register, tracks position in memory
uint16_t I; 
// Stack for 16b addresses, used to call functions and get results back
uint16_t stack[16];
// Stack pointer, keeps track of stack
uint16_t sp;
// 16 8b general purpose var registers, tracked through hexadec (V0-VF), with VF being a flag register
uint8_t v[16];
// Two 8b timers, decremented at a rate to determine fps/sound until they reach 0
uint8_t delayTimer;
uint8_t soundTimer;

// Track if a drawing operation happens
uint_fast8_t drawflag;


// Nibbles for our instructions
uint8_t X, Y, n, nn;
uint16_t nnn;

// 16 hexadecimal inputs from our keyboard
uint8_t keyboard[16];	

// Basic universal font, each num being 4x5 with a binary num representing a pixel being b/w
uint8_t chip8Font[80] =
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

const static uint8_t keyMap[16] = {
    SDLK_x, // 0
    SDLK_1, // 1
    SDLK_2, // 2
    SDLK_3, // 3
    SDLK_q, // 4
    SDLK_w, // 5
    SDLK_e, // 6
    SDLK_a, // 7
    SDLK_s, // 8
    SDLK_d, // 9
    SDLK_z, // A
    SDLK_c, // B
    SDLK_4, // C
    SDLK_r, // D
    SDLK_f, // E
    SDLK_v  // F
};

#endif
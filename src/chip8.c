#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>
#include <stdbool.h>
#include <SDL.h>
#include "chip8.h"

// SDL handlers, keep global to modify across different functions
SDL_Window * window;
SDL_Renderer * renderer;
SDL_Texture * screen;


void initEmu(){
    // Set all init vals to zero, copy font into memory, and adjust PC accordingly
    opcode = 0;
    PC = 0x200;
    I = 0;
    sp = 0;
    delayTimer = 0;
    soundTimer = 0;
    memset(stack, 0, 16);
    memset(v, 0, 16);
    memset(display, 0, 2048);
    memset(memory, 0, 4096);
    memcpy(memory, chip8Font, 80 * sizeof(int8_t));
}

void loadRom(char * fileName){
    // Read in rom as binary
    FILE * rom = fopen(fileName, "rb");

    // Get file size
    fseek(rom, 0, SEEK_END);
    int fileSize = ftell(rom);
    fseek(rom, 0, SEEK_SET);

    //Load rom into mem, saving space for fonts
    fread(memory + 0x200, sizeof(uint16_t), fileSize, rom);

    // Close out file to avoid mem leaks
    fclose(rom);
    printf("loaded");
    printf("%d", fileSize);
}

// 00E0, simply reset all display bits to 0
void clearScreen(){
    drawflag = true;
    memset(display, 0, 2048);
}

// 00EE, returning from a subroutine by popping and saving it
void returnSubroutine(){
    sp--;
    PC = stack[sp];
}

// 1NNN, set our program counter to our 12b immediate address (nnn)
void jump(){
    PC = nnn;
}

// 2NNN, calls subroutine at nnn making sure to push current pc to stack
void callSubroutine(){
    stack[sp] = PC;
    sp++;
    PC = nnn;
}

// 3XNN, skip an instruction (increment pc by 2) if vX = nn
void skipVxNnEq(){
    if (v[X] == nn) PC += 2;
}

// 4XNN, skip an instruction (increment pc by 2) if vX =/= nn
void skipVxNnNeq(){
    if (v[X] != nn) PC += 2;
}

// 5XY0,  skip an instruction (increment pc by 2) if vX = vY
void skipVxVyEq(){
    if (v[X] == v[Y]) PC += 2;
}


// 6XNN, set our current pointed to register to our 8b immediate num (nn)
void setReg(){
    v[X] = nn;
}

// 7XNN, add our 8b immediate num (nn) to our current pointed to register
void addToReg(){
    v[X] += nn;
}

// 9XY0, skip an instruction (increment pc by 2) if vX =/= vY
void skipVxVyNeq(){
    if (v[X] != v[Y]) PC += 2;
}

/////////////////////////////////////////
/* Logical and arithmetic instructions */
/////////////////////////////////////////


// 8XY0, set vX to vY
void setVxEqVy(){
    v[X] = v[Y];
}

// 8XY1
void setVxEqOrVy(){
    v[X] |= v[Y];
}

// 8XY2
void setVxEqAndVy(){
    v[X] &= v[Y];
}

// 8XY3
void setVxEqXorVy(){
    v[X] ^= v[Y];
}

// 8XY4, add vY to vX, set flag reg to 1 if overflow occurs
void addVyToVx(){
    uint16_t res = v[X] + v[Y];
    uint8_t checkOverflow = res > 0xFF ? 1 : 0;
    v[X] = res & 0xFF;
    v[0xF] = checkOverflow;
}

// 8XY5
void subVyFromVx(){
    uint8_t checkOverflow = (v[X] >= v[Y]) ? 1 : 0;
    v[X] -= v[Y];
    v[0xF] = checkOverflow;
}

// 8XY6
void vxSr(){
    v[X] = v[Y];
    uint8_t checkOverflow = v[X] & 0x1;
	v[X] >>= 1;
	v[0xF] = checkOverflow;
}

// 8XY7
void subVxFromVy(){
    uint8_t checkOverflow = (v[Y] >= v[X]) ? 1 : 0;
    v[X] = v[Y] - v[X];
    v[0xF] = checkOverflow;
}

// 8XYE
void vxSL(){
    v[X] = v[Y];
    uint8_t checkOverflow = v[X] >> 7;
	v[X] <<= 1;
	v[0xF] = checkOverflow;
}

// ANNN, set our index register 12b immediate register
void setIndReg(){
    I = nnn;
}

// BNNN
void jumpToNnnPlusV0(){
    PC = nnn + v[0x0];
}

// CXNN
void randAndNnn(){
    v[X] = (rand() % 0x100) & nn;
}

// DXYN, draw a sprite stored in memory based on our instruction nibbles
void draw(){
    // Store the x and y coords via our registers and make a placeholder pixel
    uint16_t x = v[X];
    uint16_t y = v[Y];
    uint8_t curPixel;
    
    // Flag for checking if a pixel is drawn overed/xor'd, set to 0 to start out with 
    v[0xF] = 0;

    // Go over the height of the sprite and the max width of 8
    for (int i = 0; i < n; i++){
        // Get the current pixel from our index register and our current y
        curPixel = memory[I + i];
        for (int j = 0; j < 8; j++){
            // Check if we are drawing over an existing pixel and set the flag accordingly
            if (((curPixel & (0x80 >> j)) != 0) && (display[x + j + ((y + i) * 64)] == 1)){
                v[0xF] = 1;
            }
            /* Get our current display pixel and xor it by the pixel in memory, 
            properly drawing a new pixel or drawing over an old one */
            display[x + j + ((y + i) * 64)] ^= ((curPixel >> (7 - j)) & 1);
        }
    }

    // Set a draw flag to true for use with our SDL window
    drawflag = true;
}

// EX9E 
void skipVxEqKeyboard(){
    if (keyboard[v[X]] == 0) PC += 2;
}

// EXA1
void skipVxNeqKeyboard(){
    if (keyboard[v[X]] != 0) PC += 2;
}

// FX07
void setVxToDelay(){
    v[X] = delayTimer;
}

// FX15 
void setDelayToVx(){
    delayTimer = v[X];
}

// FX18
void setSoundToVx(){
    soundTimer = v[X];
}

// FX1E
void addVxToI(){
    I += v[X];
}

// FX0A
void stopExecuteTillInput(){
    PC -= 2;
    for (int i = 0; i < 16; i++){
        if (keyboard[i]){
            //SDL_Delay(2000);
            v[X] = i;
            PC += 2;
            return;
        }
    }
    
}

// FX29
void setIToFontFromVx(){
    I = v[X] * 5;
}

// FX33
void saveDecVxToI(){
    memory[I] = v[X] / 100;
    memory[I+1] = (v[X] / 10) % 10;
    memory[I+2] = (v[X] % 100) % 10;
}

// FX55
void storeRegs(){
    for (uint8_t i = 0; i <= X; i++){
        memory[I+i] = v[i];
    }
}

// FX65
void loadRegs(){
    for (uint8_t i = 0; i <= X; i++){
        v[i] = memory[I+i];
    }
}

void executeOp(){
    //printf(" %X \n", opcode);
    /* Simple switch statement based on the first num of our instruction, 
    break down the instruction further based on further nibbles */
    switch(opcode & 0xF000){
        case 0x0000: 
            switch (opcode & 0x00FF){
                case 0x00E0:
                    clearScreen();
                    break;
                case 0x00EE:
                    returnSubroutine();
                    break;
            }
            break;
        case 0x1000: 
            jump();
            break;
        case 0x2000: 
            callSubroutine();
            break;
        case 0x3000: 
            skipVxNnEq();
            break;
        case 0x4000: 
            skipVxNnNeq();
            break;
        case 0x5000: 
            skipVxVyEq();
            break;
        case 0x6000: 
            setReg();
            break;
        case 0x7000: 
            addToReg();
            break;
        case 0x8000: 
            switch(n){
                case 0x0000:
                    setVxEqVy();
                    break;
                case 0x0001:
                    setVxEqOrVy();
                    break;
                case 0x0002:
                    setVxEqAndVy();
                    break;
                case 0x0003:
                    setVxEqXorVy();
                    break;
                case 0x0004:
                    addVyToVx();
                    break;
                case 0x0005:
                    subVyFromVx();
                    break;
                case 0x0006:
                    vxSr();
                    break;
                case 0x0007:
                    subVxFromVy();
                    break;
                case 0x000E:
                    vxSL();
                    break;
            }
            break;
        case 0x9000: 
            skipVxVyNeq();
            break;
        case 0xA000: 
            setIndReg();
            break;
        case 0xB000: 
            jumpToNnnPlusV0();
            break;
        case 0xC000: 
            randAndNnn();
            break;
        case 0xD000: 
            draw();
            break;
        case 0xE000: 
            switch(nn){
                case 0x009E:
                    skipVxEqKeyboard();
                    break;
                case 0x00A1:
                    skipVxNeqKeyboard();
                    break;
            }
            break;
        case 0xF000: 
            switch(nn){
                case 0x0007:
                    setVxToDelay();
                    break;
                case 0x0015:
                    setDelayToVx();
                    break;
                case 0x0018:
                    setSoundToVx();
                    break;
                case 0x001E:
                    addVxToI();
                    break;
                case 0x000A:
                    stopExecuteTillInput();
                    break;
                case 0x0029:
                    void setIToFontFromVx();
                    break;
                case 0x0033:
                    saveDecVxToI();
                    break;
                case 0x0055:
                    storeRegs();
                    break;
                case 0x0065:
                    loadRegs();
                    break;
            }
            break;
        default:
            printf("Unknown opcode: 0x%X\n", opcode);
            break;
    }
}

void sdlDraw(){
    // Only draw if an operation was ran that requires a screen redraw
	if (drawflag){
        // Chip-8 screen is 64x32 px
        uint32_t pixels[64 * 32];
        // Temp x and y coords
        unsigned int x, y;

        // Set all screen pixels to 0, aka black
		memset(pixels, 0, (64 * 32) * 4);

        /* Loop through all screen pixels and set any pixels that share the same 
        index as our saved display on pixels to white (unsigned 32b int max) */
		for(x = 0; x < 64; x++){
		    for(y = 0; y < 32; y++){
				if (display[(x) + ((y) * 64)] == 1){
					pixels[(x) + ((y) * 64)] = UINT32_MAX;
				}
			}
		}
		
        // Update textures redrawing the screen
		SDL_UpdateTexture(screen, NULL, pixels, 64 * sizeof(uint32_t));

        // Set window position and size
		SDL_Rect position;
		position.x = 0;
		position.y = 0;
		position.w = 64;
		position.h = 32;

        // Save textures to our screen, and render it
		SDL_RenderCopy(renderer, screen, NULL, &position);
		SDL_RenderPresent(renderer);
	}

    //Reset drawflag to false to allow operations to set it on in the future
	drawflag = false;
}


// Take in one arg for the rom file name as a string
int main(int argc, char ** argv){
    // Graphics and keeb input setup
    SDL_Init(SDL_INIT_EVERYTHING);
    
	window = SDL_CreateWindow(("CHIP-8:  %s",argv[1]),SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,640,320,0);
	renderer = SDL_CreateRenderer(window,-1,SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	SDL_RenderSetLogicalSize(renderer, 64, 32);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);	
	screen = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING,64,32);


    // Emu setup
    initEmu();

    // Load rom
    loadRom(argv[1]);

    int speed = 11;

    SDL_Event event;

    // Main game loop, handle fetch, decode, and execute alongside updating/tracking keeb and graphical changes
    while (true){
        // Handle keyboard input
        if (SDL_PollEvent(&event)){
            printf("%c",  event.key.keysym.sym);
            if (event.type == SDL_Quit){
                SDL_DestroyWindow(window);
                SDL_Quit();
	            return 0;
            } else if(event.type == SDL_KEYDOWN){
                for(int i = 0; i < 16; i++){
                    if(event.key.keysym.sym == keyMap[i]){
                        keyboard[i] = 1;
                    }
                }
            } else if(event.type == SDL_KEYUP){
                for(int i = 0; i < 16; i++){
                    if(event.key.keysym.sym == keyMap[i]){
                        keyboard[i] = 0;
                    }
                }
            }
        }

        for (int i = 0; i < 9; i++) {
            // Fetch opcode/instruction, immediately increment PC by 2 rather then later to avoid possible multi increments
            opcode = memory[PC] << 8 | memory[PC + 1];
            PC += 2;

            // Save our nibbles from our 16b instruction
            X = (opcode & 0x0F00) >> 8;
            Y = (opcode & 0x00F0) >> 4;
            n =  (opcode & 0x000F);
            nn = (opcode & 0x00FF);
            nnn = (opcode & 0x0FFF); 

            // Run decoding + execute
            executeOp();
        }

        // Decrement timers
        if (delayTimer > 0) delayTimer--;
        if (soundTimer > 0) soundTimer--;


        // Redraw SDL window and simulate delay/hz via sdl_delay
        sdlDraw();
        SDL_Delay(speed);
    }

}

// -lmingw32 -lSDL2main -lSDL2
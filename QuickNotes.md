http://www.emulator101.com/
https://tobiasvl.github.io/blog/write-a-chip-8-emulator/

- Notes
. specs: 4 kb mem, 64x32 bw display, PC, (I) index reg 16b, 16b address function stack, 8b delay timer 60hz, 8b sound timer, 16 8b regs 0 to F (v0)

. All mem is writable, not like ROMS (read only mem games)

. Older programs rely on the interpreter/emu to be loaded into mem from the start add, then themselves to be loaded right after. Thus running older programs requires the interp to be set up this way in mem
 

. Store font data in mem, games draw chars like reg sprites

. Special instruc for setting I to font location, store font within 000-1FF (convention is 050-09f)

. Boilerplate font (older games used to give custom fonts, anything works):
0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
0x20, 0x60, 0x20, 0x20, 0x70, // 1
0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3 
0x90, 0x90, 0xF0, 0x10, 0x10, / 4
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

. 64x32 bw makes drawing simple, each pixel is on or off represented by a single bit

. Display can be updated at 60hz for 60 fps byt you could redraw when an instruction is executed for simpler games to improve performance

. DXYN is draw instruc per sprite, sprites are 8b bytes that are 1-15 bytes tall treating all 0 bits as transparent and 1 bits to a flip

. DXYN causes flickering objs, moving a sprite erases it by drawing over it and flipping 1 bits with a xor, and then redrawing causing a gap where the sprite is gone

. Chip-8 stack is used for subroutine addresses aka 16/12b nums

. Older stacks needed reserved mem and were operated and saved onto by the program, but we can just use a var outside the emu mem

. Can make stack mem unlimited or limited (can run into stack overflow issues)

. Delay/sound timer are one byte and decrement by 1 60 times a sec if at 60hz independent of the execution loop

. Sound timer should make computer "beep" as long as it is above 0

. Delay timer doesnt force delay, simply there for checking/tracking

. Chip-8 ran off hexadec keyboard controls, 4x4 keeb 0-F, orig layout was:
1 	2 	3 	C
4 	5 	6 	D
7 	8 	9 	E
A 	0 	B 	F

. New layout for modern qwerty (use scancodes for adaptability to other keebs/formats):
1 	2 	3 	4
Q 	W 	E 	R
A 	S 	D 	F
Z 	X 	C 	V

. Main inf loop is the fetch/decode/execute loop

. Fetch handles getting instruction from PC, decode the instruction and find what it does, then execute what it says to do

. Older systems had timings of 1-4mhz

. Standard speed of 700 instructs per sec fits most programs

. For fetch, read in 2 bytes at a time making 16 bit instructs, immediately increment PC by 2 during exec or at the end of fetch

. For decode, very simple for chip, simple switch on the first hexa num (nibble/half byte) in the instruc 

. Rest of the instruc has more info, 2nd num/nibble (X) is used to access the v0 regs same with the 3rd (Y), 4th (N) is a 4 bit num, NN is the second byte which is an 8 bit immediate num, NNN is the 2nd 3rd and 4th nibs aka a 12 bit immediate mem add

. Execute super simple, do the proper instruction

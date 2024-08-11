OBJS = src/chip8.c
HEADERS = src/chip8.h
CC = gcc
L_FLAGS = -lmingw32 -lSDL2main -lSDL2
OBJ_NAME = chip8Emu

all: $(OBJS)
	$(CC) $(OBJS) $(HEADERS) $(C_FLAGS) $(L_FLAGS) -o $(OBJ_NAME) 
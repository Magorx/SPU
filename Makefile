CUR_PROG = kspu

all: kasm kspu

run: all
	./$(CUR_PROG)

fast: kasm_o3 kspu_o3

ifndef VERBOSE
.SILENT:
endif

CC = gcc

WARNINGS = -Wall -Wno-multichar
STANDARD =  
CFLAGS = $(STANDARD) $(WARNINGS)

kspu: cpu.c cpu.h general.h stack.h opcodes.h opdefs.h
	$(CC) $(CFLAGS) cpu.c -o kspu -lm

kspu_o3: cpu.c cpu.h general.h stack.h opcodes.h opdefs.h
	$(CC) $(CFLAGS) cpu.c -o kspu -lm -O3

kasm: assembler.c assembler.h general.h opcodes.h opdefs.h
	$(CC) $(CFLAGS) assembler.c -o kasm

kasm_o3: assembler.c assembler.h general.h opcodes.h opdefs.h
	$(CC) $(CFLAGS) assembler.c -o kasm

valg: all
	make clear
	valgrind --leak-check=full --track-origins=yes -s ./$(CUR_PROG)

clear:
	rm *.o -f

show:
	hexdump -C out.kctf
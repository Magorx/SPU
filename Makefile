CUR_PROG = cpu

all: assembler cpu
	./assembler
	./cpu

ifndef VERBOSE
.SILENT:
endif

CC = gcc

WARNINGS = -Wall -Wno-multichar
STANDARD =  
CFLAGS = $(STANDARD) $(WARNINGS)

cpu: cpu.c cpu.h general.h stack.h opcodes.h opdefs.h
	$(CC) $(CFLAGS) cpu.c -o cpu -lm


assembler: assembler.c assembler.h general.h opcodes.h opdefs.h
	$(CC) $(CFLAGS) assembler.c -o assembler

cur_run: cpu assembler
	make clear
	./assembler

run: cpu assembler
	./assembler
	./$(CUR_PROG)

valg: cur_run
	make clear
	valgrind --leak-check=full --track-origins=yes -s ./$(CUR_PROG)

clear:
	rm *.o -f

show:
	hexdump -C out.kctf
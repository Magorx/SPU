#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "general.h"
#include "opcodes.h"
#include "metainf.h"

enum LINE_ASSEMBLEE_RESULT {
	EXPR_READ = 0,
	LABEL_FOUND,
	LABEL_USED
};

typedef struct Label_t {
	const unsigned char *name;
	size_t len;
	long long shift;
	size_t place;
} Label;

int check_and_process_opname   (const unsigned char **symb, ByteOP *bop, const char *opname, const int opcode);

int get_extract_register_index (const unsigned char **symb, byte *reg_index);
int get_extract_const_value    (const unsigned char **symb, double *value);

int put_extract_register_index (const unsigned char **symb, ByteOP *bop);
int put_extract_operator_value (const unsigned char **symb, ByteOP *bop);
int put_extract_value          (const unsigned char **symb, ByteOP *bop);
int put_extract_line           (const unsigned char **symb, ByteOP *bop);

int assemble_file(const char* fin_name, const char* fout_name);

#endif // ASSEMBLER_H
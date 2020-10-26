#ifndef ASSEMBER_COMMANDS_H
#define ASSEMBER_COMMANDS_H

#include <stdint.h>

const int16_t VERSION = 1;

enum CPU_SETTINGS {
	REGISTERS_COUNT = 4
};

enum VALUE_TYPES {
	VALUE_CONSTANT = 16,
	VALUE_REGISTER = 32,
	VALUE_LABEL    = 48,
    VALUE_RANDOM   = 56
};

enum OPCODES_CHAR_LITERALS {
	OPCODE_SHORT = 's',
	OPCODE_INT = 'i',
	OPCODE_LONG = 'l',
	OPCODE_UNSIGNED = 'u',
	OPNAME_DOUBLE = 'd',
	OPNAME_PLUS = '+',
	OPNAME_MINUS = '-',
	OPNAME_MULTIPLY = '*',
	OPNAME_DIVIDE = '/',
    OPNAME_RANDOM = '$'
};
const unsigned char *OPERATIONS = (unsigned char*)"+*$";

const byte OPERATIONS_ARGS[] = {
    ['+'] = 2,
    ['*'] = 2,
    ['$'] = 2
};

const char *OPNAME_PUSH = "push";
const char *OPNAME_POP  = "pop";
const char *OPNAME_ADD  = "add";
const char *OPNAME_SUB  = "sub";
const char *OPNAME_MUL  = "mul";
const char *OPNAME_DIV  = "div";
const char *OPNAME_SIN  = "sin";
const char *OPNAME_COS  = "cos";
const char *OPNAME_OUT  = "out";

const int MAX_COMMAND_ARGS_COUNT = 3;
const size_t MAX_OPCODES_COUNT = 256;

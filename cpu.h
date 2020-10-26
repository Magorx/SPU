#ifndef CPU_H
#define CPU_H

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "general.h"
#include "metainf.h"
#include "opcodes.h"

#include "cpu_settings.h"

#define STACK_SECURITY_LEVEL 0
#define STACK_VALUE_TYPE double
#define STACK_VALUE_PRINTF_SPEC "%lg"
#include "stack.h"
#undef STACK_VALUE_TYPE
#undef STACK_VALUE_PRINTF_SPEC


typedef double Register;

//<KCTF> Thread ===============================================================

typedef struct Thread_t {
	byte id;
	Register registers[REGISTERS_COUNT];
	
	ByteIP *bip;
	Stack_double  *rsp;
} Thread;

Thread *new_Thread(const byte id);
int delete_Thread(Thread *cake);

//=============================================================================
//<KCTF> CPU ==================================================================

typedef struct CPU_t {
	ByteIP *signature_reader;
	Register registers[REGISTERS_COUNT];

	Stack_double  *rsp; // Stack register pointer
	ByteIP *bip;        // ByteIP of current thread
	size_t *rip;

	double *ram;
	double *vram;

	Thread **threads;
	size_t threads_capacity;
	size_t threads_size;
	size_t next_thread;
	byte thread_id;

	byte to_tick_graphics;
	size_t screen_width;
	size_t screen_height;

	byte halt_flag;
} CPU;

CPU *new_CPU();
int delete_CPU(CPU *cake);

size_t CPU_stack_size(CPU *cake);
double CPU_stack_top (CPU *cake);
int    CPU_stack_push(CPU *cake, const double value);

int CPU_read_value      (CPU *cake, double *value);
int CPU_execute_push    (CPU *cake);
int CPU_execute_out     (CPU *cake);
int CPU_execute_pop     (CPU *cake);
int CPU_execute_add     (CPU *cake);
int CPU_execute_sub     (CPU *cake);
int CPU_execute_mul     (CPU *cake);
int CPU_execute_div     (CPU *cake);
int CPU_execute_sin     (CPU *cake);
int CPU_execute_cos     (CPU *cake);
int CPU_execute_sqrt    (CPU *cake);
int CPU_execute_in      (CPU *cake);

int CPU_execute_command (CPU *cake);

int CPU_set_cpu_context    (CPU *cake, const Thread *thread);
int CPU_set_thread_context (CPU *cake, Thread *thread);

int CPU_init_thread(CPU *cake, const char *file_name);
int CPU_stop_thread(CPU *cake, Thread *thread);

int CPU_tick(CPU *cake);

//=============================================================================

#endif
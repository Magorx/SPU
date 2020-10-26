#include "cpu.h"
#undef KCTF_DEBUG_LEVEL
#define KCTF_DEBUG_LEVEL 0

// The cpu itself =============================================================

int main(const int argc, const char **argv) {

	srand((unsigned int) time(NULL));

	const char *input_file = "out.tf";
	if (argc > 1) {
		input_file = argv[1];
	}

	CPU *cpu = new_CPU();

	CPU_init_thread(cpu, input_file);
	// CPU_init_thread(cpu, input_file);
	while(CPU_tick(cpu) == OK);

	delete_CPU(cpu);

	return 0;
}

//=============================================================================

Thread *new_Thread(const byte id) {
	Thread *thread = calloc(1, sizeof(Thread));
	VERIFY_t(thread != NULL, Thread*);

	thread->rsp = NULL;
	thread->bip = NULL;
	thread->id = id;
	for (byte i = 0; i < REGISTERS_COUNT; ++i) {
		thread->registers[i] = 0.0;
	}

	return thread;
}

int delete_Thread(Thread *cake) {
	VERIFY(cake != NULL);

	free(cake);

	return 0;
}

CPU *new_CPU() {
	CPU *cpu = calloc(1, sizeof(CPU));

	for (byte i = 0; i < REGISTERS_COUNT; ++i) {
		cpu->registers[i] = 0.0;
	}

	VERIFY_t((cpu->signature_reader = new_ByteIP(sizeof(Signature))) != NULL, CPU*);

	cpu->threads_capacity = 5;
	cpu->threads_size = 0;
	cpu->next_thread = 0;
	cpu->thread_id = 0;
	VERIFY_t((cpu->threads = calloc(sizeof(Thread), cpu->threads_capacity)) != NULL, CPU*);

	cpu->rsp = NULL;
	cpu->rip = NULL;

	return cpu;
}

int delete_CPU(CPU *cake) {
	VERIFY(cake != NULL);
	
	free(cake->threads);
	delete_ByteIP(cake->signature_reader);
	free(cake);

	return 0;
}

size_t CPU_stack_size(CPU *cake) {
	VERIFY_t(cake != NULL, size_t);
	return Stack_size_double(cake->rsp);
}

double CPU_stack_pop(CPU *cake) {
	VERIFY(cake != NULL);
	VERIFY(CPU_stack_size(cake) > 0);
	
	double val = Stack_top_double(cake->rsp);
	DEBUG(5) printf("poped %lg\n", val);
	Stack_pop_double(cake->rsp);

	return val;
}

int CPU_stack_push(CPU *cake, const double value) {
	VERIFY(cake != NULL);

	Stack_push_double(cake->rsp, value);
	return 0;
}

long long CPU_random(CPU *cake) {
	return randlong() % CPU_MAX_RANDOM_LONG;
}

double CPU_symb_operation(CPU *cake, byte op, const double val_1, const double val_2) {
	switch (op) {
		case OPNAME_PLUS:
			return val_1 + val_2;
		case OPNAME_MINUS:
			return val_1 - val_2;
		case OPNAME_MULTIPLY:
			return val_1 * val_2;
		case OPNAME_DIVIDE:
			return val_1 / val_2;
		case OPNAME_RANDOM:
			return (double) (CPU_random(cake) % ((long long) val_2) + (long long) val_1);
			break;
		default:
			printf("[ERR]<cpu>: operator %.2x doesn't exist, aborting\n", op);
	}

	return 0.0 / 0.0;
}

size_t CPU_read_size_t(CPU *cake) {
	size_t val;
	//memcpy(&val, &cake->bip->buffer[*cake->rip], sizeof(size_t));
	//*cake->rip += sizeof(size_t);
	ByteIP_get_size_t(cake->bip, &val);
	return val;
}

byte CPU_read_byte(CPU *cake) {
	byte val;
	ByteIP_get_byte(cake->bip, &val);
	return val;
}

int CPU_read_value(CPU *cake, double *value) {
	const byte symb = cake->bip->buffer[cake->bip->cur_idx];

	cake->bip->cur_idx += 1;
	DEBUG(5) printf("Reading from byte %.2x\n", symb);
	if (symb == VALUE_CONSTANT) {
		DEBUG(5) printf("const\n");
		ByteIP_get_double(cake->bip, value);
	} else if (symb == VALUE_REGISTER) {
		DEBUG(5) printf("register\n");
		byte reg_idx = 0;
		ByteIP_get_byte(cake->bip, &reg_idx);
		DEBUG(5) printf("[%.2x] -> %lg\n", reg_idx, cake->registers[reg_idx]);
		*value = cake->registers[reg_idx];
	} else {
		double val_1 = 0;
		double val_2 = 0;
		CPU_read_value(cake, &val_1);
		CPU_read_value(cake, &val_2);
		*value = CPU_symb_operation(cake, symb, val_1, val_2);
	}

	return 0;
}				

int CPU_execute_push(CPU *cake) {
	double val = 0;
	CPU_read_value(cake, &val);
	DEBUG(5) printf("pushing val = %lg\n", val);
	CPU_stack_push(cake, val);

	return 0;
}

int CPU_execute_out(CPU *cake) {
	printf("%lg\n", CPU_stack_pop(cake));

	return 0;
}

int CPU_execute_pop(CPU *cake) {
	VERIFY(CPU_stack_size(cake) > 0);

	byte reg_idx = 0;
	ByteIP_get_byte(cake->bip, &reg_idx);
	VERIFY(reg_idx == VALUE_REGISTER);
	ByteIP_get_byte(cake->bip, &reg_idx);

	cake->registers[reg_idx] = CPU_stack_pop(cake);
	DEBUG(5) printf("pushed %lg in %.2x\n", cake->registers[reg_idx], reg_idx);

	return 0;
}

int CPU_execute_dup(CPU *cake) {
	double val = CPU_stack_pop(cake);
	CPU_stack_push(cake, val);
	CPU_stack_push(cake, val);

	return 0;
}

int CPU_execute_op(CPU *cake) {
	byte op = 0;
	op = CPU_read_byte(cake);
	double val_1 = CPU_stack_pop(cake); // POP INSIDE OF OP
	double val_2 = CPU_stack_pop(cake);
	CPU_stack_push(cake, CPU_symb_operation(cake, op, val_2, val_1));

	return 0;
}

int CPU_execute_add(CPU *cake) {
	VERIFY(CPU_stack_size(cake) >= 2);

	double val_1 = CPU_stack_pop(cake);
	double val_2 = CPU_stack_pop(cake);
	CPU_stack_push(cake, val_2 + val_1);

	return 0;
}

int CPU_execute_sub(CPU *cake) {
	VERIFY(CPU_stack_size(cake) >= 2);

	double val_1 = CPU_stack_pop(cake);
	double val_2 = CPU_stack_pop(cake);
	CPU_stack_push(cake, val_2 - val_1);

	return 0;
}

int CPU_execute_mul(CPU *cake) {
	VERIFY(CPU_stack_size(cake) >= 2);

	double val_1 = CPU_stack_pop(cake);
	double val_2 = CPU_stack_pop(cake);
	CPU_stack_push(cake, val_2 * val_1);

	return 0;
}

int CPU_execute_div(CPU *cake) {
	VERIFY(CPU_stack_size(cake) >= 2);

	double val_1 = CPU_stack_pop(cake);
	double val_2 = CPU_stack_pop(cake);
	CPU_stack_push(cake, val_2 / val_1);

	return 0;
}

int CPU_execute_sin(CPU *cake) {
	VERIFY(CPU_stack_size(cake) > 0);
	
	double val = CPU_stack_pop(cake);
	CPU_stack_push(cake, sin(val));


	return 0;
}

int CPU_execute_cos(CPU *cake) {
	VERIFY(CPU_stack_size(cake) > 0);
	
	double val = CPU_stack_pop(cake);
	CPU_stack_push(cake, cos(val));

	return 0;
}

int CPU_execute_sqrt(CPU *cake) {
	VERIFY(Stack_size_double(cake->rsp) > 0);
	
	double val = CPU_stack_pop(cake);
	CPU_stack_push(cake, sqrt(val));

	return 0;
}

int CPU_execute_in(CPU *cake) {
	double val = 0;
	printf("[   ]<   >: [thread](%d)->(input)\n", cake->thread_id);
	scanf("%lg", &val);
	DEBUG(5) printf("Read %lg from the keyboard\n", val);
	CPU_stack_push(cake, val);

	return 0;
}

int CPU_jump(CPU *cake, size_t rip) {
	DEBUG(3) printf("Jumping on %zu\n", rip);
	*cake->rip = rip;
	return 0;
}

int CPU_execute_jmp(CPU *cake) {
	size_t new_rip = CPU_read_size_t(cake);
	CPU_jump(cake, new_rip);

	return 0;
}

int CPU_execute_call(CPU *cake) {
	size_t new_rip = CPU_read_size_t(cake);
	CPU_stack_push(cake, (double) *cake->rip);
	CPU_jump(cake, new_rip);

	return 0;
}

int CPU_execute_ret(CPU *cake) {
	size_t new_rip = (size_t) CPU_stack_pop(cake);
	CPU_jump(cake, new_rip);

	return 0;
}

int CPU_execute_ja(CPU *cake) {
	size_t new_rip = (size_t) CPU_read_size_t(cake);
	if (CPU_stack_pop(cake) < CPU_stack_pop(cake)) {
		CPU_jump(cake, new_rip);
	}

	return 0;
}

int CPU_execute_jae(CPU *cake) {
	size_t new_rip = (size_t) CPU_read_size_t(cake);
	if (CPU_stack_pop(cake) <= CPU_stack_pop(cake)) {
		CPU_jump(cake, new_rip);
	}

	return 0;
}

int CPU_execute_jb(CPU *cake) {
	size_t new_rip = (size_t) CPU_read_size_t(cake);
	if (CPU_stack_pop(cake) > CPU_stack_pop(cake)) {
		printf("YPOUOUO\n");
		CPU_jump(cake, new_rip);
	}

	return 0;
}

int CPU_execute_jbe(CPU *cake) {
	size_t new_rip = (size_t) CPU_read_size_t(cake);
	if (CPU_stack_pop(cake) >= CPU_stack_pop(cake)) {
		CPU_jump(cake, new_rip);
	}

	return 0;
}

int CPU_execute_je(CPU *cake) {
	size_t new_rip = (size_t) CPU_read_size_t(cake);

	//printf("abs = %lg\n", fabs(CPU_stack_pop(cake) - CPU_stack_pop(cake)));
	if (fabs(CPU_stack_pop(cake) - CPU_stack_pop(cake)) <= EPS) {
		CPU_jump(cake, new_rip);
	}

	return 0;
}

int CPU_execute_jne(CPU *cake) {
	size_t new_rip = (size_t) CPU_read_size_t(cake);
	if (fabs(CPU_stack_pop(cake) - CPU_stack_pop(cake)) > EPS) {
		CPU_jump(cake, new_rip);
	}

	return 0;
}

int CPU_execute_halt(CPU *cake) {
	return -1;
}

#define OPDEF(opname, opcode, opargs)			 	  \
	case OPCODE_ ## opname:			  			  	  \
		return CPU_execute_ ## opname(cake);

int CPU_execute_command(CPU *cake) {
	if (cake->bip->cur_idx == cake->bip->size) {
		return -1;
	}

	byte op = 0;
	ByteIP_get_byte(cake->bip, &op);
	DEBUG(2) printf("op %.2x on ind %zu stack size %zu\n", op, cake->bip->cur_idx - 1, Stack_size_double(cake->rsp));

	switch (op) {
		
		#include "opcodes_def.h"

		default:
			return -1;
			break;
	}

	return 0;
}

#undef OPDEF

int CPU_set_cpu_context(CPU *cake, const Thread *thread) {
	cake->rsp = thread->rsp;
	cake->bip = thread->bip;
	cake->rip = &cake->bip->cur_idx;
	cake->thread_id  = thread->id;
	for (byte i = 0; i < REGISTERS_COUNT; ++i) {
		cake->registers[i] = thread->registers[i];
	}

	return 0;
}

int CPU_set_thread_context(CPU *cake, Thread *thread) {
	thread->rsp = cake->rsp;
	thread->bip = cake->bip;
	thread->id = cake->thread_id;
	for (byte i = 0; i < REGISTERS_COUNT; ++i) {
		thread->registers[i] = cake->registers[i];
	}

	return 0;
}

int CPU_init_thread(CPU *cake, const char *file_name) {
	ByteIP_read_file(cake->signature_reader, file_name, 0);
	Signature sig = {};
	ByteIP_get(cake->signature_reader, &sig, sizeof(Signature));
	ByteIP_reset(cake->signature_reader);

	if (sig.magic != KCTF_MAGIC) {
		printf("[ERR]<cpu>: (^^^^^^) Magic is not 'KCTF'\n");
		return 0;
	}

	Thread *thr = NULL;
	for (byte i = 0; i < cake->threads_capacity; ++i) {
		if (cake->threads[i] == NULL) {
			printf("[INT]<cpu>: [thread](%d)->(init)\n", i);
			cake->threads[i] = new_Thread(i);
			thr = cake->threads[i];
			cake->thread_id = i;
			break;
		}
	}
	if (thr == NULL) {
		printf("[ERR]<cpu>: (^^^^^^) No more thread slots\n");
		return 0;
	}

	cake->rsp = Stack_new_double();
	if (cake->rsp == NULL) {
		printf("[ERR]<cpu>: (^^^^^^) Stack creation failed\n");
	}

	ByteIP *bip = new_ByteIP((size_t) sig.file_size);
	ByteIP_read_file(bip, file_name, (size_t) sig.file_size);
	ByteIP_get(bip, &sig, sizeof(Signature));
	cake->bip = bip;

	CPU_set_thread_context(cake, thr);
	++cake->threads_size;
	printf("[STH]<cpu>: [thread](%d)\n", thr->id);

	return 0;
}

int CPU_stop_thread(CPU *cake, Thread *thread) {
	VERIFY(cake != NULL);
	VERIFY(thread != NULL);
	printf("[ETH]<cpu>: [thread](%d)->(stop)\n", thread->id);

	CPU_set_cpu_context(cake, thread);

	Stack_delete_double(cake->rsp);
	delete_ByteIP(cake->bip);
	delete_Thread(thread);

	return 0;
}

int CPU_tick(CPU *cake) {
	VERIFY(cake != NULL);

	size_t thr_idx = cake->next_thread;
	size_t thrs_tried = 0;
	while (thrs_tried <= cake->threads_capacity && cake->threads[thr_idx] == NULL) {
		thr_idx = (thr_idx + 1) % cake->threads_capacity;
		++thrs_tried;
	}
	cake->next_thread = thr_idx + 1;
	Thread *thr = cake->threads[thr_idx];
	if (thr == NULL) {
		return -1;
	}

	CPU_set_cpu_context(cake, thr);
	if (CPU_execute_command(cake) == OK) {
		CPU_set_thread_context(cake, thr);	
	} else {
		CPU_stop_thread(cake, thr);
		cake->threads[thr_idx] = NULL;
	}

	return 0;
}

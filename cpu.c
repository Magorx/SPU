#include "cpu.h"
#undef KCTF_DEBUG_LEVEL
#define KCTF_DEBUG_LEVEL 0

long long cur_time;
long long prev_cur_time;
long long cnt;
// The cpu itself =============================================================

int main(const int argc, const char **argv) {

	srand((unsigned int) time(NULL));
	prev_cur_time = time(NULL);
	cnt = 0;

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

//=============================================================================

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
	cpu->ram = calloc(CPU_RAM_SIZE, sizeof(double));
	cpu->vram = cpu->ram;
	VERIFY_t(cpu->ram != NULL, CPU*);

	cpu->to_tick_graphics = 0;

	cpu->halt_flag = 0;

	return cpu;
}

int delete_CPU(CPU *cake) {
	VERIFY(cake != NULL);
	
	free(cake->threads);
	delete_ByteIP(cake->signature_reader);
	free(cake->ram);
	free(cake);

	return 0;
}

//=============================================================================
// Graphic ====================================================================

int CPU_graphics_init(CPU *cake, const size_t width, const size_t height) {
	size_t pixels_cnt = width * height;
	cake->screen_width = width;
	cake->screen_height = height;
	cake->vram = cake->ram + CPU_RAM_SIZE - (pixels_cnt + 1);
	printf("\033c");

	return 0;
}

int CPU_graphics_draw_clear(CPU *cake) {
	printf("=== You shouldn't see this ===\n");
	for (size_t y = 0; y < cake->screen_height; ++y) {
		printf("\n");
	}

	return 0;
}

#define clear() printf("\033[H\033[J")
#define gotoxy(x,y) printf("\033[%d;%dH", (y), (x))

int CPU_graphics_draw_screen(CPU *cake) {
	gotoxy(0, 0);
	//printf("\e[?25l"); // printf("\e[?25h"); - recover :)

	putchar('|');
	for (size_t x = 0; x < cake->screen_width; ++x) {
		putchar('='	);
	}
	//printf("w=%zuh=%zu", cake->screen_width, cake->screen_height);
	putchar('|');
	putchar('\n');
	for (size_t y = 0; y < cake->screen_height; ++y) {
		printf("|");
		for (size_t x = 0; x < cake->screen_width; ++x) {
			//printf("%zu %zu\n", y, x);
			unsigned char symb = (unsigned char) cake->vram[y * cake->screen_width + x];	
			if (symb < 254) {
				if (symb < 12) {
					putchar(".-,~:;*=!#$@"[symb]);
				}
				else {
					putchar(symb);
				}
			} else {
				putchar(' ');
			}
		}
		putchar('|');
		putchar('\n');
	}
	putchar('|');
	for (size_t x = 0; x < cake->screen_width; ++x) {
		putchar('=');
	}
	putchar('|');
	putchar('\n');
	//printf("\e[?25h");

	return 0;
}

int CPU_graphics_tick(CPU *cake) {
	//CPU_graphics_draw_clear(cake);
	//clear();
	CPU_graphics_draw_screen(cake);

	return 0;
}

//=============================================================================
// General incapsulations =====================================================

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

long long CPU_random() {
	return randlong() % CPU_MAX_RANDOM_LONG;
}

//=============================================================================
// Logic and operations =======================================================

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
			return (double) (CPU_random(cake) % ((long long) (val_2 + 1 - val_1)) + (long long) val_1);
			break;
		default:
			printf("[ERR]<cpu>: operator %.2x doesn't exist, aborting\n", op);
	}

	return 0.0 / 0.0;
}

size_t CPU_read_size_t(CPU *cake) {
	size_t val = 0;
	ByteIP_get_size_t(cake->bip, &val);
	return val;
}

byte CPU_read_byte(CPU *cake) {
	byte val = 0;
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
	} else if (symb == VALUE_RAM) {
		double val = 0;
		CPU_read_value(cake, &val);
		size_t ram_index = (size_t) val;
		VERIFY(ram_index > 0 && ram_index < CPU_RAM_SIZE);
		*value = cake->ram[ram_index];
	} else if (symb == VALUE_VRAM) {
		double val = 0;
		CPU_read_value(cake, &val);
		size_t vram_index = (size_t) val;
		VERIFY(vram_index < cake->screen_width * cake->screen_height);
		*value = cake->vram[vram_index];
	} else {
		double val_1 = 0;
		double val_2 = 0;
		CPU_read_value(cake, &val_1);
		CPU_read_value(cake, &val_2);
		*value = CPU_symb_operation(cake, symb, val_1, val_2);
	}

	return 0;
}

int CPU_jump(CPU *cake, size_t rip) {
	DEBUG(3) printf("Jumping on %zu\n", rip);
	*cake->rip = rip;
	return 0;
}

#define OPDEF(opname, opcode, oparg, code)  \
int CPU_execute_ ## opname (CPU *cake) {	\
	byte reg_idx    = 0;					\
	byte byte_reg   = 0;					\
	double val      = 0;					\
	double val_1    = 0;					\
	double val_2    = 0;					\
	size_t new_rip  = 0;					\
	byte type       = 0;					\
	double idx      = 0;					\
	{code}									\
	return 0;								\
}

#include "opdefs.h"

#undef OPDEF

#define OPDEF(opname, opcode, opargs, ignr)		 	  \
	case OPCODE_ ## opname:			  			  	  \
		return CPU_execute_ ## opname(cake);

int CPU_execute_command(CPU *cake) {
	/*cnt++;
	cur_time = time(NULL);
	if (cur_time > prev_cur_time) {
		printf("sec passed! %lld commands executed\n", cnt);
		prev_cur_time = cur_time;
		cnt = 0;
	}*/

	if (cake->bip->cur_idx == cake->bip->size) {
		return -1;
	}

	byte op = 0;
	ByteIP_get_byte(cake->bip, &op);
	DEBUG(2) printf("op %.2x on ind %zu stack size %zu \n", op, cake->bip->cur_idx - 1, Stack_size_double(cake->rsp));

	switch (op) {
		
		#include "opdefs.h"

		default:
			printf("[ERR]<cpu>: unknown command on %zu: %.2x\n", *cake->rip, cake->bip->buffer[*cake->rip]);
			return -1;
			break;
	}

	return 0;
}

#undef OPDEF

//=============================================================================
// Threads work ===============================================================

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

	if (cake->halt_flag) {
		return -1;
	}

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

	if (cake->to_tick_graphics) {
		CPU_graphics_tick(cake);
	}

	return 0;
}

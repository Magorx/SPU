#include "assembler.h"

// The assembler itself =======================================================

int main(const int argc, const char **argv) {
	const char *input_file  = "prog.kc";
	const char *output_file = "out.tf";
	if (argc > 1) {
		input_file = argv[1];
	}
	if (argc > 2) {
		output_file = argv[2];
	}

	assemble_file(input_file, output_file);

	return 0;
}

//=============================================================================

//=============================================================================

Label *new_Lable(const unsigned char *name, size_t place) {
	Label *l = calloc(1, sizeof(Label));
	l->name = name;
	l->place = place;

	return l;
}

int delete_Lable(Label *l) {
	VERIFY(l != NULL);
	free(l);

	return 0;
}

int Label_compare(const Label *first, const Label *second) {
	const unsigned char *f = first->name;
	const unsigned char *s = second->name;
	while(!(isspace(*f) || isspace(*s)) && *f != ':' && *s != ':' &&*f && *s) {
		if (*f != *s) {
			return -1;
		}
		++f;
		++s;
	}
	return 0;
}

//=============================================================================

int check_and_process_opname(const unsigned char **symb, ByteOP *bop, const char *opname, const int opcode) {
	size_t opname_len = strlen(opname);
	if (String_starts_with(*symb, opname) && (*(*symb + opname_len) == ' ' || *(*symb + opname_len) == '\0')) {
		*symb += strlen(opname);
    	Char_get_next_symb(symb);

    	ByteOP_put_byte(bop, (byte) opcode);
    	return 1;
	}
	return 0;
}

int get_extract_register_index(const unsigned char **symb, byte *reg_index) {
	VERIFY(**symb == 'r');

	*symb += 1;

	const byte register_index = (byte) (**symb - 'a');
	VERIFY(register_index <= REGISTERS_COUNT);
	
	*symb += 2;
	Char_get_next_symb(symb);

	*reg_index = register_index;
	return 0;
}

int put_extract_register_index(const unsigned char **symb, ByteOP *bop) {
	byte reg_idx = 0;
	get_extract_register_index(symb, &reg_idx);
	ByteOP_put_byte(bop, reg_idx);
	return 0;
}

int get_extract_const_value(const unsigned char **symb, double *value) {
	double val = 0;
	int symbs_read = 0;
	sscanf((const char *) *symb, "%lg%n", &val, &symbs_read);
	VERIFY(symbs_read > 0);

	*symb += symbs_read;
	Char_get_next_symb(symb);

	*value = val;
	return 0;
}

int put_extract_operator_value(const unsigned char **symb, ByteOP *bop) {
	VERIFY(Char_in_string(**symb, OPERATIONS));
	ByteOP_put_byte(bop, **symb);

	*symb += 1;
	Char_get_next_symb(symb);

	return 0;
}

byte check_put_extract_op(const unsigned char **symb, ByteOP *bop) {
	if (Char_in_string(**symb, OPERATIONS)) {
		put_extract_operator_value(symb, bop);
		return 1;
	} else {
		return 0;
	}
}

int put_extract_value(const unsigned char **symb, ByteOP *bop) {
	VERIFY(**symb != '\n');

	DEBUG(5) printf("Current str is: |%s|\n", *symb);

	if (Char_in_string(**symb, OPERATIONS)) {
		put_extract_operator_value(symb, bop);
	} else if (**symb == 'r') {
		byte reg_idx = 0;
		get_extract_register_index(symb, &reg_idx);
		byte operation_put = check_put_extract_op(symb, bop);
		
		ByteOP_put_byte(bop, VALUE_REGISTER);
		ByteOP_put_byte(bop, reg_idx);

		if (operation_put) {
			put_extract_value(symb, bop);
		}
	} else if (isdigit(**symb) || **symb == '+' || **symb == '-') {
		double const_val = 0;
		get_extract_const_value(symb, &const_val);
		byte operation_put = check_put_extract_op(symb, bop);

		ByteOP_put_byte(bop, VALUE_CONSTANT);
		ByteOP_put_double(bop, const_val);

		if (operation_put) {
			put_extract_value(symb, bop);
		}
	} else if (**symb == '[') {
		ByteOP_put_byte(bop, VALUE_RAM);
		*symb += 1;
		put_extract_value(symb, bop);
		*symb += 1;
	} else if (**symb == '(') {
		ByteOP_put_byte(bop, VALUE_VRAM);
		*symb += 1;
		put_extract_value(symb, bop);
		*symb += 1;
	} else if (**symb == ';') {
		while (**symb) {
			*symb += 1;
		}
	}

	Char_get_next_symb(symb);

	return 0;
}

int put_extract_line(const unsigned char **symb, ByteOP *bop) {
	for (int opcode = 0; opcode < (int) MAX_OPCODES_COUNT; ++opcode) {
		if (OPNAMES[opcode] == NULL) {
			continue;
		}

		if (check_and_process_opname(symb, bop, OPNAMES[opcode], opcode)) {
			if (OPARGS[opcode] <= MAX_COMMAND_ARGS_COUNT) {
				for (byte i = 0; i < OPARGS[opcode]; ++i) {
					VERIFY_OK(put_extract_value(symb, bop));
				}
				if (**symb != '\0') {
					printf("[ERR]<assembler>: line was not read to the end, aborting. left: |%s|\n", *symb);
					return -1;
				}
				return EXPR_READ;
			} else if (OPARGS[opcode] == VALUE_LABEL) {
				return LABEL_USED;
			} else {
				printf("[ERR] Arguments for %d are ucked up\n", opcode);
			}
		}
	}

	const unsigned char *s = *symb;
	while (*s && *s != '\n') {
		++s;
	}
	--s;
	if (*s == ':') {
		return LABEL_FOUND;
	}


    return -1;
}

int assemble_file(const char *fin_name, const char* fout_name) {
	File fin = {};
    int ret = File_construct(&fin, fin_name);
    if (ret < 0) {
    	printf("File doesn't exits :(\n");
        return -1;
    }

    Signature signature = {};
    signature.magic   = KCTF_MAGIC;
    signature.version = VERSION;

    ByteOP *bop = new_ByteOP(STANDART_INIT_SIZE);
    ByteOP *lst = new_ByteOP(STANDART_INIT_SIZE);

    ByteOP_put(bop, &signature, sizeof(Signature));

    printf("[SAT]<assembler>: Starting assembling\n");
    printf("[   ]<         >: [filename](%s) [lines_cnt](%zu)\n", fin_name, fin.lines_cnt);
    printf("\n");

    ByteOP_put_string(lst, "------\n");
    ByteOP_put_string(lst, "------\n");
    printf("------\n");

    Label **lables_used    = calloc(fin.lines_cnt, sizeof(Label));
    size_t l_used_cnt = 0;
    Label **lables_found   = calloc(fin.lines_cnt, sizeof(Label));
    size_t l_found_cnt = 0;

    for (size_t line_i = 0; line_i < fin.lines_cnt; ++line_i) {
    	Line *line = fin.lines[line_i];
    	if (line->len == 0 || line->string[0] == ';') {
    		continue;
    	}
    	const unsigned char *str = line->string;
    	const unsigned char *symb = str;
		Char_get_next_symb(&symb);

    	const int before_line_index = (int) (bop->cur_ptr - bop->buffer); // for listing
    	char listed = 0;

    	switch(put_extract_line(&symb, bop)) {
    		case EXPR_READ:
    			break;
    		case LABEL_USED:
    			lables_used[l_used_cnt] = new_Lable(symb, (size_t)(bop->cur_ptr - bop->buffer));
    			++l_used_cnt;
    			ByteOP_put_double(bop, 0.0);
    			break;
    		case LABEL_FOUND:
    			lables_found[l_found_cnt] = new_Lable(line->string, (size_t)(bop->cur_ptr - bop->buffer));
    			size_t lable_len = strlen((char*) line->string);
    			line->string[lable_len - 1] = '\0';
    			--lable_len;
    			printf("     |");
    			for (size_t i = 0; i < 3 * STANDART_BYTE_LINE_BYTES_COUNT - 2 - lable_len; ++i) {
    				printf(" ");
    			}
    			printf("%s ->|\n", lables_found[l_found_cnt]->name);
    			listed = 1;
    			++l_found_cnt;
    			break;
    		default:
    			printf("[ERR]<assembler>: Command doesn't exist: [%zu](%s)\n", line_i, line->string);
    			return -1;
    	}

    	// Listing =====
    	if (!listed) {
	    	printf("%.4ld | ", bop->cur_ptr - bop->buffer);
	    	size_t bytes_prined = 0;
	    	for (size_t i = 0; i < (size_t) (bop->cur_ptr - bop->buffer - before_line_index); ++i) {
	    		printf("%.2x ", bop->buffer[before_line_index + (int) i]);
	    		++bytes_prined;
	    	}

	    	while (bytes_prined < STANDART_BYTE_LINE_BYTES_COUNT) {
	    		printf("   ");
	    		++bytes_prined;
	    	}
	    	printf("| [%zu] %s", line_i, str);
	    	printf("\n");
	    }

    	//==============
    }

    for (size_t l_used = 0; l_used < l_used_cnt; ++l_used) {
    	Label *used = lables_used[l_used];
    	char is_found = 0;
    	for (size_t l_found = 0; l_found < l_found_cnt; ++l_found) {
    		if (Label_compare(lables_found[l_found], used) == 0) {
    			Label *found = lables_found[l_found];
    			
    			byte* prev_cur_ptr = bop->cur_ptr;
    			bop->cur_ptr = bop->buffer + used->place;
    			bop->size    = bop->size - sizeof(double);
    			ByteOP_put_size_t(bop, found->place);
    			bop->cur_ptr = prev_cur_ptr;

    			DEBUG(5) printf("place %zu for %s\n", found->place, used->name);
    			is_found = 1;
    		}
    	}
    	if (!is_found) {
    		printf("[ERR]<assembler>: assembling interupted: label for %s is not found\n", lables_used[l_used]->name);
    		return -1;
    	}
    }

    File_destruct(&fin);

    signature.file_size = bop->cur_ptr - bop->buffer;
    bop->cur_ptr = bop->buffer;
    ByteOP_put(bop, &signature, sizeof(Signature));
    bop->size -= sizeof(Signature);
    ByteOP_to_file(bop, fout_name);

    printf("------\n");

    printf("%.4ld - %s\n", signature.file_size, fout_name);

    delete_ByteOP(bop);

   	ByteOP_to_file(lst, "lst.lst");
   	delete_ByteOP(lst);

    printf("\n[END]<assembler>: Done assembling\n");

	return 0;
}

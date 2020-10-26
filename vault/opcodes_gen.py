#!/bin/python3

fout = open('opcodes.h', 'w')

opnames = {
   #'name' : [opcode, args_cnt],	
	'push' : [1,   1],
	'pop'  : [2,   1],
	'dup'  : [3,   0],
	'op'   : [10,  1],
	'add'  : [11,  0],
	'sub'  : [12,  0],
	'mul'  : [13,  0],
	'div'  : [14,  0],
	'sin'  : [20,  0],
	'cos'  : [21,  0],
	'sqrt' : [22,  0],
	'in'   : [50,  0],
	'out'  : [51,  0],
	'jmp'  : [101, 'VALUE_LABEL'],
	'ja'   : [102, 'VALUE_LABEL'],
	'jae'  : [103, 'VALUE_LABEL'],
	'jb'   : [104, 'VALUE_LABEL'],
	'jbe'  : [105, 'VALUE_LABEL'],
	'je'   : [106, 'VALUE_LABEL'],
	'jne'  : [107, 'VALUE_LABEL'],
	'call' : [108, 'VALUE_LABEL'],
	'ret'  : [109, 0],
	'halt' : [255, 0]
}

opcodes = [[0, ''] for _ in range(256)]
for i in range(len(opcodes)):
	for opname in opnames:
		args = opnames[opname]
		if args[0] == i:
			cnt = args[1]
			if opcodes[i][0]:
				print('Similar opcodes!', i)
				exit()

			opcodes[i][0] = i
			opcodes[i][1] = opname

upper_h = open('opcodes_h_upper.h').read()
lower_h = open('opcodes_h_lower.h').read()

print(upper_h, file=fout)

# Generation of opcode_def.h ==================================================

fout_suf = open('opcodes_def.h', 'w')
for suf in opnames:
	print('OPDEF({}, {}, {})'.format(suf, opnames[suf][0], opnames[suf][1]), file=fout_suf)
fout_suf.close()

#==============================================================================
# Generation of opcodes, opnames and opargs_count =============================

# Opcodes
print("enum OPCODES {", file=fout)
print("    #define OPDEF(opname, opcode, opargs) OPCODE_ ## opname = opcode,", file=fout)
print('    #include "opcodes_def.h"', file=fout)
print('    #undef OPDEF', file=fout)
print("};\n", file=fout)

##for code in opcodes:
##	if code[0]:
##		name = 'OPCODE_{}'.format(code[1])
##		print('   {:<12}  = {},'.format(name,code[0]), file=fout)

# Opnames =====================================================================

print("const char *OPNAMES[] = {", file=fout)
print("    #define OPDEF(opname, opcode, opargs) [opcode] = #opname,", file=fout)
print('    #include "opcodes_def.h"', file=fout)
print('    #undef OPDEF', file=fout)
print("};\n", file=fout)

##for code in opcodes:
##	if code[0]:
##		print('    [{:<3}] = "{}",'.format(code[0], code[1]), file=fout)
##

# Opargs ======================================================================

print("const byte OPARGS[] = {", file=fout)
print("    #define OPDEF(opname, opcode, opargs) [opcode] = opargs,", file=fout)
print('    #include "opcodes_def.h"', file=fout)
print('    #undef OPDEF', file=fout)
print("};\n", file=fout)
##for code in opcodes:
##	if code[0]:
##		print('    [{:<3}] = {},'.format(code[0], opnames[code[1]][1]), file=fout)

#==============================================================================

print(lower_h, file=fout)

fout.close()

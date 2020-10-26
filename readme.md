# Assembler + CPU

The set of commands can be changed, look for ```opdefs.h``` file. Add just one block like others and use it in your assembler! (But don't try to use same opcodes for different opnames)

# Assembler
Usage:
1) Build assembler
```
make kasm
```
2) Run 
```
kasm your_prog.xxx assembled_name.xxx
```
to get bytecode for the SPU in ```assembled_name.xxx```. 

The suggested format is ```.kc``` for programms with assebler code and ```.tf``` for assembled files.

Running just ```kasm``` will try to find file named ```prog.kc``` in current directory and assemble it into ```out.tf```.

# SPU
Usage:
1) Build spu
```
make kspu 
```
2) Run
```
kspu prog.xxx
```
to run programm produced **only by assembler**. Running any other file will result in UB and other horrible things.

The suggested format is ```.tf``` for programms.

# -O3 versions
For both kasm and kspu there are make options with -O3 flag: ```kasm_o3``` ```kspu_o3```. Run ```make fast``` to get them instead of just ```make all```
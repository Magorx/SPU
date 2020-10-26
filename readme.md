# Assembler + CPU

The set of commands can be changed, look for ```opdefs.h``` file. Add just one block like others and use it in your assembler! (But don't try to use same opcodes for different opnames)

# Assembler
Usage:
1) Build assembler
```
make assembler
```
2) Run 
```
assembler your_prog.xxx assembled_name.xxx
```
to get bytecode for the CPU in ```assembled_name.xxx```. 

The suggested format is ```.kc``` for programms with assebler code and ```.tf``` for assembled files.

Running just ```assembler``` will try to find file named ```prog.kc``` in current directory and assemble it into ```out.tf```.

# CPU
Usage:
1) Build cpu
```
make cpu 
```
2) Run
```
cpu prog.xxx
```
to run programm produced **only by assembler**. Running any other file will result in UB and other horrible things.

The suggested format is ```.tf``` for programms.